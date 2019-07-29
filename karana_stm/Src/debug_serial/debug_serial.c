/* Src/debug_serial/debug_serial.c
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


#include "debug_serial.h"
#include "cmsis_os.h"
#include "usart.h"
#include "string.h"

static osMutexId print_mutex;
static osMutexDef(print_mutex);

void dbgs_init(void)
{
	print_mutex = osMutexCreate(osMutex(print_mutex));
}

void dbgs_println(dbgs_inst_t *dbgs_inst, const char* msg)
{

	unsigned i;
	for (i = 0; (i < 32 - 2) && (dbgs_inst->name[i] != 0); i++)
		dbgs_inst->buff[i] = dbgs_inst->name[i];

	dbgs_inst->buff[i++] = ':';
	dbgs_inst->buff[i++] = ' ';

	osMutexWait(print_mutex, osWaitForever);

	UART2_write(dbgs_inst->buff, i);
	UART2_write(msg, strlen(msg));
	UART2_write("\r\n", 2);

	osMutexRelease(print_mutex);
}
