/* Src/SIM800_TCP/SIM800L_TCP.h
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

#ifndef SIM800L_TCP_H
#define SIM800L_TCP_H

#include "stdint.h"

typedef enum  {SIM800L_OFF, SIM800L_ON} SIM800L_state;

#define RX_DATA_OVERTIME 10

typedef struct SIM800L_tlStruct {
	float longitude;
	float latitude;

	uint8_t year; // year - 2000
	uint8_t month;
	uint8_t day;

	uint8_t hour;
	uint8_t min;
	uint8_t sec;

} SIM800L_tlStruct;

typedef struct SIM800L_TCP_initStruct {
	uint32_t baud;
} SIM800L_TCP_initStruct;

void SIM800L_TCP_init(SIM800L_TCP_initStruct *init_struct);
void SIM800L_TCP_simPowerState(int simState);
void SIM800L_TCP_simReset(void);
int	SIM800L_TCP_initGPRS(const char* apn, const char* user, const char* pwd);
int SIM800L_TCP_initIP(void);
int SIM800L_TCP_connect(const char *ip, uint16_t port);
int SIM800L_TCP_disconnect(void);
int SIM800L_TCP_write(const char* buf, unsigned int len);
int SIM800L_TCP_read(const char* buf, unsigned int len);
int SIM800L_TCP_get_loc_time(SIM800L_tlStruct *tl_struct);
int SIM800L_TCP_http_init(void);
int SIM800L_TCP_http_init_post(const char* url, const char* mime);
int SIM800L_TCP_http_init_post_download(uint32_t data_size);
int SIM800L_TCP_http_post_download(const char *data, unsigned size);
int SIM800L_TCP_http_post_end(unsigned max_bytes, unsigned fed_bytes);

#endif /* SIM800L_TCP_H */
