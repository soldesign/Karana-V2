/* Src/srcio_module_windspeed/SourceIOMod_windspeed.h
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

#ifndef SRCIO_MODULE_WINDSPEED_SOURCEIOMOD_WINDSPEED_H_
#define SRCIO_MODULE_WINDSPEED_SOURCEIOMOD_WINDSPEED_H_

#include "SourceIOMod.h"

typedef struct {
	SourceIOMod _super;
} SourceIOMod_windspeed;

SRCIO_R SourceIOMod_windspeed_construct(SourceIOMod_windspeed *_this);

#endif /* SRCIO_MODULE_WINDSPEED_SOURCEIOMOD_WINDSPEED_H_ */
