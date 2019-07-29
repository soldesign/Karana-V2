/* Src/ctrl/ctrl.c
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

#include "ctrl.h"
#include "SourceIO_instances.h"
#include "time_loc.h"
#include "debug_serial.h"
#include "SourceIOMod.h"


void start_ctrl_task(void const *arg)
{
	static dbgs_inst_t dbgs_ctrl = {.name = "CTRL"};
	static uint32_t mod_ctrl_loop_left, mod_ctrl_left;
	static SRCIO_R srcio_r;
	/*
	 * Wait for the SourceIO modules to be initialized
	 */

	while ( SrcIO_isReady() != SRCIO_OK) {
		osDelay(10);
	}

	while (1) {
		for (unsigned i = 0; i < SrcIO_mod_cnt(); i++) {
			mod_ctrl_left = time_nobl_wait_left(&srcio_mods[i]->ctrl_period);

			if (!mod_ctrl_left) {
				srcio_r = SourceIOMod_ctrl(srcio_mods[i]);
				if (srcio_r == SRCIO_OK)
					dbgs_println(&dbgs_ctrl, "OK");
				else
					dbgs_println(&dbgs_ctrl, "not OK");
			}
		}

		mod_ctrl_loop_left = 0xFFffFFff;
		for (unsigned i = 0; i < SrcIO_mod_cnt(); i++) {
			mod_ctrl_left = time_nobl_wait_left(&srcio_mods[i]->ctrl_period);
			if (mod_ctrl_left < mod_ctrl_loop_left)
				mod_ctrl_loop_left = mod_ctrl_left;
		}
		/*
		 * Wait the minimum time left before a module needs to be updated
		 */
		osDelay(mod_ctrl_loop_left);
	}


}
