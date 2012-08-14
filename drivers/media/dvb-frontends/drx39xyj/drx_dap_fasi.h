/*******************************************************************************
* FILENAME: $Id: drx_dap_fasi.h,v 1.5 2009/07/07 14:21:40 justin Exp $
*
* DESCRIPTION:
* Part of DRX driver.
* Data access protocol: Fast Access Sequential Interface (fasi)
* Fast access, because of short addressing format (16 instead of 32 bits addr)
* Sequential, because of I2C.
*
* USAGE:
* Include.
*
* NOTES:
* $(c) 2008-2009 Trident Microsystems, Inc. - All rights reserved.
*
* This software and related documentation (the 'Software') are intellectual
* property owned by Trident and are copyright of Trident, unless specifically
* noted otherwise.
*
* Any use of the Software is permitted only pursuant to the terms of the
* license agreement, if any, which accompanies, is included with or applicable
* to the Software ('License Agreement') or upon express written consent of
* Trident. Any copying, reproduction or redistribution of the Software in
* whole or in part by any means not in accordance with the License Agreement
* or as agreed in writing by Trident is expressly prohibited.
*
* THE SOFTWARE IS WARRANTED, IF AT ALL, ONLY ACCORDING TO THE TERMS OF THE
* LICENSE AGREEMENT. EXCEPT AS WARRANTED IN THE LICENSE AGREEMENT THE SOFTWARE
* IS DELIVERED 'AS IS' AND TRIDENT HEREBY DISCLAIMS ALL WARRANTIES AND
* CONDITIONS WITH REGARD TO THE SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
* AND CONDITIONS OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIT
* ENJOYMENT, TITLE AND NON-INFRINGEMENT OF ANY THIRD PARTY INTELLECTUAL
* PROPERTY OR OTHER RIGHTS WHICH MAY RESULT FROM THE USE OR THE INABILITY
* TO USE THE SOFTWARE.
*
* IN NO EVENT SHALL TRIDENT BE LIABLE FOR INDIRECT, INCIDENTAL, CONSEQUENTIAL,
* PUNITIVE, SPECIAL OR OTHER DAMAGES WHATSOEVER INCLUDING WITHOUT LIMITATION,
* DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS
* INFORMATION, AND THE LIKE, ARISING OUT OF OR RELATING TO THE USE OF OR THE
* INABILITY TO USE THE SOFTWARE, EVEN IF TRIDENT HAS BEEN ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGES, EXCEPT PERSONAL INJURY OR DEATH RESULTING FROM
* TRIDENT'S NEGLIGENCE.                                                        $
*
*
*******************************************************************************/

/*-------- compilation control switches --------------------------------------*/

#ifndef __DRX_DAP_FASI_H__
#define __DRX_DAP_FASI_H__

/*-------- Required includes -------------------------------------------------*/

#include "drx_driver.h"

/*-------- Defines, configuring the API --------------------------------------*/

/********************************************
* Allowed address formats
********************************************/

/*
* Comments about short/long addressing format:
*
* The DAP FASI offers long address format (4 bytes) and short address format
* (2 bytes). The DAP can operate in 3 modes:
* (1) only short
* (2) only long
* (3) both long and short but short preferred and long only when necesarry
*
* These modes must be selected compile time via compile switches.
* Compile switch settings for the diffrent modes:
* (1) DRXDAPFASI_LONG_ADDR_ALLOWED=0, DRXDAPFASI_SHORT_ADDR_ALLOWED=1
* (2) DRXDAPFASI_LONG_ADDR_ALLOWED=1, DRXDAPFASI_SHORT_ADDR_ALLOWED=0
* (3) DRXDAPFASI_LONG_ADDR_ALLOWED=1, DRXDAPFASI_SHORT_ADDR_ALLOWED=1
*
* The default setting will be (3) both long and short.
* The default setting will need no compile switches.
* The default setting must be overridden if compile switches are already
* defined.
*
*/

/* set default */
#if !defined( DRXDAPFASI_LONG_ADDR_ALLOWED )
#define  DRXDAPFASI_LONG_ADDR_ALLOWED 1
#endif

/* set default */
#if !defined( DRXDAPFASI_SHORT_ADDR_ALLOWED )
#define  DRXDAPFASI_SHORT_ADDR_ALLOWED 1
#endif

/* check */
#if ( ( DRXDAPFASI_LONG_ADDR_ALLOWED==0 ) && \
      ( DRXDAPFASI_SHORT_ADDR_ALLOWED==0 ) )
#error  At least one of short- or long-addressing format must be allowed.
*;   /* illegal statement to force compiler error */
#endif


/********************************************
* Single/master multi master setting
********************************************/
/*
* Comments about SINGLE MASTER/MULTI MASTER  modes:
*
* Consider the two sides:1) the master and 2)the slave.
*
* Master:
* Single/multimaster operation set via DRXDAP_SINGLE_MASTER compile switch
*  + single master mode means no use of repeated starts
*  + multi master mode means use of repeated starts
*  Default is single master.
*  Default can be overriden by setting the compile switch DRXDAP_SINGLE_MASTER.
*
* Slave:
* Single/multi master selected via the flags in the FASI protocol.
*  + single master means remember memory address between i2c packets
*  + multimaster means flush memory address between i2c packets
*  Default is single master, DAP FASI changes multi-master setting silently
*  into single master setting. This cannot be overrriden.
*
*/
/* set default */
#ifndef DRXDAP_SINGLE_MASTER
#define DRXDAP_SINGLE_MASTER 0
#endif

