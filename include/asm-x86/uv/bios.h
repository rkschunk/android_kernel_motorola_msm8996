#ifndef ASM_X86__UV__BIOS_H
#define ASM_X86__UV__BIOS_H

/*
 * UV BIOS layer definitions.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  Copyright (c) 2008 Silicon Graphics, Inc.  All Rights Reserved.
 *  Copyright (c) Russ Anderson
 */

#include <linux/rtc.h>

/*
 * Values for the BIOS calls.  It is passed as the first * argument in the
 * BIOS call.  Passing any other value in the first argument will result
 * in a BIOS_STATUS_UNIMPLEMENTED return status.
 */
enum uv_bios_cmd {
	UV_BIOS_COMMON,
	UV_BIOS_GET_SN_INFO,
	UV_BIOS_FREQ_BASE
};

/*
 * Status values returned from a BIOS call.
 */
enum {
	BIOS_STATUS_SUCCESS		=  0,
	BIOS_STATUS_UNIMPLEMENTED	= -ENOSYS,
	BIOS_STATUS_EINVAL		= -EINVAL,
	BIOS_STATUS_UNAVAIL		= -EBUSY
};

/*
 * The UV system table describes specific firmware
 * capabilities available to the Linux kernel at runtime.
 */
struct uv_systab {
	char signature[4];	/* must be "UVST" */
	u32 revision;		/* distinguish different firmware revs */
	u64 function;		/* BIOS runtime callback function ptr */
};

enum {
	BIOS_FREQ_BASE_PLATFORM = 0,
	BIOS_FREQ_BASE_INTERVAL_TIMER = 1,
	BIOS_FREQ_BASE_REALTIME_CLOCK = 2
};

/*
 * bios calls have 6 parameters
 */
extern s64 uv_bios_call(enum uv_bios_cmd, u64, u64, u64, u64, u64);
extern s64 uv_bios_call_irqsave(enum uv_bios_cmd, u64, u64, u64, u64, u64);
extern s64 uv_bios_call_reentrant(enum uv_bios_cmd, u64, u64, u64, u64, u64);

extern void uv_bios_init(void);

extern long
x86_bios_freq_base(unsigned long which, unsigned long *ticks_per_second,
		   unsigned long *drift_info);

#endif /* ASM_X86__UV__BIOS_H */
