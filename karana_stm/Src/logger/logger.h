/* Src/logger/logger.h
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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdint.h>
#include "data_pack.h"

typedef enum {
	LOG_OK,
	LOG_IOERR,
	LOG_FULL,
	LOG_NA,
	LOG_NOTIME,	// time not available
	LOG_INTERN	// internal error

} LOG_R;

/*
 * !!! used for mapping !!!
 */
typedef enum {
	LOGSRC_SYS,
	LOGSRC_DATA,
	LOGSRC_CTRL,

	LOGSRC_ENUM_SIZE
} log_source;

typedef struct {
	log_source src;
	uint8_t id;
} log_instance;

void startLogger_thread(void const* arg);

#endif
