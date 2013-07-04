/*
 * cyttsp_i2c_common.c
 * Cypress TrueTouch(TM) Standard Product (TTSP) I2C touchscreen driver.
 * For use with Cypress Txx3xx and Txx4xx parts.
 * Supported parts include:
 * CY8CTST341
 * CY8CTMA340
 * TMA4XX
 * TMA1036
 *
 * Copyright (C) 2009, 2010, 2011 Cypress Semiconductor, Inc.
 * Copyright (C) 2012 Javier Martinez Canillas <javier@dowhile0.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Contact Cypress Semiconductor at www.cypress.com <ttdrivers@cypress.com>
 *
 */

#include <linux/device.h>
#include <linux/export.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/types.h>

int cyttsp_i2c_read_block_data(struct device *dev, u8 *xfer_buf,
				      u8 addr, u8 length, void *values)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct i2c_msg msgs[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = 1,
			.buf = &addr,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = values,
		},
	};
	int retval;

	retval = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (retval < 0)
		return retval;

	return retval != ARRAY_SIZE(msgs) ? -EIO : 0;
}
EXPORT_SYMBOL_GPL(cyttsp_i2c_read_block_data);

int cyttsp_i2c_write_block_data(struct device *dev, u8 *xfer_buf,
				       u8 addr, u8 length, const void *values)
{
	struct i2c_client *client = to_i2c_client(dev);
	int retval;

	xfer_buf[0] = addr;
	memcpy(&xfer_buf[1], values, length);

	retval = i2c_master_send(client, xfer_buf, length + 1);

	return retval < 0 ? retval : 0;
}
EXPORT_SYMBOL_GPL(cyttsp_i2c_write_block_data);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cypress");
