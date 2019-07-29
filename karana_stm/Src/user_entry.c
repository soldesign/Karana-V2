/* Src/user_entry.c
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

#include "user_entry.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "usart.h"
//#include "gpio.h"
//#include "spi.h"
//#include  <errno.h>
//#include  <sys/unistd.h>
#include "logger.h"
//#include "time_loc.h"
#include "SourceIO_thd.h"
#include "net.h"
#include "debug_serial.h"
#include "ctrl.h"


/*
 * The CMSIS-OS API defines stack size in bytes. However, Free-RTOS defines
 * stack size as multiple of StackType_t, which is 4 bytes. If we follow the
 * osThreadCreate() CMSIS-OS API call, we end up in xTaskCreate(), which gets
 * called with the stack size parameter declared below. In xTaskCreate() we find
 * the following memory allocation call:
 *
 * pxStack = ( StackType_t * ) pvPortMalloc( ( ( ( size_t ) usStackDepth ) * sizeof( StackType_t ) ) );
 *
 * which clearly proves my point...
 */
static osThreadId logger_threadHandle;
static osThreadDef(logger_thread, startLogger_thread, osPriorityNormal, 0, 2048 / 4);

static osThreadId SrcIO_threadHandle;
static osThreadDef(SrcIO_thread, start_SourceIO_task, osPriorityNormal, 0, 2048 / 4);

static osThreadId net_threadHandle;
static osThreadDef(net_thread, startNetTask, osPriorityBelowNormal, 0, 2048 / 4);

static osThreadId ctrl_threadHandle;
static osThreadDef(ctrl_thread, start_ctrl_task, osPriorityNormal, 0, 2048 / 4);


void user_entry(void)
{
	UART2_user_init();
	UART6_user_init();

	dbgs_init();

	SrcIO_threadHandle = osThreadCreate(osThread(SrcIO_thread), NULL);
	ctrl_threadHandle = osThreadCreate(osThread(ctrl_thread), NULL);
	logger_threadHandle = osThreadCreate(osThread(logger_thread), NULL);
	net_threadHandle = osThreadCreate(osThread(net_thread), NULL);
}

