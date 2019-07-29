/* Src/source_io/SourceIO_instances.c
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

#include <SourceIO_user.h>

static volatile uint8_t is_ready;

SRCIO_R SrcIO_init(void)
{
	if (SrcIO_user_init() == SRCIO_OK) {
		is_ready = 1;
		return SRCIO_OK;
	}

	return SRCIO_NR;
}

SRCIO_R SrcIO_isReady(void)
{
	if (is_ready) return SRCIO_OK;
	return SRCIO_NR;
}

unsigned SrcIO_mod_cnt(void){
	return sizeof(srcio_mods) / sizeof(SourceIOMod*);
}
