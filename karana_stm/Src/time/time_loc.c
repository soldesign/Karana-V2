/* Src/time/time_loc.c
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

#include "time_loc.h"
#include "cmsis_os.h"

#define SEC_IN_COMMON 31536000UL
#define SEC_IN_LEAP 31622400UL

#define _millis(void) osKernelSysTick()

static inline uint8_t is_leap (uint16_t year);

// days in a month in a common year, January at index 1
static const uint8_t days[] =  {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static volatile uint32_t time_zero_tick_ms; // [ms]
static volatile uint32_t time_zero_unix_s; // [s]
static volatile char time_ready = 0;

extern osMutexId time_lock;

void time_init(time_tlStruct *tloc)
{
	uint32_t _time_zero_tick_ms = _millis();
	uint32_t _time_zero_unix_s = time_utc_to_unix(tloc);

	osMutexWait(time_lock, 100);
	time_zero_tick_ms = _time_zero_tick_ms;
	time_zero_unix_s = _time_zero_unix_s;
	osMutexRelease(time_lock);

	time_ready = 1;
}

/*
 * Returns UNIX epoch, in ms. Reentrant.
 */

TL_R time_get_unix_ms(uint64_t *unixt_ms)
{
	if(!time_ready)
		return TL_TIME_NA;

	uint32_t _time_zero_tick_ms;
	uint32_t _time_zero_unix_s;

	osMutexWait(time_lock, 100);
	_time_zero_tick_ms = time_zero_tick_ms;
	_time_zero_unix_s = time_zero_unix_s;
	osMutexRelease(time_lock);

	uint32_t tick_now_ms = _millis();
	uint32_t delta_ms = tick_now_ms - _time_zero_tick_ms;

	if (_time_zero_tick_ms > tick_now_ms) {
		/*
		 * Systick overflow
		 */
		delta_ms = 0xFFffFFff - delta_ms + 1;
	}

	*unixt_ms = _time_zero_unix_s * 1000ULL + delta_ms;

	return TL_OK;
}

/*
reentrant
*/
void time_unix_to_utc(uint32_t curr_unix, time_tlStruct* timestamp)
{
	uint32_t sec_curr_year;
	uint32_t sec_curr_month;
	uint16_t curr_year = 1970;

	timestamp->month = 1;
	timestamp->day = 1;

	sec_curr_year = is_leap(curr_year) ? SEC_IN_LEAP : SEC_IN_COMMON;

	while(curr_unix >= sec_curr_year) {
		curr_unix -= sec_curr_year;
		curr_year++;
		sec_curr_year = is_leap(curr_year) ? SEC_IN_LEAP : SEC_IN_COMMON;
	}

	timestamp->year = curr_year - 2000;

	sec_curr_month = (uint32_t)days[timestamp->month] * 24 * 60 * 60;

	while (curr_unix >= sec_curr_month) {
		curr_unix -= sec_curr_month;
		timestamp->month++;
		sec_curr_month = (uint32_t)days[timestamp->month] * 24 * 60 * 60;

		// February of leap year
		if (timestamp->month == 2 && is_leap(curr_year)) {
			sec_curr_month += 24 * 60 * 60;
		}
	}

	timestamp->day = curr_unix / (24 * 60 * 60) + 1;
	curr_unix %= (24 * 60 * 60);

	timestamp->hour = curr_unix / (60 * 60);
	curr_unix %= (60 * 60);

	timestamp->min = curr_unix / 60;
	curr_unix %= 60;

	timestamp->sec = curr_unix;
}

/*
 * Reentrant
 */
uint32_t time_utc_to_unix (time_tlStruct* timestamp)
{
	uint16_t year, month, day, hour, min, sec;
	uint32_t res = 0;
	uint8_t leap_years = 0;

	year = timestamp->year + 2000;
	month = timestamp->month;
	day = timestamp->day;
	hour = timestamp->hour;
	min = timestamp->min;
	sec = timestamp->sec;

	for (uint16_t y = 1970; y < year; y++) {
		leap_years += is_leap(y);
	}

	// seconds in the past leap years
	res += (uint32_t)leap_years * 366 * 24 * 60 * 60;
	// seconds in the past common years
	res += (uint32_t)(year - 1970 - leap_years) * 365 * 24 * 60 * 60;


	// number of seconds in the current year's past months
	for (uint8_t m = 1; m < month; m++) {
		res += (uint32_t)days[m] * 24 * 60 * 60;
		// February of leap year
		if (m == 2 && is_leap(year)) {
			res += 24 * 60 * 60;
		}
	}


	// and so on...
	res += (uint32_t)(day - 1) * 24 * 60 * 60;
	res += (uint32_t)hour * 60 * 60;
	res += min * 60;
	res += sec;

	return res;
}

/*
 * Reentrant
 */
static inline uint8_t is_leap (uint16_t year)
{
	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		return 1;
	return 0;
}

void time_nobl_wait_start(uint32_t ms, tout_t* timeout_struct)
{
	timeout_struct->start_ms = _millis();
	timeout_struct->timeout_ms = ms;
}

/*
 * Returns the timeout_ms condition. Also sets the timer initial value to the time
 * the last timeout_ms happened.
 *
 * If one or more periods are skipped without checking, the initial value will
 * be set to the last period that should have been checked, thus avoiding
 * multiple overtime-true returns.
 *
 * @param
 * tout_t* timeout_struct	-> pointer to a timeout_ms structure that is be used
 * 								for this specific timer
 * @return
 * 1 overtime occurred, 0 overtime not occurred
 */
uint8_t time_nobl_wait_ovt(tout_t* timeout_struct)
{
	uint32_t curr_ms = _millis();
	uint32_t delta_ms;

	if (timeout_struct->timeout_ms == 0) return 1;

	delta_ms = curr_ms - timeout_struct->start_ms;

	if (curr_ms < timeout_struct->start_ms) {
		delta_ms = 0xFFffFFff - delta_ms + 1;
	}

	if (delta_ms < timeout_struct->timeout_ms) {
		return 0;
	}

	timeout_struct->start_ms += delta_ms - delta_ms % timeout_struct->timeout_ms;

	return 1;
}

/*
 * Same as time_nobl_wait_ovt(), but returns the time left to wait instead. If the
 * waiting period elapsed, 0 is returned.
 */
uint32_t time_nobl_wait_left(tout_t *timeout_struct)
{
	uint32_t curr_ms = _millis();
	uint32_t delta_ms;

	if (timeout_struct->timeout_ms == 0) return 0;

	delta_ms = curr_ms - timeout_struct->start_ms;

	if (curr_ms < timeout_struct->start_ms) {
		delta_ms = 0xFFffFFff - delta_ms + 1;
	}

	if (delta_ms < timeout_struct->timeout_ms) {
		return timeout_struct->timeout_ms - delta_ms;
	}


	timeout_struct->start_ms += delta_ms - delta_ms % timeout_struct->timeout_ms;
	return 0;
}
