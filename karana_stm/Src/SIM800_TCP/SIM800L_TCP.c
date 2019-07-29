/* Src/SIM800_TCP/SIM800L_TCP.c
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

#include "SIM800L_TCP.h"
#include <string.h>
#include <stdlib.h>
#include "SIM800L_TCP_io.h"
#include "time_loc.h"
#include "../karana_conf.h"

//static int send_command_data(const char *command, const char *response, const char data, unsigned size);
static int send_command(const char *command, const char *response, unsigned int attempts, unsigned int wait_time);
static int get_response(uint32_t timeout);

#define RX_BUF_SIZE (128)
#define TX_BUF_SIZE (256)

#define TX_DELAY_FRACTION 1

static char rx_buf[RX_BUF_SIZE];
static char tx_buf[TX_BUF_SIZE];

static char dbg_buff[256];

static tout_t module_timer;
static unsigned byte_transt_us;

void SIM800L_TCP_init(SIM800L_TCP_initStruct *init_struct)
{
	byte_transt_us = (1000000UL * 10)  / init_struct->baud;
	byte_transt_us += byte_transt_us / TX_DELAY_FRACTION;
	time_nobl_wait_start(1, &module_timer);
}

void SIM800L_TCP_simPowerState(int simState)
{
	SIM800L_io.set_sim_state(simState);
}

void SIM800L_TCP_simReset(void)
{
	SIM800L_io.set_sim_state(SIM800L_OFF);
    SIM800L_io.delay_ms(500);
    SIM800L_io.set_sim_state(SIM800L_ON);
    time_nobl_wait_start(3500, &module_timer);
}

int SIM800L_TCP_initGPRS(const char* apn, const char* user, const char* pwd)
{
	// switch off echoing
	while(!time_nobl_wait_ovt(&module_timer));
	if(send_command("E0", "OK", 3, 5000)) return -1;

	// ??
	if(send_command("+COPS=0", "OK", 5, 1000)) return -1;

	// set connection type
	if(send_command("+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK", 4, 5000)) return -1;

	// set acces point
	strcpy(tx_buf, "+SAPBR=3,1,\"APN\",\"");
	strcat(tx_buf, apn);
	strcat(tx_buf, "\"");
	if(send_command(tx_buf, "OK", 4, 5000)) return -1;

	// set user
	strcpy(tx_buf, "+SAPBR=3,1,\"USER\",\"");
	strcat(tx_buf, user);
	strcat(tx_buf, "\"");
	if(send_command(tx_buf, "OK", 5, 5000)) return -1;

	// set password
	strcpy(tx_buf, "+SAPBR=3,1,\"PWD\",\"");
	strcat(tx_buf, pwd);
	strcat(tx_buf, "\"");
	if(send_command(tx_buf, "OK", 2, 5000)) return -1;

	time_nobl_wait_start(5000, &module_timer);
	return 0;
}


int SIM800L_TCP_initIP(void)
{
	while(!time_nobl_wait_ovt(&module_timer));
	time_nobl_wait_start(0, &module_timer);

	if (send_command("+SAPBR=1,1", "OK", 5, KC_SIM_GET_IP_TIMEOUT_MS / 5)) {
		return -1;
	}

	if (send_command("+SAPBR=2,1", "OK", 4, 1000)) return -1;
	return 0;
}

int SIM800L_TCP_connect(const char *ip, uint16_t port)
{


	char utoab[10];
	utoa(port, utoab, 10);

	if(send_command("+CIPMUX=0", "OK", 2, 500)) return -1;
	if(send_command("+CIPRXGET=1", "OK", 2, 500)) return -1;

	strcpy(tx_buf, "+CIPSTART=\"TCP\",\"");
	strcat(tx_buf, ip);
	strcat(tx_buf, "\",\"");
	strcat(tx_buf, utoab);
	strcat(tx_buf, "\"");

	if (send_command(tx_buf, "CONNECT OK", 2, 10000)) return -1;
	return 0;
}

int SIM800L_TCP_write(const char* data, unsigned size)
{
	char utoab[20];

	utoa(size, utoab, 10);

	strcpy(tx_buf, "+CIPSEND=\"");
	strcpy(tx_buf, utoab);
	strcpy(tx_buf, "\"");

	if(send_command(tx_buf, ">", 1, 100)) return -1;

	SIM800L_io.write(data, size);

	if (get_response(5000)) return -1;
	if (strstr(rx_buf, "SEND_OK") != rx_buf) return -1;

	return 0;

}


// Lat / Long not working (proably locale settings)
int SIM800L_TCP_get_loc_time(SIM800L_tlStruct *tl_struct)
{
	uint8_t att = 3;
	while (att--) {
		if(send_command("+CIPGSMLOC=1,1", "+CIPGSMLOC", 3, KC_SIM_GET_TIME_TIMEOUT_MS / 3)) return -1;

		char *tok;
		tok = strtok(rx_buf, ":");
		tok = strtok(NULL, ", ");

		// if bad answer or SIM response error code not 0
		if (!tok || *tok != '0')
			continue;

		tok = strtok(NULL, ",");
	//	tl_struct->longitude = atof(tok);
		tok = strtok(NULL, ",");
	//	tl_struct->latitude = atof(tok);

		tok = strtok(NULL, "/");
		tl_struct->year = atoi(tok) - 2000;
		tok = strtok(NULL, "/");
		tl_struct->month = atoi(tok);
		tok = strtok(NULL, ",");
		tl_struct->day = atoi(tok);

		tok = strtok(NULL, ":");
		tl_struct->hour = atoi(tok);
		tok = strtok(NULL, ":");
		tl_struct->min = atoi(tok);
		tok = strtok(NULL, "");
		tl_struct->sec = atoi(tok);

		return 0;
	}
	return -1;
}

static int send_command(const char *command, const char *response, unsigned int attempts, unsigned int wait_time)
{
	tout_t command_timer;
	unsigned int response_attempts;

	if (command) {
		strcpy(dbg_buff, "TX: AT");
		strcat(dbg_buff, command);
	} else {
		dbg_buff[0] = 0;
	}

	SIM800L_io.debug_println(dbg_buff);
	// flush buffer
	SIM800L_io.clr();

	unsigned attempts_cnt = 0;
	while (attempts_cnt++ < attempts) {
		if (command) {
			SIM800L_io.write("AT", 2);
			SIM800L_io.write(command, strlen(command));
			SIM800L_io.write("\r\n", 2);
		}

		response_attempts = 10;
		time_nobl_wait_start(wait_time, &command_timer);

		while (response_attempts-- && (!time_nobl_wait_ovt(&command_timer))) {
			if (get_response(time_nobl_wait_left(&command_timer)) != 0) {
				continue;
			}

			SIM800L_io.debug_println(rx_buf);
			// if the response does contain the matching prefix, then success
			if (strstr(rx_buf, response) == rx_buf) {
				SIM800L_io.debug_println("cmd OK");
				return 0;
			}
		}
	}
	SIM800L_io.debug_println("cmd NOK");
	return -1;
}

static int get_response(uint32_t timeout)
{
	return SIM800L_io.readln(rx_buf, RX_BUF_SIZE, timeout);
}

int SIM800L_TCP_http_init(void)
{
	if (send_command("+HTTPINIT", "OK", 2, 500)) return -1;
	if (send_command("+HTTPSSL=1", "OK", 2, 500)) return -1;
    return 0;
}

int SIM800L_TCP_http_init_post(const char* url, const char* mime)
{

	if (send_command("+HTTPPARA=\"CID\",\"1\"", "OK", 4, 500)) return -1;

	strcpy(tx_buf, "+HTTPPARA=\"URL\",\"");
	strcat(tx_buf, url);
	strcat(tx_buf, "\"");

	if (send_command(tx_buf, "OK", 4, 500)) return -1;

	strcpy(tx_buf, "+HTTPPARA=\"CONTENT\",\"");
	strcat(tx_buf, mime);
	strcat(tx_buf, "\"");

	if (send_command(tx_buf, "OK", 4, 500)) return -1;
	return 0;
}

int SIM800L_TCP_http_init_post_download(uint32_t data_size) {
	uint32_t data_time_ms = (data_size * byte_transt_us) / 1000;
	if (data_time_ms < 1000) data_time_ms = 1000;

	strcpy(tx_buf, "+HTTPDATA=");
	utoa(data_size, tx_buf + strlen(tx_buf), 10);
	strcat(tx_buf, ",");
	utoa(data_time_ms, tx_buf + strlen(tx_buf), 10);

	return send_command(tx_buf, "DOWNLOAD", 1, 10000);

}

int SIM800L_TCP_http_post_download(const char *data, unsigned size)
{
	return SIM800L_io.write(data, size);
}


int SIM800L_TCP_http_post_end(unsigned max_bytes, unsigned fed_bytes)
{

	static tout_t post_timer;
	static uint32_t ms_left;
	int retval = 0;
	uint32_t wait_ms = ((max_bytes) * byte_transt_us) / 1000;
	if (send_command(NULL, "OK", 100, wait_ms)) return -1;

	time_nobl_wait_start(KC_SIM_HTTP_POST_TIMEOUT_MS, &post_timer);

	retry_post_end:
	ms_left = time_nobl_wait_left(&post_timer);

	if (!ms_left) retval = -1;
	else if (send_command("+HTTPACTION=1", "+HTTPACTION", 4, ms_left / 4) != 0) retval = -1;

	if (retval == 0) {
		if (strstr(rx_buf, "+HTTPACTION: 1,204") == rx_buf){
			return 0;
		} else if (strstr(rx_buf, "+HTTPACTION: 1,604") == rx_buf) {
			osDelay(1000);
			goto retry_post_end;
		} else {
			retval = -1;

		}
	}

	send_command("+HTTPREAD=0,100", "OK", 1, 10000);

	return retval;
}
