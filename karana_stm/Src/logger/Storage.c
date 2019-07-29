/* Src/logger/Storage.c
 * Written by Mihai Renea for MicroEnergy International
 *
 * This file is part of the Karana V2 Framework (https://github.com/soldesign/Karana-V2).
 * Copyright (c) 2019 MicroEnergy International
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Storage.h"
#include "time_loc.h"
#include "fatfs.h"
#include "cmsis_os.h"
#include "string.h"

/* Karana Vector File v1
 *
 * Vector file header:
 * [version	]
 * [1 B		]
 *
 * record structure:
 * [UNIX time ms			][vector ID	][vector size	][data				]
 * [<VF_RECORD_UNIXT_LEN> B	][1 B		][1 B			][<vector size> B	]
 */

/*
 * Every new hour a new file is created for each logging source. This simplifies the concurrent reading and
 * writing of records: allow reading only from past hours, thus avoiding concurrent reads and writes on the same
 * file.
 *
 * The file name structure:
 * MMDDHHXX.KVF
 * where XX is the increment number (defaulting 00), if more files are created within the same calendar hour.
 */

#define KVF_RECORD_UNIXT_LEN 			8
#define KVF_RECORD_VECID_LEN 			1
#define KVF_RECORD_VECSIZE_LEN 			1
#define KVF_RECORD_HEADLEN				(KVF_RECORD_UNIXT_LEN + KVF_RECORD_VECID_LEN + KVF_RECORD_VECSIZE_LEN)
#define KVF_HEADLEN						1

#define KVF_VERSION						1

#define STOR_STACK_BUFFSZ 			64

typedef struct {
	uint64_t hour_starts;	// ms
	FIL file;
	const char * const folder_path;
//	const STOR_TGT target;
	char file_name[13];
//	uint8_t is_open;

} StorageTarget;

typedef struct {
	uint64_t unix_ms;
	uint8_t vector_id;
	uint8_t vector_size;

} kvf_record_head;

/*
 * KVF checks
 */
enum {
	KVFCHK_OK,
	KVFCHK_HCORRUPT,
	KVFCHK_HNOMATCH,
	KVFCHK_EMPTY,
	KVFCHK_MISALIGN,
	KVFCHK_INTERN
};

/*
 * !!! used for mapping !!!
 */
 enum {
	STOR_E_W,
	STOR_E_R,
	STOR_E_OPEN,
	STOR_E_CLOSE,
	STOR_E_NULL,
	STOR_E_TRUNC,
	STOR_E_SEEK,
	STOR_E_DFMT_UNKNOWN,

	STOR_E_ENUM_SIZE
};

/*
 * labels for the specific errors, used for debugging.
 */

static const char * const stor_e_labels[STOR_E_ENUM_SIZE] = {
		"writing",
		"reading",
		"opening",
		"closing",
		"",
		"truncating",
		"seeking",
		"parsing datafmt"
};

static inline STOR_R open_wFile(
		STOR_TGT target,
		uint64_t curr_unix_ms,
		char buff[]);
static inline STOR_R kvf_write_record(STOR_TGT target, stor_data_t data);
static inline unsigned kvf_create_filename(
		time_tlStruct *utc_time,
		char * dest,
		uint8_t fname_inc);
static inline int utoa_padd(uint32_t u, char *dest, uint8_t padd);
static inline STOR_R kvf_write_header(STOR_TGT target, char *buff);
static void strcpy_padd(char *dest, const char *source, unsigned dest_size);

static  StorageTarget tgt_list[STOR_TGT_ENUMSIZE] = {
		{
				.folder_path = "0:/DATA"
		},
		{
				.folder_path = "0:/SYS"

		},
		{
				.folder_path = "0:/CTRL"
		}
};


/*
 * Pointer to the specific error string.
 */
const char *stor_e_str = "";

/*
 * File system
 */
static FATFS fs;

/*
 * Readonly, thread exclusive!!!
 */
DIR dir_read;
FIL fil_read;
FILINFO fil_to_read_info;
STOR_TGT read_target;
uint8_t stor_get_isInit;

/*
 * Status of storage system
 */
static volatile uint8_t stor_status = 0;

/*
 * Init the file system.
 */


