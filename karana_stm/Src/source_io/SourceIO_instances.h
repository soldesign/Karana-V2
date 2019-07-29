/* Src/source_io/SourceIO_instances.h
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

#ifndef SOURCE_IO_SOURCEIO_INSTANCES_H_
#define SOURCE_IO_SOURCEIO_INSTANCES_H_

#include "SourceIO_defs.h"
#include "SourceIOMod.h"

extern SourceIOMod * const srcio_mods[];

extern SRCIO_R SrcIO_init(void);
extern SRCIO_R SrcIO_isReady(void);
unsigned SrcIO_mod_cnt(void);

#endif /* SOURCE_IO_SOURCEIO_INSTANCES_H_ */
