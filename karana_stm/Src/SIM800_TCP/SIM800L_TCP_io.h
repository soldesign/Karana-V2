/* Src/SIM800_TCP/SIM800L_TCP_io.h
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

#ifndef SIM800_TCP_SIM800L_TCP_IO_H_
#define SIM800_TCP_SIM800L_TCP_IO_H_

#include "SIM800L_TCP.h"

#include "usart.h"
#include "cmsis_os.h"
#include "debug_serial.h"

struct SIM800L_TCP_iostruct {
	int (*write)(const char *data, unsigned int size);
	int (*read)(char* data, unsigned int size, unsigned timeout);
	int (*readln)(char *data, unsigned int max_size, unsigned timeout);
	void (*clr)(void);
	void (*set_sim_state)(SIM800L_state state);
	void (*debug_println)(char *str);
	void (*delay_ms)(uint32_t ms);
};

static void sim_set_state(SIM800L_state state)
{
	if (state == SIM800L_ON) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	}
}

static void uart_dbg_println(char *str)
{
	static dbgs_inst_t sim_dbgs_inst = { .name = "SIM800L" };
	dbgs_println(&sim_dbgs_inst, str);
}

static inline void os_delay_ms(uint32_t ms)
{
	osDelay(ms);
}

static const struct SIM800L_TCP_iostruct SIM800L_io = {
	.write = UART6_write,
	.read = UART6_read,
	.readln = UART6_readln,
	.clr = UART6_clr,
	.set_sim_state = sim_set_state,
	.debug_println = uart_dbg_println,
	.delay_ms = os_delay_ms
};

#endif /* SIM800_TCP_SIM800L_TCP_IO_H_ */