/********************************************
* Chunk/mode checking
********************************************/
/*
* Comments about DRXDAP_MAX_WCHUNKSIZE in single or multi master mode and
* in combination with short and long addressing format. All text below
* assumes long addressing format. The table also includes information
* for short ADDRessing format.
*
* In single master mode, data can be written by sending the register address
* first, then two or four bytes of data in the next packet.
* Because the device address plus a register address equals five bytes,
* the mimimum chunk size must be five.
* If ten-bit I2C device addresses are used, the minimum chunk size must be six,
* because the I2C device address will then occupy two bytes when writing.
*
* Data in single master mode is transferred as follows:
* <S> <devW>  a0  a1  a2  a3  <P>
* <S> <devW>  d0  d1 [d2  d3] <P>
* ..
* or
* ..
* <S> <devW>  a0  a1  a2  a3  <P>
* <S> <devR> --- <P>
*
* In multi-master mode, the data must immediately follow the address (an I2C
* stop resets the internal address), and hence the minimum chunk size is
* 1 <I2C address> + 4 (register address) + 2 (data to send) = 7 bytes (8 if
* 10-bit I2C device addresses are used).
*
* The 7-bit or 10-bit i2c address parameters is a runtime parameter.
* The other parameters can be limited via compile time switches.
*
*-------------------------------------------------------------------------------
*
*  Minimum chunk size table (in bytes):
*
*       +----------------+----------------+
*       | 7b i2c addr    | 10b i2c addr   |
*       +----------------+----------------+
*       | single | multi | single | multi |
* ------+--------+-------+--------+-------+
* short | 3      | 5     | 4      | 6     |
* long  | 5      | 7     | 6      | 8     |
* ------+--------+-------+--------+-------+
*
*/

/* set default */
#if !defined( DRXDAP_MAX_WCHUNKSIZE)
#define  DRXDAP_MAX_WCHUNKSIZE 254
#endif

/* check */
#if ( (DRXDAPFASI_LONG_ADDR_ALLOWED==0)&&(DRXDAPFASI_SHORT_ADDR_ALLOWED==1) )
#if DRXDAP_SINGLE_MASTER
#define  DRXDAP_MAX_WCHUNKSIZE_MIN 3
#else
#define  DRXDAP_MAX_WCHUNKSIZE_MIN 5
#endif
#else
#if DRXDAP_SINGLE_MASTER
#define  DRXDAP_MAX_WCHUNKSIZE_MIN 5
#else
#define  DRXDAP_MAX_WCHUNKSIZE_MIN 7
#endif
#endif

#if  DRXDAP_MAX_WCHUNKSIZE <  DRXDAP_MAX_WCHUNKSIZE_MIN
#if ( (DRXDAPFASI_LONG_ADDR_ALLOWED==0)&&(DRXDAPFASI_SHORT_ADDR_ALLOWED==1) )
#if DRXDAP_SINGLE_MASTER
#error  DRXDAP_MAX_WCHUNKSIZE must be at least 3 in single master mode
*;   /* illegal statement to force compiler error */
#else
#error  DRXDAP_MAX_WCHUNKSIZE must be at least 5 in multi master mode
*;   /* illegal statement to force compiler error */
#endif
#else
#if DRXDAP_SINGLE_MASTER
#error  DRXDAP_MAX_WCHUNKSIZE must be at least 5 in single master mode
*;   /* illegal statement to force compiler error */
#else
#error  DRXDAP_MAX_WCHUNKSIZE must be at least 7 in multi master mode
*;   /* illegal statement to force compiler error */
#endif
#endif
#endif

/* set default */
#if !defined( DRXDAP_MAX_RCHUNKSIZE)
#define  DRXDAP_MAX_RCHUNKSIZE 254
#endif

/* check */
#if  DRXDAP_MAX_RCHUNKSIZE < 2
#error  DRXDAP_MAX_RCHUNKSIZE must be at least 2
*;   /* illegal statement to force compiler error */
#endif

/* check */
#if  DRXDAP_MAX_RCHUNKSIZE & 1
#error  DRXDAP_MAX_RCHUNKSIZE must be even
*;   /* illegal statement to force compiler error */
#endif

/*-------- Public API functions ----------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif


extern DRXAccessFunc_t drxDapFASIFunct_g;

#define DRXDAP_FASI_RMW           0x10000000
#define DRXDAP_FASI_BROADCAST     0x20000000
#define DRXDAP_FASI_CLEARCRC      0x80000000
#define DRXDAP_FASI_SINGLE_MASTER 0xC0000000
#define DRXDAP_FASI_MULTI_MASTER  0x40000000
#define DRXDAP_FASI_SMM_SWITCH    0x40000000    /* single/multi master switch */
#define DRXDAP_FASI_MODEFLAGS     0xC0000000
#define DRXDAP_FASI_FLAGS         0xF0000000

#define DRXDAP_FASI_ADDR2BLOCK( addr )  (((addr)>>22)&0x3F)
#define DRXDAP_FASI_ADDR2BANK( addr )   (((addr)>>16)&0x3F)
#define DRXDAP_FASI_ADDR2OFFSET( addr ) ((addr)&0x7FFF)

#define DRXDAP_FASI_SHORT_FORMAT( addr )     (((addr)& 0xFC30FF80)==0)
#define DRXDAP_FASI_LONG_FORMAT( addr )      (((addr)& 0xFC30FF80)!=0)
#define DRXDAP_FASI_OFFSET_TOO_LARGE( addr ) (((addr)& 0x00008000)!=0)


#ifdef __cplusplus
}
#endif


#endif         /* __DRX_DAP_FASI_H__ */
