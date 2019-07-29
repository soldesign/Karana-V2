/* Src/time/time_loc.h
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

#ifndef TIME_LOC_H_
#define TIME_LOC_H_
#include <stdint.h>

typedef enum {
	TL_OK,
	TL_TIME_NA,
	TL_LOC_NA
} TL_R;

typedef struct time_tlStruct {
	float longitude;
	float latitude;

	uint8_t year; // year - 2000
	uint8_t month;
	uint8_t weekd; // weekday, Monday is 1
	uint8_t day;

	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint16_t msec;

} time_tlStruct;

typedef struct tout_t {
	uint32_t start_ms;
	uint32_t timeout_ms;
} tout_t;

void time_init(time_tlStruct *tloc);
TL_R time_get_unix_ms(uint64_t *unixt_ms);
uint32_t time_utc_to_unix (time_tlStruct* timestamp);
void time_unix_to_utc(uint32_t curr_unix, time_tlStruct* timestamp);
void time_nobl_wait_start(uint32_t ms, tout_t* timeout_struct);
uint8_t time_nobl_wait_ovt(tout_t* timeout_struct);
uint32_t time_nobl_wait_left(tout_t *timeout_struct);

#endif /* TIME_LOC_H_ */
