/* Src/srcio_module_windspeed/SourceIOMod_windspeed.c
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

#include "SourceIOMod_windspeed.h"
#include "cmsis_os.h"

#define WSP_LOG_PER_MS 1000
#define WSP_UPD_PER_MS 1000

extern unsigned get_windspeed(void);
extern void init_windspeed_pin(void);
static SRCIO_R windspeed_update(SourceIOMod *_this)
{
	_this->comp_list[0].comp.comp_u16 = get_windspeed();
	_this->state = SRCIO_OK;
	return SRCIO_OK;
}

SRCIO_R SourceIOMod_windspeed_construct(SourceIOMod_windspeed *_this)
{
	SRCIO_R res;
	SourceIOMod_init_t init;

	init_windspeed_pin();

	init.comp_cnt = 1;
	init.comp_list = pvPortMalloc(init.comp_cnt * sizeof(data_comp));
	if (init.comp_list == NULL) return SRCIO_MEM;

	init.comp_list[0].comp_dtype = DTYPE_U16;
	init.comp_list[0].label = "wsp";

	init.ID = 0xF0;
	init.label = "wspm";

	init.ctrl_period_ms = SRCIOM_PERIOD_INFTY;	// we don't have any control
	init.log_period_ms = WSP_LOG_PER_MS;
	init.update_period_ms = WSP_UPD_PER_MS;

	res = SourceIOMod_construct((SourceIOMod*)_this, &init);

	static const SourceIOMod_VMT vmt = {
			._SourceIOMod_update = &windspeed_update,
			._SourceIOMod_ctrl = _SourceIOMod_ctrl
	};


	_this->_super.vmt = &vmt;

	return res;
}