STOR_R stor_init(void) {
	FRESULT res;

	res = f_mount(&fs, "", 1);
	if (res != FR_OK) {
		return STOR_NA;
	}

	for (unsigned i = 0; i < STOR_TGT_ENUMSIZE; i++) {
		tgt_list[i].file_name[0] = 0;
		tgt_list[i].file_name[13 - 1] = 0;
	}

	asm volatile("": : :"memory");	// memory barrier
	stor_status = 1;
	return STOR_OK;
}


/*
 * CAUTION: this function uses the global file structure fil_read !!!
 */
STOR_R stor_get_config(
		const char *cfg_label,
		char *cfg_data,
		uint8_t size)
{
	if (!stor_status) return STOR_NA;

	FRESULT res;
	char sbuff[STOR_STACK_BUFFSZ];
	res = f_open(&fil_read, "0:/conf.kcf", FA_READ);
	if (res != FR_OK) return STOR_IO;

	char *bp;
	bp = f_gets(sbuff, STOR_STACK_BUFFSZ, &fil_read);
	f_close(&fil_read);
	if (bp != sbuff) return STOR_IO;

	bp = strstr(sbuff, cfg_label);
	if (bp != sbuff) return STOR_BAD_CFGLAB;

	bp += strlen(cfg_label);
	if (*bp != '=') return STOR_BAD_CFGLAB;

	bp++;
	uint8_t conf_len = strlcpy(cfg_data, bp, size);

	if (conf_len && conf_len <= size) {
		if (cfg_data[conf_len - 1] == '\n') cfg_data[conf_len - 1] = 0;
	}
	return STOR_OK;
}
/*
 * Close all files associated with all targets.
 * Unmounts the file system.
 */

STOR_R stor_deinit(void)
{
	for(unsigned target = 0; target < STOR_TGT_ENUMSIZE; target++) {
		if(tgt_list[target].file_name[0]) {
			f_close(&tgt_list[target].file);
		}

		tgt_list[target].file_name[0] = 0;

	}

	f_mount(NULL, "", 1);
	stor_status = 0;
	return STOR_OK;
}

STOR_R stor_put(STOR_TGT target, stor_data_t data)
{
	uint32_t temp_ms; // ms since the day started
	//uint64_t curr_unix_ms;
	char buff[STOR_STACK_BUFFSZ];
	FRESULT res;
	STOR_R stor_res;

	if (target >= STOR_TGT_ENUMSIZE)
		return STOR_INTERN;

	if (time_get_unix_ms(&data.unix_ms) != TL_OK) {
		stor_e_str = stor_e_labels[STOR_E_NULL];
		return STOR_NOTIME;
	}

	check_open:

	if (!tgt_list[target].file_name[0]) {
		stor_res = open_wFile(target, data.unix_ms, buff);
		if (stor_res != STOR_OK) {
			return stor_res;
		}
	}

	temp_ms = data.unix_ms - tgt_list[target].hour_starts;

	if (temp_ms >= 3600000UL) {
		// new hour
		res = f_close(&tgt_list[target].file);

		if (res != FR_OK) {
			stor_e_str = stor_e_labels[STOR_E_CLOSE];
			return STOR_IO;
		}

		tgt_list[target].file_name[0] = 0;
		goto check_open;
	}

	stor_res = kvf_write_record(target, data);
	if (stor_res != STOR_OK) {
		return stor_res;
	}

	res = f_sync(&tgt_list[target].file);

	if (res != FR_OK) {
		stor_e_str = stor_e_labels[STOR_E_W];
		return STOR_IO;
	}

	return STOR_OK;

}

