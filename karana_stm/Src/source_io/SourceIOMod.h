/* Src/source_io/SourceIOMod.h
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
#ifndef SOURCE_IO_SOURCEIOMOD_H_
#define SOURCE_IO_SOURCEIOMOD_H_

#include "SourceIO_defs.h"
#include "data_pack.h"
#include "cmsis_os.h"		// for pvPortMalloc() and mutex
#include "time_loc.h"

typedef struct SourceIOMod SourceIOMod;
typedef struct SourceIOMod_VMT SourceIOMod_VMT;
typedef struct SourceIOMod_init_t SourceIOMod_init_t;

#define SRCIOM_PERIOD_INFTY	0xFFffFFff

struct SourceIOMod {
	/*
	 * Pointer to a Virtual Method Table structure. This holds
	 * function pointers to the methods a inheriting class implements.
	 * Per default, this structure is initialized to empty functions.
	 */
	const SourceIOMod_VMT *vmt;
	/*
	 * This lock can be used to ensure atomic accesses on the structure.
	 * All predefined functions that operate on this class use this lock,
	 * so atomicity is guaranteed, as long as the inheriting class also
	 * uses this lock.
	 */
	const osMutexDef_t os_mutex_def_lock;
	osMutexId lock;
	/*
	 * Pointer to the list of data components.
	 */
	data_comp * comp_list;
	/*
	 * Number of data components
	 */
	uint8_t comp_cnt;
	/*
	 * ID of the instance. Every instance of this class is required to
	 * have an unique ID
	 */
	uint8_t ID;
	/*
	 * Size of the raw, packed data contained by the data components.
	 */
	uint8_t data_raw_size;
	/*
	 * The current state of the instance
	 */
	volatile SRCIO_R state;
	/*
	 * Timers that ensure the update, logging, and controlling of a instance
	 * are done on a regular, periodic basis.
	 */
	tout_t update_period;
	tout_t log_period;
	tout_t ctrl_period;
	/*
	 * A label for the instance (InfluxDB: measurement). Keep it short!
	 */
	const char *label;
};

struct  SourceIOMod_VMT {
	/*
	 * This method updates the values of the module instance by performing
	 * the specific measurements
	 */
	SRCIO_R (*_SourceIOMod_update)(SourceIOMod *_this);
	/*
	 * This method performs control actions on the respective module instance
	 */
	SRCIO_R (*_SourceIOMod_ctrl)(SourceIOMod *_this);
};

struct SourceIOMod_init_t {
	uint8_t ID;					// Unique ID of the instance
	data_comp * comp_list;		// Pointer to the components array
	uint8_t comp_cnt;			// # of components in the array

	/*
	 * How often, in ms...
	 */
	uint32_t update_period_ms;	// the values update function should be called
	uint32_t log_period_ms;		// the values should be logged on the storage device
	uint32_t ctrl_period_ms;	// the control function should be called

	const char *label;			// a label for the instance. Keep it short!
};

/* ==================================================================
 * Do not call these!!!
 */
extern SRCIO_R _SourceIOMod_update(SourceIOMod *_this);
extern SRCIO_R _SourceIOMod_ctrl(SourceIOMod *this);
/*
 * ==================================================================
 */
extern const SourceIOMod_VMT sourceIOMod_VMT;

extern SRCIO_R SourceIOMod_construct(
		SourceIOMod * _this,
		const SourceIOMod_init_t *init_stct);
extern SRCIO_R SourceIOMod_update(SourceIOMod *_this);
extern SRCIO_R SourceIOMod_ctrl(SourceIOMod * _this);
extern SRCIO_R SourceIOMod_getRaw(SourceIOMod *_this, void *dst, uint8_t dst_size);
extern SRCIO_R SourceIOMod_rawToMod(
		SourceIOMod *mod_dst,
		const void *raw_data_src,
		uint8_t raw_data_size);
extern SRCIO_R SourceIOMod_lock_acq(SourceIOMod *_this);
extern void SourceIOMod_lock_rel(SourceIOMod *_this);


#endif /* SOURCE_IO_SOURCEIOMOD_H_ */
