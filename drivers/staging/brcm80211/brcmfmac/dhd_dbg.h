/*
 * Copyright (c) 2010 Broadcom Corporation
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _dhd_dbg_
#define _dhd_dbg_

#if defined(BCMDBG)

#define DHD_ERROR(args) \
	do {if ((brcmf_msg_level & BRCMF_ERROR_VAL) && (net_ratelimit())) \
		printk args; } while (0)
#define DHD_TRACE(args)		do {if (brcmf_msg_level & BRCMF_TRACE_VAL) \
					printk args; } while (0)
#define DHD_INFO(args)		do {if (brcmf_msg_level & BRCMF_INFO_VAL) \
					printk args; } while (0)
#define DHD_DATA(args)		do {if (brcmf_msg_level & BRCMF_DATA_VAL) \
					printk args; } while (0)
#define DHD_CTL(args)		do {if (brcmf_msg_level & BRCMF_CTL_VAL) \
					printk args; } while (0)
#define DHD_TIMER(args)		do {if (brcmf_msg_level & BRCMF_TIMER_VAL) \
					printk args; } while (0)
#define DHD_HDRS(args)		do {if (brcmf_msg_level & BRCMF_HDRS_VAL) \
					printk args; } while (0)
#define DHD_BYTES(args)		do {if (brcmf_msg_level & BRCMF_BYTES_VAL) \
					printk args; } while (0)
#define DHD_INTR(args)		do {if (brcmf_msg_level & BRCMF_INTR_VAL) \
					printk args; } while (0)
#define DHD_GLOM(args)		do {if (brcmf_msg_level & BRCMF_GLOM_VAL) \
					printk args; } while (0)
#define DHD_EVENT(args)		do {if (brcmf_msg_level & BRCMF_EVENT_VAL) \
					printk args; } while (0)

#define DHD_DATA_ON()		(brcmf_msg_level & BRCMF_DATA_VAL)
#define DHD_CTL_ON()		(brcmf_msg_level & BRCMF_CTL_VAL)
#define DHD_HDRS_ON()		(brcmf_msg_level & BRCMF_HDRS_VAL)
#define DHD_BYTES_ON()		(brcmf_msg_level & BRCMF_BYTES_VAL)
#define DHD_GLOM_ON()		(brcmf_msg_level & BRCMF_GLOM_VAL)

#else	/* (defined BCMDBG) || (defined BCMDBG) */

#define DHD_ERROR(args)  do {if (net_ratelimit()) printk args; } while (0)
#define DHD_TRACE(args)
#define DHD_INFO(args)
#define DHD_DATA(args)
#define DHD_CTL(args)
#define DHD_TIMER(args)
#define DHD_HDRS(args)
#define DHD_BYTES(args)
#define DHD_INTR(args)
#define DHD_GLOM(args)
#define DHD_EVENT(args)

#define DHD_DATA_ON()		0
#define DHD_CTL_ON()		0
#define DHD_HDRS_ON()		0
#define DHD_BYTES_ON()		0
#define DHD_GLOM_ON()		0

#endif				/* defined(BCMDBG) */

extern int brcmf_msg_level;

#endif				/* _dhd_dbg_ */
