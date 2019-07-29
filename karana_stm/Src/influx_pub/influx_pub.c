/* Src/influx_pub/influx_pub.c
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

#include "influx_pub.h"
#include "cmsis_os.h"
#include "SIM800L_TCP.h"
#include <stdio.h>
#include "Storage.h"
#include <string.h>
#include "../karana_conf.h"

#define MAX_FEED (32 * 1024UL)
#define MAX_FEED_EXTRA 1024
#define PUB_BUFF_SIZE 1024

static SourceIOMod *const *mod_list;
static uint8_t mod_cnt;
static SourceIOMod pubmod;
static uint8_t pubcomp_cnt;
static char pub_buff[PUB_BUFF_SIZE];

static struct {
	const char *serv;
	uint16_t port;
	const char *ifxdb;
	const char *user;
	const char *pwd;
} ifxdb_vars;

static size_t pub_http_feed_record(const SourceIOMod *src_mod, uint64_t unix_ms);
static IFX_RES pub_http_start_post(void);

IFX_RES ifxp_init(SourceIOMod * const srciomod_list[], uint8_t srciomod_cnt)
{
	mod_list = srciomod_list;
	mod_cnt = srciomod_cnt;

	pubcomp_cnt = 0;
	for (unsigned i = 0; i < mod_cnt; i++) {
		if (mod_list[i]->comp_cnt > pubcomp_cnt)
			pubcomp_cnt = mod_list[i]->comp_cnt;
	}

	pubmod.comp_list = pvPortMalloc(pubcomp_cnt * sizeof(data_comp));
	if (pubmod.comp_list == NULL)
		return IFX_MEM;

	return IFX_OK;
}

IFX_RES ifxp_pub_init(
		const char *serv,
		uint16_t port,
		const char *ifxdb,
		const char *user,
		const char *pwd)
{
	ifxdb_vars.serv = serv;
	ifxdb_vars.port = port;
	ifxdb_vars.ifxdb = ifxdb;
	ifxdb_vars.user = user;
	ifxdb_vars.pwd = pwd;

	if (SIM800L_TCP_http_init() != 0) return IFX_ERR;
	return IFX_OK;
}

static IFX_RES pub_http_start_post(void) {
	int res;
	sprintf(pub_buff, "https://%s:%u/write?db=%s&u=%s&p=%s&precision=s",
			ifxdb_vars.serv,
			ifxdb_vars.port,
			ifxdb_vars.ifxdb,
			ifxdb_vars.user,
			ifxdb_vars.pwd);

	res = SIM800L_TCP_http_init_post(pub_buff, "application/x-www-form-urlencoded");
	if (res != 0) return IFX_ERR;

	res = SIM800L_TCP_http_init_post_download(MAX_FEED + MAX_FEED_EXTRA);
	if (res != 0) return IFX_ERR;

	return IFX_OK;
}

static size_t fed = 0;
static char is_feeding = 0;
IFX_RES ifxp_pub_http_feed(stor_data_t *data)
{
	const SourceIOMod *template = NULL;

	if (!is_feeding) {
		// init HTTP
		// init download
		fed = 0;
		if (pub_http_start_post() != IFX_OK) return IFX_ERR;
		is_feeding = 1;
	}

	for (unsigned i = 0; i < mod_cnt; i++) {
		if (mod_list[i]->ID == data->vector_id) {
			template = mod_list[i];
			break;
		}
	}

	if (!template) {
		ifxp_pub_http_end();
		return IFX_NOMATCH;
	}

	pubmod.ID = template->ID;
	pubmod.comp_cnt = template->comp_cnt;
	pubmod.data_raw_size = template->data_raw_size;
	pubmod.label = template->label;
	memcpy(pubmod.comp_list, template->comp_list, template->comp_cnt * sizeof(data_comp));

	if (SourceIOMod_rawToMod(&pubmod, data->raw_data, data->data_size) != SRCIO_OK) {
		ifxp_pub_http_end();
		return IFX_NOMATCH;
	}

	fed += pub_http_feed_record(&pubmod, data->unix_ms);

	if (fed > MAX_FEED) {
		if (ifxp_pub_http_end() != IFX_OK) return IFX_ERR;

	}

	return IFX_OK;

}

IFX_RES ifxp_pub_http_end(void)
{
	if (!is_feeding) return IFX_OK;
	is_feeding = 0;
	if (SIM800L_TCP_http_post_end(MAX_FEED_EXTRA + MAX_FEED, fed) != 0)
		return IFX_ERR;

	return IFX_OK;
}

static size_t pub_http_feed_record(const SourceIOMod *src_mod, uint64_t unix_ms)
{
	char * bp = pub_buff;
#define _pbleft()((long)(PUB_BUFF_SIZE - (bp - pub_buff)))

	bp += snprintf(bp, _pbleft(), "%s,KID=%s ", src_mod->label, KC_ID);
	if (_pbleft() <=0) return 0;
	for (unsigned i = 0; i < src_mod->comp_cnt; i++) {
		bp += snprintf(bp, _pbleft(), "%s=", src_mod->comp_list[i].label);
		if (_pbleft() <=0) return 0;

		switch(src_mod->comp_list[i].comp_dtype) {
		case DTYPE_U32:
			bp += snprintf(bp, _pbleft(), "%lu,", src_mod->comp_list[i].comp.comp_u32);
			break;

		case DTYPE_I32:
			bp += snprintf(bp, _pbleft(), "%ld,", src_mod->comp_list[i].comp.comp_i32);
			break;

		case DTYPE_U16:
			bp += snprintf(bp, _pbleft(), "%u,", (unsigned)src_mod->comp_list[i].comp.comp_u16);
			break;
			break;

		case DTYPE_I16:
			bp += snprintf(bp, _pbleft(), "%d,", (int)src_mod->comp_list[i].comp.comp_i16);
			break;

		default:
			return 0;
		}
		if (_pbleft() <=0) return 0;
	}
	*(bp - 1) = ' ';
	bp += snprintf(bp, _pbleft(), "%lu\n", (long unsigned)(unix_ms / 1000));
	if (_pbleft() <=0) return 0;
	SIM800L_TCP_http_post_download(pub_buff, bp - pub_buff);
	return bp - pub_buff;
}

/*
 * Tried to get rid of printf...
 */
