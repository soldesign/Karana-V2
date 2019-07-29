/* Src/modbus_uart/modbus_uart.h
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

#ifndef MODBUS_UART_H
#define MODBUS_UART_H

#include <stdint.h>

typedef enum {
	MODBUS_RMHLD	= 0x03,
	MODBUS_RI		= 0x04,
	MODBUS_WMHLD	= 0x10,
	MODBUS_DIA		= 0x08
} modbus_fnc;

typedef struct {
	uint8_t slave_addr;

} modbusInstanceStruct;

/*
 * Minimum waiting time between the end of the last reveived frame
 * and the beginning of the next frame to be sent to another slave than
 * the previous. Required for some devices.
 */
#define SLAVE_CHANGES_TIMEBUF 15 // ms

void modbus_init(void);
int modbus_sendRaw(const unsigned char* data, unsigned int size, const modbusInstanceStruct *minstance);
int modbus_readMHld(uint16_t start_reg_addr, uint16_t reg_cnt, const modbusInstanceStruct *minstance, uint16_t *content);
int modbus_readInput(uint16_t start_addr, uint16_t reg_cnt, const modbusInstanceStruct *minstance, uint16_t *content);
int modbus_writeMHld(uint16_t start_addr, uint16_t reg_cnt, const uint16_t* data, const modbusInstanceStruct *minstance);
int modbus_readInput_list(const uint16_t *reg_list, uint16_t *data, uint8_t cnt, const modbusInstanceStruct *minstance);
int modbus_readMHld_list(const uint16_t *reg_list, uint16_t *data, uint8_t cnt, const modbusInstanceStruct *minstance);

#endif /* MODBUS_UART_H */
