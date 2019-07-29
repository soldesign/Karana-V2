/* Src/logger/logger.c
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

#include "logger.h"
#include "modbus_uart.h"
#include "usart.h"
#include "time_loc.h"
#include "cmsis_os.h"
#include "Storage.h"
#include "debug_serial.h"
#include "SourceIO_instances.h"
#include "data_pack.h"
#include <string.h>
#include <stdlib.h>

uint8_t logger_ready = 0;

#define RAW_DATA_SIZE 32
char raw_data[RAW_DATA_SIZE];
stor_data_t data = {
		.raw_data = raw_data,
		.buff_size = RAW_DATA_SIZE
};

char dbg_buff[32];

void startLogger_thread(void const* arg)
{
	static dbgs_inst_t dbgs_logger = {.name = "LOG"};
	static uint32_t mod_log_loop_left, mod_log_left;
	static STOR_R stor_r;
	static SRCIO_R srciom_r;

	if (stor_init() != STOR_OK) {
		while(1) {
			dbgs_println(&dbgs_logger, "init failed!");
			osDelay(5000);
		}
	}

	while (1) {

		if (SrcIO_isReady() == SRCIO_OK) {
			for (unsigned i = 0; i < SrcIO_mod_cnt(); i++) {
				mod_log_left = time_nobl_wait_left(&srcio_mods[i]->log_period);

				if (!mod_log_left) {
					srciom_r = SourceIOMod_getRaw(srcio_mods[i], data.raw_data, data.buff_size);
					if (srciom_r != SRCIO_OK) {
						dbgs_println(&dbgs_logger, "srcio err");
						itoa(srciom_r, dbg_buff, 10);
						dbgs_println(&dbgs_logger, dbg_buff);

					} else {
						data.data_size = srcio_mods[i]->data_raw_size;
						data.vector_id = srcio_mods[i]->ID;

						stor_r = stor_put(STOR_TGT_DATA, data);
						if (stor_r != STOR_OK) {
							dbgs_println(&dbgs_logger, stor_e_str);
						} else {
							dbgs_println(&dbgs_logger, "log ok");
						}
					}
				}
			}

			mod_log_loop_left = 0xFFffFFff;
			for (unsigned i = 0; i < SrcIO_mod_cnt(); i++) {
				mod_log_left = time_nobl_wait_left(&srcio_mods[i]->log_period);
				if (mod_log_left < mod_log_loop_left)
					mod_log_loop_left = mod_log_left;
			}
			osDelay(mod_log_loop_left);

		} else {
			dbgs_println(&dbgs_logger, "SrcIO not ready");
			osDelay(500);
		}
	}
}
