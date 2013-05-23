/*
 * u_ncm.h
 *
 * Utility definitions for the ncm function
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Author: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef U_NCM_H
#define U_NCM_H

#include <linux/usb/composite.h>

struct f_ncm_opts {
	struct usb_function_instance	func_inst;
	struct net_device		*net;
	bool				bound;
};

#endif /* U_NCM_H */
