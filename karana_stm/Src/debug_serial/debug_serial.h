/* Src/debug_serial/debug_serial.h
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


#ifndef DEBUG_SERIAL_DEBUG_SERIAL_H_
#define DEBUG_SERIAL_DEBUG_SERIAL_H_

#include "cmsis_os.h"

typedef struct {
	const char *name;
	char buff[32];
} dbgs_inst_t;

void dbgs_init(void);
void dbgs_println(dbgs_inst_t *dbgs_inst, const char* msg);

#endif /* DEBUG_SERIAL_DEBUG_SERIAL_H_ */