STOR_R stor_get_init(STOR_TGT target) {
	FRESULT ff_res;
	char namebuff[32];

	if (target >= STOR_TGT_ENUMSIZE)
		return STOR_INTERN;

	if (stor_status == 0) {
		return STOR_NA;
	}


	ff_res = f_opendir(&dir_read, tgt_list[target].folder_path);
	if (ff_res != FR_OK) {
		return STOR_IO;
	}

stor_get_init_chk_existing:

	ff_res = f_readdir(&dir_read, &fil_to_read_info);
	if (ff_res != FR_OK) {
		ff_res = f_closedir(&dir_read);
		return STOR_IO;
	}

	if (fil_to_read_info.fname[0] == 0) {
		ff_res = f_closedir(&dir_read);
		return STOR_NA;
	}

	strcpy(namebuff, tgt_list[target].folder_path);
	strcat(namebuff, "/");
	strcat(namebuff, fil_to_read_info.fname);

	ff_res = f_open(&fil_read, namebuff, FA_READ);
	switch (ff_res) {
	case FR_OK:
		/*
		 * We've found a file
		 */
		break;

	case FR_LOCKED:
	case FR_TOO_MANY_OPEN_FILES:
	case FR_NO_FILE:
		/*
		 * We are trying to access the file currently being written.
		 * Check for the next entry.
		 */
		goto stor_get_init_chk_existing;

	default:
		ff_res = f_closedir(&dir_read);
		return STOR_IO;

	}

	ff_res = f_closedir(&dir_read);
	if (ff_res != FR_OK) {
		return STOR_IO;
	}

	// skip header
	UINT readcnt;
	char dummy[KVF_HEADLEN];
	ff_res = f_read(&fil_read, dummy, KVF_HEADLEN, &readcnt);
	if (ff_res != FR_OK) {
		return STOR_IO;
	}

	if (readcnt != KVF_HEADLEN) {
		// file empty. close and unlink.
		ff_res = f_close(&fil_read);
		if (ff_res != FR_OK) {
			return STOR_IO;
		}

		ff_res = f_unlink(namebuff);
		if (ff_res != FR_OK) {
			return STOR_IO;
		}
		/*
		 * Check for the next file.
		 */
		ff_res = f_opendir(&dir_read, tgt_list[target].folder_path);
		if (ff_res != FR_OK) {
			return STOR_IO;
		}
		goto stor_get_init_chk_existing;

	}

	read_target = target;
	stor_get_isInit = 1;
	return STOR_OK;
}

STOR_R stor_get(stor_data_t *data)
{
	FRESULT ff_res;
	UINT read_cnt;

	if (!stor_get_isInit) {
		return STOR_NA;
	}

	/*
	 * Can be static, as there is at most one file being read at any given time.
	 */
	static union {
		kvf_record_head as_head;
		char as_array[sizeof(kvf_record_head)];
	} rec_h;

	ff_res = f_read(&fil_read, rec_h.as_array, KVF_RECORD_HEADLEN, &read_cnt);
	if (ff_res != FR_OK) {
		return STOR_IO;
	}

	if (read_cnt != KVF_RECORD_HEADLEN) {
		return STOR_NA;
	}

	if (rec_h.as_head.vector_size > data->buff_size) {
		/*
		 * The given buffer is too small.
		 */
		return STOR_MEM;
	}

	ff_res = f_read(&fil_read, data->raw_data, rec_h.as_head.vector_size, &read_cnt);
	if (ff_res != FR_OK) {
		return STOR_IO;
	}

	if (read_cnt != rec_h.as_head.vector_size) {
		return STOR_NA;
	}

	data->unix_ms = rec_h.as_head.unix_ms;
	data->vector_id = rec_h.as_head.vector_id;
	data->data_size = rec_h.as_head.vector_size;

	return STOR_OK;
}

/*
 * Ends the reading session.
 * If parameter del_file is > 0, the file that was read will also be deleted.
 */
STOR_R stor_get_deinit(uint8_t del_file) {
	FRESULT ff_res;
	char namebuff[32];

	if(!stor_get_isInit) {
		return STOR_OK;
	}
	stor_get_isInit = 0;

	ff_res = f_close(&fil_read);
	if (ff_res != FR_OK) {
		return STOR_IO;
	}


	if (del_file) {
		strcpy(namebuff, tgt_list[read_target].folder_path);
		strcat(namebuff, "/");
		strcat(namebuff, fil_to_read_info.fname);

		ff_res = f_unlink(namebuff);
		if (ff_res != FR_OK) {
			return STOR_IO;
		}

	}

	return STOR_OK;
}

