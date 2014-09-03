/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __RTW_IOL_H_
#define __RTW_IOL_H_

#include <osdep_service.h>
#include <drv_types.h>

#define IOREG_CMD_END_LEN	4

struct ioreg_cfg {
	u8	length;
	u8	cmd_id;
	__le16	address;
	__le32	data;
	__le32  mask;
};

enum ioreg_cmd {
	IOREG_CMD_LLT		= 0x01,
	IOREG_CMD_REFUSE	= 0x02,
	IOREG_CMD_EFUSE_PATH	= 0x03,
	IOREG_CMD_WB_REG	= 0x04,
	IOREG_CMD_WW_REG	= 0x05,
	IOREG_CMD_WD_REG	= 0x06,
	IOREG_CMD_W_RF		= 0x07,
	IOREG_CMD_DELAY_US	= 0x10,
	IOREG_CMD_DELAY_MS	= 0x11,
	IOREG_CMD_END		= 0xFF,
};

struct xmit_frame *rtw_IOL_accquire_xmit_frame(struct adapter *adapter);
int rtw_IOL_append_cmds(struct xmit_frame *xmit_frame, u8 *IOL_cmds,
			u32 cmd_len);
int rtw_IOL_append_LLT_cmd(struct xmit_frame *xmit_frame, u8 page_boundary);
int rtw_IOL_exec_cmds_sync(struct adapter  *adapter,
			   struct xmit_frame *xmit_frame, u32 max_wating_ms,
			   u32 bndy_cnt);
bool rtw_IOL_applied(struct adapter  *adapter);
int rtw_IOL_append_DELAY_US_cmd(struct xmit_frame *xmit_frame, u16 us);
int rtw_IOL_append_DELAY_MS_cmd(struct xmit_frame *xmit_frame, u16 ms);
int rtw_IOL_append_END_cmd(struct xmit_frame *xmit_frame);

void read_efuse_from_txpktbuf(struct adapter *adapter, int bcnhead,
			      u8 *content, u16 *size);

int _rtw_IOL_append_WB_cmd(struct xmit_frame *xmit_frame, u16 addr,
			   u8 value, u8 mask);
int _rtw_IOL_append_WD_cmd(struct xmit_frame *xmit_frame, u16 addr,
			   u32 value, u32 mask);
int _rtw_IOL_append_WRF_cmd(struct xmit_frame *xmit_frame, u8 rf_path,
			    u16 addr, u32 value, u32 mask);
#define rtw_IOL_append_WB_cmd(xmit_frame, addr, value, mask)		\
	_rtw_IOL_append_WB_cmd((xmit_frame), (addr), (value) , (mask))
#define rtw_IOL_append_WW_cmd(xmit_frame, addr, value, mask)		\
	_rtw_IOL_append_WW_cmd((xmit_frame), (addr), (value), (mask))
#define rtw_IOL_append_WD_cmd(xmit_frame, addr, value, mask)		\
	_rtw_IOL_append_WD_cmd((xmit_frame), (addr), (value), (mask))
#define rtw_IOL_append_WRF_cmd(xmit_frame, rf_path, addr, value, mask)	\
	_rtw_IOL_append_WRF_cmd((xmit_frame), (rf_path), (addr), (value), (mask))

u8 rtw_IOL_cmd_boundary_handle(struct xmit_frame *pxmit_frame);
void  rtw_IOL_cmd_buf_dump(struct adapter  *Adapter, int buf_len, u8 *pbuf);

#endif /* __RTW_IOL_H_ */
