/* Src/net/net.c
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

//#include <utils.h.old>
#include "net.h"
#include "SIM800L_TCP.h"
#include "time_loc.h"
#include "gpio.h"
#include "usart.h"
#include "Storage.h"
#include "debug_serial.h"
#include "stdlib.h"
#include "SourceIO_instances.h"
#include "influx_pub.h"
#include "../karana_conf.h"
#include "string.h"

#define TIME_UPDATE_PER (1000UL * 60 * 60)
#define DATA_SEND_PER (1000UL * 5)

#define CARD_READB_SZ 128

static void init_sim(void);
static int update_time(void);
static int send_data(void);

static tout_t misc_timer;
static tout_t time_update_timer; 	// real time syncing with the cellular network
static tout_t data_send_timer;

static dbgs_inst_t net_dbg = { .name = "NET" };
static char card_readb[CARD_READB_SZ];
static stor_data_t raw_data = {
		.raw_data = card_readb,
		.buff_size = CARD_READB_SZ
};

#define APN_BUFLEN 32
static char apn[APN_BUFLEN];
#define APN_CFGLAB "SIM_APN"

	//static char dbg_buff[64];
void startNetTask(void const* arg) {
	UART6_user_init();

	time_nobl_wait_start(2000, &misc_timer);
	STOR_R stor_r = STOR_NA;;
	while (stor_r == STOR_NA && time_nobl_wait_left(&misc_timer)) {
		stor_r = stor_get_config(APN_CFGLAB, apn, APN_BUFLEN);
	}

	if (stor_r != STOR_OK) {
		strlcpy(apn, KC_SIM_APN, APN_BUFLEN);
	}

	init_sim();

	while (update_time() != 0);
	time_nobl_wait_start(TIME_UPDATE_PER, &time_update_timer);
	time_nobl_wait_start(DATA_SEND_PER, &data_send_timer);

	while (SrcIO_isReady() == SRCIO_NR);

	if (ifxp_init(srcio_mods, SrcIO_mod_cnt()) != IFX_OK) {
		while (1) {
			dbgs_println(&net_dbg, "Cannot init ifxp!");
			osDelay(5000);
		}
	}

	while(1) {
		if (time_nobl_wait_ovt(&time_update_timer))
			while(update_time() != 0);

		if (time_nobl_wait_ovt(&data_send_timer)) {
			send_data();

		} else {
			dbgs_println(&net_dbg, "idle");
		}

		osDelay(1000);
	}
}

static void init_sim(void)
{
	init_SIM_rstpin();

	SIM800L_TCP_initStruct sim_init;
	sim_init.baud = 115200;
	SIM800L_TCP_init(&sim_init);
}

static int update_time(void)
{

	SIM800L_TCP_simReset();
	if (SIM800L_TCP_initGPRS(apn, KC_SIM_USR, KC_SIM_PWD) != 0)
		return -1;
	if (SIM800L_TCP_initIP() != 0)
		return -1;


	SIM800L_tlStruct tl_s;

	if (SIM800L_TCP_get_loc_time(&tl_s)) {
		//uart_dbg_send_string("timeloc fail\r\n");
		return -1;
	} else {
		SIM800L_TCP_simPowerState(SIM800L_OFF);

		time_tlStruct tloc;
		tloc.year = tl_s.year;
		tloc.month = tl_s.month;
		tloc.day = tl_s.day;
		tloc.hour = tl_s.hour;
		tloc.min = tl_s.min;
		tloc.sec = tl_s.sec;

		time_init(&tloc);

		return 0;
	}
}


static int send_data(void)
{
	stor_get_deinit(0);
	STOR_R stor_r = stor_get_init(STOR_TGT_DATA);
	if (stor_r == STOR_OK) {
		// init the SIM800L and influx publisher
		SIM800L_TCP_simReset();
		if (SIM800L_TCP_initGPRS(apn, KC_SIM_USR, KC_SIM_PWD) != 0) {
			stor_get_deinit(0);
			SIM800L_TCP_simPowerState(SIM800L_OFF);
			return -1;
		}

		if (SIM800L_TCP_initIP() != 0) {
			stor_get_deinit(0);
			SIM800L_TCP_simPowerState(SIM800L_OFF);
			return -1;
		}

		if (ifxp_pub_init(
				KC_IFX_URL,
				KC_IFX_PORT,
				KC_IFX_DB,
				KC_IFX_USR,
				KC_IFX_PWD) != IFX_OK) {
			stor_get_deinit(0);
			SIM800L_TCP_simPowerState(SIM800L_OFF);
			return -1;
		}

	}
	IFX_RES ifxr;

	while (stor_r == STOR_OK) {
		while ((stor_r = stor_get(&raw_data)) == STOR_OK) {
			if ((ifxr = ifxp_pub_http_feed(&raw_data)) != IFX_OK) {
				if (ifxr == IFX_NOMATCH) {
					// probably file corrupt
					dbgs_println(&net_dbg, "ifxp: no match!");
					ifxp_pub_http_end();
					stor_get_deinit(1);
					SIM800L_TCP_simPowerState(SIM800L_OFF);
					return -1;
				}
				dbgs_println(&net_dbg, "ifxp: cannot feed");
				ifxp_pub_http_end();
				stor_get_deinit(0);
				SIM800L_TCP_simPowerState(SIM800L_OFF);
				return -1;
			}
		}

		if (ifxp_pub_http_end() != IFX_OK) {
			stor_get_deinit(0);
			return -1;
		}

		if (stor_r == STOR_NA || stor_r == STOR_OK) {
			dbgs_println(&net_dbg, "Stor file completed");

		} else {
			dbgs_println(&net_dbg, "Stor err");
		}
		stor_get_deinit(1);
		stor_r = stor_get_init(STOR_TGT_DATA);
	}

	SIM800L_TCP_simPowerState(SIM800L_OFF);
	stor_get_deinit(0);
	if (stor_r != STOR_NA) {

		dbgs_println(&net_dbg, "Error storage");
	} else {
		dbgs_println(&net_dbg, "Nothning to send");
	}
	return 0;
}
