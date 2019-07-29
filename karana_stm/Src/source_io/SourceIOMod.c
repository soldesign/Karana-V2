/* Src/source_io/SourceIOMod.c
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

#include "SourceIOMod.h"
#include "string.h"

//extern SRCIO_R _SourceIOMod_update(SourceIOMod *_this);
//extern SRCIO_R _SourceIOMod_ctrl(SourceIOMod *_this, void *_srciom_ctrl_struct);
//
////const SourceIOMod_VMT sourceIOMod_VMT = {
////		._SourceIOMod_update = &_SourceIOMod_update,
////		._SourceIOMod_ctrl = &_SourceIOMod_ctrl
////};

///* ==================================================================
// * Do not call these!!!
// */
//SRCIO_R _SourceIOMod_update(SourceIOMod *_this);
//static SRCIO_R _SourceIOMod_ctrl(SourceIOMod *this);
///*
// * ==================================================================
// */

const SourceIOMod_VMT sourceIOMod_VMT = {
		._SourceIOMod_update = &_SourceIOMod_update,
		._SourceIOMod_ctrl = &_SourceIOMod_ctrl
};

SRCIO_R SourceIOMod_construct(SourceIOMod * _this, const SourceIOMod_init_t *init_stct)
{
	static const char nolabel[] = "";

	_this->vmt = &sourceIOMod_VMT;
	_this->ID = init_stct->ID;
	_this->comp_cnt = init_stct->comp_cnt;
	_this->comp_list = init_stct->comp_list;
	_this->label = init_stct->label;

	time_nobl_wait_start(init_stct->log_period_ms, &_this->log_period);
	time_nobl_wait_start(init_stct->update_period_ms, &_this->update_period);
	time_nobl_wait_start(init_stct->ctrl_period_ms, &_this->ctrl_period);

	uint16_t raw_size = 0;
	for (unsigned i = 0; i < _this->comp_cnt; i++) {
		switch (_this->comp_list[i].comp_dtype) {
		case DTYPE_U32:
		case DTYPE_I32:
			raw_size += 2;
			/* no break */
		case DTYPE_U16:
		case DTYPE_I16:
			raw_size += 2;
			break;

		default:
			return SRCIO_BADARGS;
		}
	}

	if (raw_size > 0xFF)
		return SRCIO_BADARGS;
	_this->data_raw_size = raw_size;

	_this->lock = osMutexCreate(&_this->os_mutex_def_lock);
	if (!_this->lock)
		return SRCIO_OS;

	for (unsigned i = 0; i < _this->comp_cnt; i++) {
		if (_this->comp_list[i].label == NULL) {
			_this->comp_list[i].label = nolabel;
		}

		if (_this->comp_list[i].comp_dtype >= DTYPE_ENUMSIZE) {
			_this->comp_list[i].comp_dtype = DTYPE_U16;
		}
	}

	_this->state = SRCIO_MOD_NA;


//	data_comp *components = pvPortMalloc(comp_cnt * sizeof(data_comp));
//	if (!components)
//		return SRCIO_MEM;

//	switch (dvect_init(&_this->dvect, components, comp_cnt)) {
//	case DVECT_BADARGS:
//		return SRCIO_BADARGS;
//
//	case DVECT_OS:
//		return SRCIO_OS;
//
//	case DVECT_OK:
//		return SRCIO_OK;
//
//	default:
//		return SRCIO_INTERN;
//	}

	return SRCIO_OK;
}

/*
 * Abstract
 */
SRCIO_R _SourceIOMod_update(SourceIOMod *_this)
{
	_this->state = SRCIO_MOD_NA;
	return SRCIO_MOD_NA;;
}

SRCIO_R _SourceIOMod_ctrl(SourceIOMod *this){
	return SRCIO_OK;
}

SRCIO_R SourceIOMod_getRaw(SourceIOMod *_this, void *dst, uint8_t dst_size)
{
	if (_this->state != SRCIO_OK)
		return _this->state;

	if (dst_size < _this->data_raw_size)
		return SRCIO_BADARGS;

	SRCIO_R retval = SRCIO_OK;

	union {
		void		*v;
		uint16_t	*u16;
		int16_t		*i16;
		uint32_t 	*u32;
		int32_t		*i32;
	} up;

	up.v = dst;
	if (osMutexWait(_this->lock, 100) != osOK) return SRCIO_OS;
	for (unsigned i = 0; i < _this->comp_cnt; i++) {
		switch(_this->comp_list[i].comp_dtype) {
		case DTYPE_U32:
			*up.u32 = _this->comp_list[i].comp.comp_u32;
			up.u32++;
			break;

		case DTYPE_I32:
			*up.i32 = _this->comp_list[i].comp.comp_i32;
			up.i32++;
			break;

		case DTYPE_U16:
			*up.u16 = _this->comp_list[i].comp.comp_u16;
			up.u16++;
			break;

		case DTYPE_I16:
			*up.i16 = _this->comp_list[i].comp.comp_i16;
			up.i16++;
			break;

		default:
			retval = SRCIO_BADARGS;
			goto srciomod_get_raw_return;
		}
	}

	srciomod_get_raw_return:
	osMutexRelease(_this->lock);

	return retval;
}

/*
 * Call the subclass function.
 */
SRCIO_R SourceIOMod_update(SourceIOMod *_this)
{
	return _this->vmt->_SourceIOMod_update(_this);
}

SRCIO_R SourceIOMod_ctrl(SourceIOMod *_this)
{
	return _this->vmt->_SourceIOMod_ctrl(_this);
}

SRCIO_R SourceIOMod_rawToMod(
		SourceIOMod *mod_dst,
		const void *raw_data_src,
		uint8_t raw_data_size)
{
	union {
		const void		*v;
		const uint16_t	*u16;
		const int16_t	*i16;
		const uint32_t 	*u32;
		const int32_t	*i32;
	} up;

	if (mod_dst->data_raw_size > raw_data_size)
		return SRCIO_BADARGS;

	up.v = raw_data_src;
	for (unsigned i = 0; i < mod_dst->comp_cnt; i++) {
		switch(mod_dst->comp_list[i].comp_dtype) {
		case DTYPE_U32:
			mod_dst->comp_list[i].comp.comp_u32 = *up.u32;
			up.u32++;
			break;

		case DTYPE_I32:
			mod_dst->comp_list[i].comp.comp_i32 = *up.i32;
			up.i32++;
			break;

		case DTYPE_U16:
			mod_dst->comp_list[i].comp.comp_u16 = *up.u16;
			up.u16++;
			break;

		case DTYPE_I16:
			mod_dst->comp_list[i].comp.comp_i16 = *up.i16;
			up.i16++;
			break;

		default:
			return SRCIO_BADARGS;
		}
	}

	return SRCIO_OK;
}

SRCIO_R SourceIOMod_lock_acq(SourceIOMod *_this)
{
	if (osMutexWait(_this->lock, 1000)!= osOK)
		return SRCIO_NR;
	return SRCIO_OK;
}

void SourceIOMod_lock_rel(SourceIOMod *_this)
{
	osMutexRelease(_this->lock);
}

