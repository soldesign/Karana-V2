/* Src/modbus_uart/modbus_uart_io.h
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

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "usart.h"
#include "dma.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_usart.h"
#include "time_loc.h"

/*
 * Hardware specific I/O implementation for the Modbus
 */

#define _delay_read(__size) (__size * 3)

static inline int _modbus_uart_write(const char *data, unsigned int size)
{
	return UART1_write(data, size);
}

static inline int _modbus_uart_read(char* data, unsigned int size, unsigned timeout)
{
	return UART1_read(data, size, timeout);
}

/*
 * Locks for concurrent access on the bus.
 */

static osMutexId modbus_mutex;
static osMutexDef(modbus_mutex);

static inline void _modbus_lock_acq(void)
{
	osMutexWait(modbus_mutex, osWaitForever);
}

static inline void _modbus_lock_rel(void)
{
	osMutexRelease(modbus_mutex);
}

static inline void _modbus_lock_init(void)
{
	modbus_mutex = osMutexCreate(osMutex(modbus_mutex));
}

static tout_t modbus_chaddr_timer;
static inline void _modbus_start_chaddr_timer(unsigned ms)
{
	time_nobl_wait_start(ms, &modbus_chaddr_timer);
}

static inline void _modbus_wait_chaddr_timer(void)
{
	unsigned ms_left = time_nobl_wait_left(&modbus_chaddr_timer);
	ms_left ? osDelay(ms_left) : time_nobl_wait_start(0, &modbus_chaddr_timer);
}