//static size_t pub_http_feed_record(const SourceIOMod *src_mod, uint64_t unix_ms)
//{
//	char * bp = pub_buff;
//	char comp_buff[20];
//#define _bpleft()((int)((pub_buff + PUB_BUFF_SIZE) - bp))
//
//	bp += strlcpy(bp, src_mod->label, _bpleft());
//	if (_bpleft() <= 0) return 0;
//	bp += strlcpy(bp, ",KID=", _bpleft());
//	if (_bpleft() <= 0) return 0;
//	bp += strlcpy(bp, KC_ID, _bpleft());
//	if (_bpleft() <= 0) return 0;
//	bp += strlcpy(bp, " ", _bpleft());
//	if (_bpleft() <= 0) return 0;
//
//	for (unsigned i = 0; i < src_mod->comp_cnt; i++) {
//		//bp += snprintf(bp, _pb_left(), "%s=", src_mod->comp_list[i].label);
//		bp += strlcpy(bp, src_mod->comp_list[i].label, _bpleft());
//		if (_pbleft() <=0) return 0;
//		bp += strlcpy(bp, "=", _bpleft());
//		if (_pbleft() <=0) return 0;
//
//		switch(src_mod->comp_list[i].comp_dtype) {
//		case DTYPE_U16:
//			utoa((unsigned)src_mod->comp_list[i].comp.comp_u16, comp_buff, 10);
//			break;
//		case DTYPE_I16:
//			itoa((int)src_mod->comp_list[i].comp.comp_i16, comp_buff, 10);
//			break;
//		case DTYPE_U32:
//			utoa((unsigned)src_mod->comp_list[i].comp.comp_u32, comp_buff, 10);
//			break;
//		case DTYPE_I32:
//			itoa((int)src_mod->comp_list[i].comp.comp_i32, comp_buff, 10);
//			break;
//		default:
//			return 0;
//		}
//
//		bp += strlcpy(bp, comp_buff, _bpleft());
//		if (_pbleft() <=0) return 0;
//		bp += strlcpy(bp, ",", _bpleft());
//		if (_pbleft() <=0) return 0;
//
//	}
//	*(bp - 1) = ' ';
//	utoa((long unsigned)(unix_ms / 1000))
//
//}