static inline STOR_R open_wFile(
		STOR_TGT target,
		uint64_t curr_unix_ms,
		char buff[])
{
	FRESULT ff_res;
	STOR_R stor_res;
	time_tlStruct curr_utc = {0};
	uint8_t fname_inc = 0;
	uint64_t curr_unix_s = curr_unix_ms / 1000;

	time_unix_to_utc(curr_unix_s, &curr_utc);

owf_create:

	kvf_create_filename(&curr_utc, tgt_list[target].file_name, fname_inc);

	strcpy(buff, tgt_list[target].folder_path);
	strcat(buff, "/");
	strcat(buff, tgt_list[target].file_name);

	ff_res = f_open(&tgt_list[target].file, buff, FA_WRITE | FA_CREATE_NEW);

	switch(ff_res) {
	case FR_OK:
		tgt_list[target].hour_starts = (curr_unix_s - (curr_utc.min * 60 + curr_utc.sec)) * 1000ULL;

		stor_res =  kvf_write_header(target, buff);
		if (stor_res != STOR_OK) {
			return stor_res;
		}

		return STOR_OK;
	case FR_EXIST:
		fname_inc = (fname_inc + 1) % 100;
		if (!fname_inc) {
			tgt_list[target].file_name[0] = 0;
			return STOR_NAMESPACE;
		}

		goto owf_create;

	default:
		tgt_list[target].file_name[0] = 0;
		return STOR_IO;
	}
}

/*
 * Creates the file name in the destination buffer and returns its size
 */
static inline unsigned kvf_create_filename(time_tlStruct *utc_time, char * dest, uint8_t fname_inc)
{
	/*
	 *  MMDDHHXX.KVF
	 */
	utoa_padd(utc_time->month, 	dest + 0, 2);
	utoa_padd(utc_time->day, 	dest + 2, 2);
	utoa_padd(utc_time->hour, 	dest + 4, 2);
	utoa_padd(fname_inc, 		dest + 6, 2);
	strcpy_padd(dest + 8, ".KVF", 5);

	return 12;
}

/*
 * Converts unsigned integer to array, padding with leading 0 to create a fixed length string of size padd;
 * If the input exceeds the padding size, only the padd least significant digits are converted.
 *
 * Returns 0 on success, -1 on input size exceeding padding length.
 */
static inline int utoa_padd(uint32_t u, char *dest, uint8_t padd)
{
	uint32_t r;
	dest[padd] = '\0';

	while (u) {
		if (!padd) return -1;
		padd--;

		r = u % 10;
		u /= 10;

		dest[padd] = r + '0';

	}

	while (padd) {
		dest[--padd] = '0';
	}

	return 0;
}


/*
 * Writes a record to the file associated with the target.
 */
static inline STOR_R kvf_write_record(STOR_TGT target, stor_data_t data)
{
	FRESULT ff_res;
	UINT written;

	union {
		kvf_record_head as_head;
		char as_array[sizeof(kvf_record_head)];
	} rec_h;

	rec_h.as_head.unix_ms = data.unix_ms;
	rec_h.as_head.vector_id = data.vector_id;
	rec_h.as_head.vector_size = data.data_size;

	ff_res = f_write(&tgt_list[target].file, rec_h.as_array, KVF_RECORD_HEADLEN, &written);

	if (ff_res != FR_OK) return STOR_IO;
	if (written != KVF_RECORD_HEADLEN) return STOR_FULL;

	/*
	 * Write data
	 */
	ff_res = f_write(&tgt_list[target].file, data.raw_data, data.data_size, &written);
	if (ff_res != FR_OK) return STOR_IO;
	if (written != data.data_size) return STOR_FULL;

	return STOR_OK;
}

/*
 * Writes the header of a KVF file.
 * Precondition:
 * The file associated with the log source is open and of size 0
 */
static inline STOR_R kvf_write_header(STOR_TGT target, char *buff)
{
	FRESULT ff_res;
	UINT written;

	/*
	 * Write filetype version
	 */
	buff[0] = KVF_VERSION;

	ff_res = f_write(&tgt_list[target].file, buff, 1, &written);

	if (ff_res != FR_OK) return STOR_IO;
	if (written != 1) return STOR_FULL;

	return STOR_OK;
}

/* Copies the source string into dest, but no more than dest_size.
 * If the dest length < dest_size, it appends \0
 * WARNING: If the dest lenghth >= dest_size, it stops, and no \0 is appended!
 */
static void strcpy_padd(char *dest, const char *source, unsigned dest_size)
{
	unsigned i;
	for (i = 0; i < dest_size && source[i] != 0; i++)
		dest[i] = source[i];

	for(;i < dest_size; i++) {
		dest[i] = 0;
	}
}
