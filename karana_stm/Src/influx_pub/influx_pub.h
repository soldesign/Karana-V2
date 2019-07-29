/* Src/influx_pub/influx_pub.h
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

#ifndef INFLUX_PUB_INFLUX_PUB_H_
#define INFLUX_PUB_INFLUX_PUB_H_
#include "SourceIOMod.h"
#include "Storage.h"

typedef enum {
	IFX_OK,
	IFX_ERR,
	IFX_MEM,
	IFX_NOMATCH

} IFX_RES;

IFX_RES ifxp_init(SourceIOMod * const srciomod_list[], uint8_t srciomod_cnt);
IFX_RES ifxp_pub_init(
		const char *serv,
		uint16_t port,
		const char *ifxdb,
		const char *user,
		const char *pwd);
IFX_RES ifxp_pub_http_feed(stor_data_t *data);
IFX_RES ifxp_pub_http_end(void);
#endif /* INFLUX_PUB_INFLUX_PUB_H_ */
