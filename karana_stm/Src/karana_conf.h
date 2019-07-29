/* Src/karana_conf.h
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

#ifndef KARANA_CONF_H_
#define KARANA_CONF_H_

/*
 * SIM card APN, as string constant.
 */
#define KC_SIM_APN						""
/*
 * Username for registering into the network, as string constant.
 * leave empty if you don't know what this is
 */
#define KC_SIM_USR						""
/*
 * Password for registering into the network, as string constant.
 * leave empty if you don't know what this is
 */
#define KC_SIM_PWD						""
/*
 * Maximum timeout, in ms, for a HTTP post to complete. Poor network connections may
 * require this value to be greater.
 */
#define KC_SIM_HTTP_POST_TIMEOUT_MS		120000UL
/*
 * Maximum timeout, in ms, for registering into network. Poor network connections,
 * especially if in roaming, may require this value to be greater.
 */
#define KC_SIM_GET_IP_TIMEOUT_MS		75000L
/*
 * Maximum timeout, in ms, for a time update request. Poor network connections may
 * require this value to be greater.
 */
#define KC_SIM_GET_TIME_TIMEOUT_MS		30000UL

/*
 * A string to identify the Karana, as string constant. It corresponds to a
 * tag key/value pair in InfluxDB terms. If you have more Karanas sending to
 * the same database, this can be used to filter the measurements by Karana ID
 * in an efficient manner.
 * Keep this ID as short as possible, since it is present in every write query
 * that is sent to the database, and it may bloat the data traffic unnecessarily.
 */
#define KC_ID							"k1"
/*
 * URL of the InfluxDB database, as string constant
 */
#define KC_IFX_URL						""
/*
 * Port of the InfluxDB, as integer
 */
#define KC_IFX_PORT						443
/*
 * InfluxDB database to write into
 */
#define KC_IFX_DB						""
/*
 * Username for write access to the database
 */
#define KC_IFX_USR						""
/*
 * Password for write access to the database
 */
#define KC_IFX_PWD						""


#endif /* KARANA_CONF_H_ */
