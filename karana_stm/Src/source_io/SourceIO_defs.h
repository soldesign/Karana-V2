/* Src/source_io/SourceIO_defs.h
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

#ifndef SOURCE_IO_SOURCEIO_DEFS_H_
#define SOURCE_IO_SOURCEIO_DEFS_H_

typedef enum {
	SRCIO_OK,		// OK
	SRCIO_NR,		// not ready
	SRCIO_NP,		// not present
	SRCIO_TOUT,		// timeout
	SRCIO_INTERN,	// internal error
	SRCIO_IOERR,	// I/O error
	SRCIO_MEM,		// memory fault
	SRCIO_MOD_NA,	// a module returned NR, NP, TOUT or IOERR
	SRCIO_MOD_ALLNA, // all modules returned NR, NP, TOUT or IOERR
	SRCIO_OS,		// The RTOS failed to fulfill a request
	SRCIO_BADARGS,

	SRCIO_R_ENUMSIZE
} SRCIO_R;


#endif /* SOURCE_IO_SOURCEIO_DEFS_H_ */
