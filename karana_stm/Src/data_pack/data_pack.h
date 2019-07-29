/* Src/data_pack/data_pack.h
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


/*
 * Type definitions for data sets shared between modules.
 */
#ifndef DATA_PACK_DATA_PACK_H_
#define DATA_PACK_DATA_PACK_H_

#include <stdint.h>
#include "cmsis_os.h"

// each component  has a type
typedef enum {
	DTYPE_U16,
	DTYPE_I16,
	DTYPE_U32,
	DTYPE_I32,
	DTYPE_ENUMSIZE
} data_type;


/*
 * A component container
 */
typedef struct {
	union component {
		uint16_t	comp_u16;
		int16_t		comp_i16;
		uint32_t 	comp_u32;
		int32_t		comp_i32;
	} 	comp __attribute__ ((aligned (4))); // ensure atomic accesses

	const char* label;			// label (InfluxDB: field key). Keep it as short as possible!
	data_type 	comp_dtype; 	// data type of the component

} data_comp;



#endif /* DATA_PACK_DATA_PACK_H_ */
