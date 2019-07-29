/* Src/source_io/SourceIO_thd.c
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

#include "SourceIO_thd.h"
#include "SourceIO_instances.h"
#include "debug_serial.h"
#include <string.h>
#include <stdlib.h>

//#define MOD_UPD_LOOP_PER_MS		10
static void print_mod(const SourceIOMod *mod);

static const char * const srcio_r_lab[SRCIO_R_ENUMSIZE] = {
		"OK",		// OK
		"NR",		// not ready
		"NP",		// not present
		"TOUT",		// timeout
		"INTERN",	// internal error
		"IOERR",	// I/O error
		"MEM",		// memory fault
		"MOD_NA",	// a module returned NR, NP, TOUT or IOERR
		"MOD_ALLNA", // all modules returned NR, NP, TOUT or IOERR
		"OS",		// The RTOS failed to fulfill a request
		"BADARGS"
};

static dbgs_inst_t dbgs_srcio = {.name = "SRCIO"};
void start_SourceIO_task(void const* arg)
{
	static uint32_t mod_upd_loop_left, mod_upd_left;
	static SRCIO_R srcio_r;

	srcio_r = SrcIO_init();
	if (srcio_r != SRCIO_OK) while(1) {
		// TODO: trigger some action
		dbgs_println(&dbgs_srcio, "init failed");
		osDelay(2000);
	}

	while (1) {

		for (unsigned i = 0; i < SrcIO_mod_cnt(); i++) {
			mod_upd_left = time_nobl_wait_left(&srcio_mods[i]->update_period);

			if (!mod_upd_left) {
				srcio_r = SourceIOMod_update(srcio_mods[i]);
				if (srcio_r == SRCIO_OK) {
					print_mod(srcio_mods[i]);
				} else {
					dbgs_println(&dbgs_srcio, srcio_r_lab[srcio_r]);
				}
			}
		}

		mod_upd_loop_left = 0xFFffFFff;
		for (unsigned i = 0; i < SrcIO_mod_cnt(); i++) {
			mod_upd_left = time_nobl_wait_left(&srcio_mods[i]->update_period);
			if (mod_upd_left < mod_upd_loop_left)
				mod_upd_loop_left = mod_upd_left;
		}

		/*
		 * Wait the minimum time left before a module needs to be updated
		 */
		osDelay(mod_upd_loop_left + 1);
	}
}

#define print_mod_buff_size 512
static void print_mod(const SourceIOMod *mod)
{
	static char print_mod_buff[print_mod_buff_size];
	static char print_comp_buff[16];
	print_mod_buff[0] = 0;
	char *p = print_mod_buff;
	char *be = print_mod_buff + print_mod_buff_size;

	p = stpncpy(p, mod->label, be - p);
	p = stpncpy(p, ": ", be - p);

	for (unsigned i = 0; i < mod->comp_cnt; i++) {
		p = stpncpy(p, mod->comp_list[i].label, be - p);
		p = stpncpy(p, "=", be - p);

		switch (mod->comp_list[i].comp_dtype) {
		case DTYPE_U16:
			utoa((unsigned)mod->comp_list[i].comp.comp_u16, print_comp_buff, 10);
			break;
		case DTYPE_I16:
			itoa((int)mod->comp_list[i].comp.comp_i16, print_comp_buff, 10);
			break;
		case DTYPE_U32:
			utoa((unsigned)mod->comp_list[i].comp.comp_u32, print_comp_buff, 10);
			break;
		case DTYPE_I32:
			itoa((int)mod->comp_list[i].comp.comp_i32, print_comp_buff, 10);
			break;
		default:
			break;
		}

		p = stpncpy(p, print_comp_buff, be - p);
		p = stpncpy(p, " ", be - p);
	}

	print_mod_buff[print_mod_buff_size - 1] = 0;
	dbgs_println(&dbgs_srcio, print_mod_buff);
}
