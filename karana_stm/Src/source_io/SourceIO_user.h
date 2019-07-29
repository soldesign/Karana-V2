/* Src/source_io/SourceIO_user.h
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

#include "SourceIO_instances.h"
#include "SourceIOMod_mbus.h"

/* Step 1:
 * Declare here the module instances
 */
static SourceIOMod_mbus mbdev_10;

/*
 * Step 2:
 * Register here the modules declared above, casted as SourceIOMod pointers
 */
SourceIOMod * const srcio_mods[] = {
		(SourceIOMod*)&mbdev_10
};

/*
 * Step 3:
 * Initialize all the module instances here.
 *
 * @ret		SRCIO_OK, if all instances were initializes successfully,
 * 	otherwise the error code of the module that failed
 *
 */
SRCIO_R SrcIO_user_init(void)
{
	SRCIO_R srcio_r;

	static const uint16_t mbdev_10_ireg_addr[] = {0x10, 0x11};
	static const uint16_t mbdev_10_hreg_addr[] = {0x23};
	static const char *mbdev_10_reg_lab[] = {"i_10", "i_11", "h_23"};

	SourceIOMod_mbus_initStruct mbus_initsct;

	mbus_initsct.mbaddr = 0x10;
	mbus_initsct.iregs = mbdev_10_ireg_addr;
	mbus_initsct.iregs_cnt = 2;
	mbus_initsct.hregs = mbdev_10_hreg_addr;
	mbus_initsct.hregs_cnt = 1;
	mbus_initsct.label_list = mbdev_10_reg_lab;
	mbus_initsct.update_per_ms = 1000;              // update each sec
	mbus_initsct.log_per_ms = 1000;                 // store each sec
	mbus_initsct.ctrl_per_ms = SRCIOM_PERIOD_INFTY; // we don't use control
	mbus_initsct.label = "dev_10";

	srcio_r = SourceIOMod_mbus_construct(&mbdev_10, &mbus_initsct);
    if (srcio_r != SRCIO_OK) return srcio_r;

	return SRCIO_OK;
}
