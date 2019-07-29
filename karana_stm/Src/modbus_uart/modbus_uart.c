/* Src/modbus_uart/modbus_uart.c
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

#include "modbus_uart.h"
#include "cmsis_os.h"
#include "modbus_uart_io.h"

static uint16_t CRC16 (const char *nData, uint16_t wLength);
static int pack_send(unsigned int size);
static int receive(uint8_t fnct, int size, const modbusInstanceStruct *minstance);
//#define RX_CRC_LSB_ONLY

#define ADDR_BCNT 1
#define FCT_BCNT 1
#define CRC_BCNT_TX 2

#ifdef RX_CRC_LSB_ONLY
	#define CRC_BCNT_RX 1
#else
	#define CRC_BCNT_RX CRC_BCNT_TX
#endif

#define START_ADDR_BCNT 2
#define REG_CNT_BCNT 2
#define BYTE_CNT_BCNT 1

#define _swap_bytes(__val) ((uint16_t)(((__val) >> 8) | ((__val) << 8)))


#define RTX_BUFFSIZE 64
static char rtx_buff[RTX_BUFFSIZE];

static uint8_t last_slave_addr = 0;
//static uint64_t last_frame_end = 0; // ms absolute

static char is_init = 0;
void modbus_init(void)
{
	if (is_init) return;

	UART1_user_init();
	is_init = 1;

	_modbus_lock_init();
}

/* Calculate modbus crc16
 * Credits to: https://www.modbustools.com/modbus_crc16.html
 */
static uint16_t CRC16 (const char *nData, uint16_t wLength)
{
	static const uint16_t wCRCTable[] = {
	0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
	0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
	0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
	0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
	0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
	0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
	0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
	0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
	0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
	0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
	0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
	0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
	0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
	0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
	0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
	0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
	0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
	0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
	0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
	0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
	0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
	0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
	0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
	0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
	0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
	0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
	0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
	0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
	0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
	0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
	0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
	0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };

	unsigned char nTemp;
	uint16_t wCRCWord = 0xFFFF;

	while (wLength--) {
		nTemp = *nData++ ^ wCRCWord;
		wCRCWord >>= 8;
		wCRCWord ^= wCRCTable[nTemp];
	}

	return wCRCWord;

}

/*
 * Sends the data, then its CRC.
 */
static int pack_send(unsigned int size)
{
	 // clear possible garbage on the bus
	while(_modbus_uart_read(rtx_buff, 1, 0) != -1);

	*((uint16_t*)(rtx_buff + size)) = CRC16(rtx_buff, size);
	/*
	 * Wait for the minimum time after a slave address change
	 */
	uint8_t slave_addr = rtx_buff[0];
	if (slave_addr != last_slave_addr) {
		last_slave_addr = slave_addr;
		_modbus_wait_chaddr_timer();
	}

	// send
	if (_modbus_uart_write(rtx_buff, size + CRC_BCNT_TX) == -1)
		return -1;

	return 0;

}

/*
 * Receives data of length 'size' into the rx_buff, and checks for address consistency and data integrity
 */
static int receive(uint8_t fnct, int size, const modbusInstanceStruct *minstance)
{
	unsigned int raw_size = ADDR_BCNT + FCT_BCNT + size + CRC_BCNT_RX;
	int retval = _modbus_uart_read(rtx_buff, raw_size, _delay_read(raw_size));

	_modbus_start_chaddr_timer(SLAVE_CHANGES_TIMEBUF);
	if (retval == -1)
		return -1;

	if (rtx_buff[0] != minstance->slave_addr)
		return -1;

	if (rtx_buff[1] != fnct)
		return -1;



#ifdef RX_CRC_LSB_ONLY
	uint8_t crc_msg = rtx_buff[raw_size - CRC_BCNT_RX];
	if (crc_msg  != (CRC16(rtx_buff, raw_size - CRC_BCNT_RX) & 0x00FF)) {
		return -1;
	}
#else
	uint16_t crc_msg = *((uint16_t*)(rtx_buff + raw_size - CRC_BCNT_RX));
	if (crc_msg != CRC16(rtx_buff, raw_size - CRC_BCNT_RX))
		return -1;
#endif

	return 0;
}

/*
 * Adds the address and the CRC to the data and sends it. It doesn't listen for replies.
 */
int modbus_sendRaw(const unsigned char* data, unsigned int size, const modbusInstanceStruct *minstance)
{
	//unsigned char buffer[size + 1];
	_modbus_lock_acq();
	rtx_buff[0] = minstance->slave_addr;
	int res;

	for (int i = 0; i < size; i++) {
		rtx_buff[i+1] = data[i];
	}

	res = pack_send(size + 1);

	_modbus_lock_rel();
	return res;
}

