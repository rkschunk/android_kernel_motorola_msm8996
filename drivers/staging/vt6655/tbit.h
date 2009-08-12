/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 * File: tbit.h
 *
 * Purpose: Bit routines
 *
 * Author: Tevin Chen
 *
 * Date: May 21, 1996
 *
 */

#ifndef __TBIT_H__
#define __TBIT_H__

#include "ttype.h"

/*---------------------  Export Definitions -------------------------*/

/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/

// test single bit on
#define BITbIsBitOn(tData, tTestBit)                \
    (((tData) & (tTestBit)) != 0)

// test single bit off
#define BITbIsBitOff(tData, tTestBit)               \
    (((tData) & (tTestBit)) == 0)


#define BITbIsAllBitsOn(tData, tTestBit)            \
    (((tData) & (tTestBit)) == (tTestBit))

#define BITbIsAllBitsOff(tData, tTestBit)           \
    (((tData) & (tTestBit)) == 0)

#define BITbIsAnyBitsOn(tData, tTestBit)            \
    (((tData) & (tTestBit)) != 0)

#define BITbIsAnyBitsOff(tData, tTestBit)           \
    (((tData) & (tTestBit)) != (tTestBit))

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/



#endif // __TBIT_H__



