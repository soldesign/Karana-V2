/* Src/logger/Storage.h
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

#ifndef LOGGER_STORAGE_H_
#define LOGGER_STORAGE_H_

#include <stdint.h>

typedef enum {
	STOR_OK,
	STOR_IO,
	STOR_FULL,
	STOR_NA,
	STOR_NOTIME,
	STOR_INTERN,
	STOR_NAMESPACE,
	STOR_MEM,
	STOR_BAD_CFGLAB

} STOR_R;

/*
 * Used for mapping! Do not change order!!
 */
typedef enum {
	STOR_TGT_DATA,
	STOR_TGT_SYS,
	STOR_TGT_CTRL,

	STOR_TGT_ENUMSIZE
} STOR_TGT;

typedef struct {
	char *raw_data;
	unsigned buff_size;
	unsigned data_size;
	uint64_t unix_ms;
	uint8_t vector_id;

} stor_data_t;

extern STOR_R stor_init(void);
extern STOR_R stor_get_config(
		const char *cfg_label,
		char *cfg_data,
		uint8_t size);
extern STOR_R stor_deinit(void);
extern STOR_R stor_put(STOR_TGT target, stor_data_t data);
extern STOR_R stor_get_init(STOR_TGT target);
extern STOR_R stor_get(stor_data_t *data);
extern STOR_R stor_get_deinit(uint8_t success);

extern const char* stor_e_str;

#endif /* LOGGER_STORAGE_H_ */