// Stores the content of multiple holding registers in "content" buffer (little endian).
int modbus_readMHld(uint16_t start_reg_addr, uint16_t reg_cnt, const modbusInstanceStruct *minstance, uint16_t *content)
{
	uint16_t *tmp_p;
	int res = 0;

	_modbus_lock_acq();

	rtx_buff[0] = minstance->slave_addr;
	rtx_buff[1] = MODBUS_RMHLD;

	tmp_p = (uint16_t*)(&rtx_buff[2]);
	*tmp_p = _swap_bytes(start_reg_addr);

	tmp_p = (uint16_t*)(&rtx_buff[4]);
	*tmp_p = _swap_bytes(reg_cnt);


	res = pack_send(ADDR_BCNT + FCT_BCNT + START_ADDR_BCNT + REG_CNT_BCNT);

	if (res == 0) {
		res = receive(MODBUS_RMHLD, BYTE_CNT_BCNT +  reg_cnt * 2, minstance);
	}

	if (res != 0) {
		_modbus_lock_rel();
		return -1;
	}

	unsigned int content_offset = 0 + ADDR_BCNT + FCT_BCNT + BYTE_CNT_BCNT;


	uint16_t tmp;
	for (unsigned int i = 0; i < reg_cnt; i++) {
		tmp = ((uint16_t*)(rtx_buff + content_offset))[i];
		content[i] = _swap_bytes(tmp);
	}

	_modbus_lock_rel();
	return 0;

}

// Stores the content of multiple input registers in "content" buffer in (little endian).
int modbus_readInput(uint16_t start_reg_addr, uint16_t reg_cnt, const modbusInstanceStruct *minstance, uint16_t *content)
{
	uint16_t *tmp_p;
	int res = 0;

	_modbus_lock_acq();
	rtx_buff[0] = minstance->slave_addr;
	rtx_buff[1] = MODBUS_RI;

	tmp_p = (uint16_t*)(&rtx_buff[2]);
	*tmp_p = _swap_bytes(start_reg_addr);

	tmp_p = (uint16_t*)(&rtx_buff[4]);
	*tmp_p = _swap_bytes(reg_cnt);

	res = pack_send(ADDR_BCNT + FCT_BCNT + START_ADDR_BCNT + REG_CNT_BCNT);

	if (res == 0)
		res = receive(MODBUS_RI, BYTE_CNT_BCNT + reg_cnt * 2, minstance);

	if (res != 0) {
		_modbus_lock_rel();
		return -1;

	}

	unsigned int content_offset = ADDR_BCNT + FCT_BCNT + BYTE_CNT_BCNT;

	uint16_t tmp;
	for (unsigned int i = 0; i < reg_cnt; i++) {
		tmp = ((uint16_t*)(rtx_buff + content_offset))[i];
		content[i] = _swap_bytes(tmp);
	}

	_modbus_lock_rel();
	return 0;
}

int modbus_writeMHld(uint16_t start_addr, uint16_t reg_cnt, const uint16_t* data, const modbusInstanceStruct *minstance)
{
	uint16_t byte_cnt = reg_cnt * 2;
	uint16_t *tmp_p;
	int res = 0;

	unsigned int raw_size = ADDR_BCNT + FCT_BCNT + START_ADDR_BCNT + REG_CNT_BCNT + BYTE_CNT_BCNT + byte_cnt;

	_modbus_lock_acq();
	rtx_buff[0] = minstance->slave_addr;
	rtx_buff[1] = MODBUS_WMHLD;

	tmp_p = (uint16_t*)(&rtx_buff[2]);
	*tmp_p = _swap_bytes(start_addr);

	tmp_p = (uint16_t*)(&rtx_buff[4]);
	*tmp_p = _swap_bytes(reg_cnt);

	rtx_buff[6] = byte_cnt;

	unsigned int data_offset = raw_size - byte_cnt;
	tmp_p = (uint16_t*)(rtx_buff + data_offset);

	for (unsigned int i = 0; i < reg_cnt; i++) {
		tmp_p[i] = _swap_bytes(data[i]);
	}

	res = pack_send(raw_size);

	if (res == 0)
		res = receive(MODBUS_WMHLD, START_ADDR_BCNT + REG_CNT_BCNT, minstance);

	_modbus_lock_rel();
	return res;

}

int modbus_readMHld_list(const uint16_t *reg_list, uint16_t *data, uint8_t cnt, const modbusInstanceStruct *minstance)
{
	for (unsigned i = 0; i < cnt; i++) {
		if (modbus_readMHld(reg_list[i], 1, minstance, &data[i]) == -1) {
			return -1;
		}
	}
	return 0;
}

int modbus_readInput_list(const uint16_t *reg_list, uint16_t *data, uint8_t cnt, const modbusInstanceStruct *minstance)
{
	for (unsigned i = 0; i < cnt; i++) {
		if (modbus_readInput(reg_list[i], 1, minstance, &data[i]) == -1) {
			return -1;
		}
	}
	return 0;
}
