/* Src/srcio_module_modbus/SourceIOMod_mbus.c
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


#include "SourceIOMod_mbus.h"

static uint16_t log_buff[64]; // required for the modbus uart lib

static SRCIO_R _SourceIOMod_mbus_update(SourceIOMod *_this)
{
	/*
	 * Cast to our specific instance
	 */
	SourceIOMod_mbus *__this = (SourceIOMod_mbus*)_this;

	int8_t res = 0;

	/*
	 * Read the input registers from IO
	 */
	res = modbus_readInput_list(__this->iregs, log_buff, __this->iregs_cnt, &__this->minstance);
	if (res != 0) {
		_this->state = SRCIO_MOD_NA;
		return SRCIO_IOERR;
	}

	/*
	 * Store the input in the data components registers.
	 * Acquire the lock to guarantee atomicity across all the input registers.
	 * Remeber, this is not required if atomicity is not relevant.
	 */
	SourceIOMod_lock_acq(_this);
	for (unsigned i = 0; i < __this->iregs_cnt; i++) {
		_this->comp_list[i].comp.comp_u16 = log_buff[i];
	}
	SourceIOMod_lock_rel(_this);

	/*
	 * To the same for holding registers
	 */
	res = modbus_readMHld_list(__this->hregs, log_buff, __this->hregs_cnt, &__this->minstance);
	if (res != 0) {
		_this->state = SRCIO_MOD_NA;
		return SRCIO_IOERR;
	}

	SourceIOMod_lock_acq(_this);
	for (unsigned i = 0; i < __this->hregs_cnt; i++) {
		_this->comp_list[i + __this->iregs_cnt].comp.comp_u16 = log_buff[i];
	}
	SourceIOMod_lock_rel(_this);

	_this->state = SRCIO_OK;
	return SRCIO_OK;
}


SRCIO_R SourceIOMod_mbus_construct(SourceIOMod_mbus * _this, const SourceIOMod_mbus_initStruct *initstr)
{

	SourceIOMod_init_t srciom_init; // init struct for the SourceIOMod abstract class

	/*
	 * Calculate the number of components and allocate the memory required.
	 */
	srciom_init.comp_cnt = initstr->iregs_cnt + initstr->hregs_cnt;
	srciom_init.comp_list = pvPortMalloc(srciom_init.comp_cnt * sizeof(data_comp));
	srciom_init.label = initstr->label;

	if (srciom_init.comp_list == NULL)
		return SRCIO_MEM;
	/*
	 * Set all components to unsigned short and update their labels. This step is important,
	 * because the construct function of the super class also calculates the raw size of
	 * the data.
	 */
	for (unsigned i = 0; i < srciom_init.comp_cnt; i++) {
		srciom_init.comp_list[i].comp_dtype = DTYPE_U16;
		srciom_init.comp_list[i].label = initstr->label_list[i];
	}
	/*
	 * Set as ID the modbus address of the device
	 */
	srciom_init.ID = initstr->mbaddr;
	/*
	 * Set the log/update/ctrl periods, in ms
	 */
	srciom_init.log_period_ms = initstr->log_per_ms;
	srciom_init.update_period_ms = initstr->update_per_ms;
	srciom_init.ctrl_period_ms = initstr->ctrl_per_ms;
	/*
	 * Now construct the super class, SourceIOMod, by casting the pointer accordingly.
	 */
	SRCIO_R srciom_r = SourceIOMod_construct((SourceIOMod*)_this, &srciom_init);
	if (srciom_r != SRCIO_OK) return srciom_r;
	/*
	 * Now that the super class is constructed, we can continue with the specialization
	 *
	 * Initialize the modbus library
	 */
	modbus_init();
	/*
	 * Create the new virtual method table
	 */
	static SourceIOMod_VMT const vmt = {
			/*
			 * We created our own implementation of the update function,
			 * so we point to it in the VMT
			 */
			._SourceIOMod_update = &_SourceIOMod_mbus_update,
			/*
			 * We don't use the control, so we can use the default function
			 */
			._SourceIOMod_ctrl = &_SourceIOMod_ctrl
	};

	_this->_super.vmt = &vmt;	// override the VMT
	_this->iregs = initstr->iregs;
	_this->hregs = initstr->hregs;
	_this->iregs_cnt = initstr->iregs_cnt;
	_this->hregs_cnt = initstr->hregs_cnt;
	_this->minstance.slave_addr = initstr->mbaddr;

	return SRCIO_OK;
}
