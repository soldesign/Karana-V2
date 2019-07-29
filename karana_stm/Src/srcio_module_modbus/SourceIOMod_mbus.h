/* Src/srcio_module_modbus/SourceIOMod_mbus.h
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

#ifndef SRCIO_MODULE_MODBUS_SOURCEIOMOD_MBUS_H_
#define SRCIO_MODULE_MODBUS_SOURCEIOMOD_MBUS_H_

#include "SourceIOMod.h"
#include "modbus_uart.h"

/*
 * Used for construction of the class.
 * Can be discarded after construction.
 */
typedef struct {
	uint8_t mbaddr;				// slave address of the modbus device
	const char ** label_list;	// list of the labels for both input and holding registers (input first)
	const uint16_t *iregs;		// list of input registers addresses
	const uint16_t *hregs;		// list of holding registers addresses
	uint8_t iregs_cnt;			// number of input registers
	uint8_t hregs_cnt;			// number of holding registers
	uint32_t update_per_ms;
	uint32_t log_per_ms;
	uint32_t ctrl_per_ms;
	const char *label;
	//TODO: add type array

} SourceIOMod_mbus_initStruct;

typedef struct {
	SourceIOMod _super;				// This has to be the first !!!
	modbusInstanceStruct minstance; // modbus instance for the modbus library
	const uint16_t *iregs;			// list of input registers addresses
	const uint16_t *hregs;			// list of holding registers addresses
	uint8_t iregs_cnt;				// number of input registers
	uint8_t hregs_cnt;				// number of holding registers

} SourceIOMod_mbus;

extern SRCIO_R SourceIOMod_mbus_construct(SourceIOMod_mbus * _this, const SourceIOMod_mbus_initStruct *initstr);

#endif /* SRCIO_MODULE_MODBUS_SOURCEIOMOD_MBUS_H_ */
