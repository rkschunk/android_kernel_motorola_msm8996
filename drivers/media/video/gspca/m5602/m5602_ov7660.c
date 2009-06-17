/*
 * Driver for the ov7660 sensor
 *
 * Copyright (C) 2009 Erik Andrén
 * Copyright (C) 2007 Ilyes Gouta. Based on the m5603x Linux Driver Project.
 * Copyright (C) 2005 m5603x Linux Driver Project <m5602@x3ng.com.br>
 *
 * Portions of code to USB interface and ALi driver software,
 * Copyright (c) 2006 Willem Duinker
 * v4l2 interface modeled after the V4L2 driver
 * for SN9C10x PC Camera Controllers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#include "m5602_ov7660.h"

static int ov7660_get_gain(struct gspca_dev *gspca_dev, __s32 *val);
static int ov7660_set_gain(struct gspca_dev *gspca_dev, __s32 val);
static int ov7660_get_blue_gain(struct gspca_dev *gspca_dev, __s32 *val);
static int ov7660_set_blue_gain(struct gspca_dev *gspca_dev, __s32 val);
static int ov7660_get_red_gain(struct gspca_dev *gspca_dev, __s32 *val);
static int ov7660_set_red_gain(struct gspca_dev *gspca_dev, __s32 val);

const static struct ctrl ov7660_ctrls[] = {
#define GAIN_IDX 1
	{
		{
			.id		= V4L2_CID_GAIN,
			.type		= V4L2_CTRL_TYPE_INTEGER,
			.name		= "gain",
			.minimum	= 0x00,
			.maximum	= 0xff,
			.step		= 0x1,
			.default_value	= OV7660_DEFAULT_GAIN,
			.flags		= V4L2_CTRL_FLAG_SLIDER
		},
		.set = ov7660_set_gain,
		.get = ov7660_get_gain
	},
#define BLUE_BALANCE_IDX 2
	{
		{
			.id		= V4L2_CID_BLUE_BALANCE,
			.type		= V4L2_CTRL_TYPE_INTEGER,
			.name		= "blue balance",
			.minimum	= 0x00,
			.maximum	= 0x7f,
			.step		= 0x1,
			.default_value	= OV7660_DEFAULT_BLUE_GAIN,
			.flags		= V4L2_CTRL_FLAG_SLIDER
		},
		.set = ov7660_set_blue_gain,
		.get = ov7660_get_blue_gain
	},
#define RED_BALANCE_IDX 3
	{
		{
			.id		= V4L2_CID_RED_BALANCE,
			.type		= V4L2_CTRL_TYPE_INTEGER,
			.name		= "red balance",
			.minimum	= 0x00,
			.maximum	= 0x7f,
			.step		= 0x1,
			.default_value	= OV7660_DEFAULT_RED_GAIN,
			.flags		= V4L2_CTRL_FLAG_SLIDER
		},
		.set = ov7660_set_red_gain,
		.get = ov7660_get_red_gain
	},
};

static struct v4l2_pix_format ov7660_modes[] = {
	{
		640,
		480,
		V4L2_PIX_FMT_SBGGR8,
		V4L2_FIELD_NONE,
		.sizeimage =
			640 * 480,
		.bytesperline = 640,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0
	}
};

static void ov7660_dump_registers(struct sd *sd);

int ov7660_probe(struct sd *sd)
{
	int err = 0, i;
	u8 prod_id = 0, ver_id = 0;

	s32 *sensor_settings;

	if (force_sensor) {
		if (force_sensor == OV7660_SENSOR) {
			info("Forcing an %s sensor", ov7660.name);
			goto sensor_found;
		}
		/* If we want to force another sensor,
		don't try to probe this one */
		return -ENODEV;
	}

	/* Do the preinit */
	for (i = 0; i < ARRAY_SIZE(preinit_ov7660) && !err; i++) {
		u8 data[2];

		if (preinit_ov7660[i][0] == BRIDGE) {
			err = m5602_write_bridge(sd,
				preinit_ov7660[i][1],
				preinit_ov7660[i][2]);
		} else {
			data[0] = preinit_ov7660[i][2];
			err = m5602_write_sensor(sd,
				preinit_ov7660[i][1], data, 1);
		}
	}
	if (err < 0)
		return err;

	if (m5602_read_sensor(sd, OV7660_PID, &prod_id, 1))
		return -ENODEV;

	if (m5602_read_sensor(sd, OV7660_VER, &ver_id, 1))
		return -ENODEV;

	info("Sensor reported 0x%x%x", prod_id, ver_id);

	if ((prod_id == 0x76) && (ver_id == 0x60)) {
		info("Detected a ov7660 sensor");
		goto sensor_found;
	}
	return -ENODEV;

sensor_found:
	sensor_settings = kmalloc(
		ARRAY_SIZE(ov7660_ctrls) * sizeof(s32), GFP_KERNEL);
	if (!sensor_settings)
		return -ENOMEM;

	sd->gspca_dev.cam.cam_mode = ov7660_modes;
	sd->gspca_dev.cam.nmodes = ARRAY_SIZE(ov7660_modes);
	sd->desc->ctrls = ov7660_ctrls;
	sd->desc->nctrls = ARRAY_SIZE(ov7660_ctrls);

	for (i = 0; i < ARRAY_SIZE(ov7660_ctrls); i++)
		sensor_settings[i] = ov7660_ctrls[i].qctrl.default_value;
	sd->sensor_priv = sensor_settings;

	return 0;
}

int ov7660_init(struct sd *sd)
{
	int i, err = 0;
	s32 *sensor_settings = sd->sensor_priv;

	/* Init the sensor */
	for (i = 0; i < ARRAY_SIZE(init_ov7660); i++) {
		u8 data[2];

		if (init_ov7660[i][0] == BRIDGE) {
			err = m5602_write_bridge(sd,
				init_ov7660[i][1],
				init_ov7660[i][2]);
		} else {
			data[0] = init_ov7660[i][2];
			err = m5602_write_sensor(sd,
					init_ov7660[i][1], data, 1);
		}
	}

	if (dump_sensor)
		ov7660_dump_registers(sd);

	err = ov7660_set_gain(&sd->gspca_dev, sensor_settings[GAIN_IDX]);
	if (err < 0)
		return err;

	return err;
}

int ov7660_start(struct sd *sd)
{
	return 0;
}

int ov7660_stop(struct sd *sd)
{
	return 0;
}

void ov7660_disconnect(struct sd *sd)
{
	ov7660_stop(sd);

	sd->sensor = NULL;
	kfree(sd->sensor_priv);
}

static int ov7660_get_gain(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	*val = sensor_settings[GAIN_IDX];
	PDEBUG(D_V4L2, "Read gain %d", *val);
	return 0;
}

static int ov7660_set_gain(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	u8 i2c_data;
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	PDEBUG(D_V4L2, "Setting gain to %d", val);

	sensor_settings[GAIN_IDX] = val;

	err = m5602_write_sensor(sd, OV7660_GAIN, &i2c_data, 1);
	return err;
}

static int ov7660_get_blue_gain(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	*val = sensor_settings[BLUE_BALANCE_IDX];
	PDEBUG(D_V4L2, "Read blue balance %d", *val);
	return 0;
}

static int ov7660_set_blue_gain(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	u8 i2c_data;
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	PDEBUG(D_V4L2, "Setting blue balance to %d", val);

	sensor_settings[BLUE_BALANCE_IDX] = val;

	err = m5602_write_sensor(sd, OV7660_BLUE_GAIN, &i2c_data, 1);
	return err;
}

static int ov7660_get_red_gain(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	*val = sensor_settings[RED_BALANCE_IDX];
	PDEBUG(D_V4L2, "Read red balance %d", *val);
	return 0;
}

static int ov7660_set_red_gain(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	u8 i2c_data;
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	PDEBUG(D_V4L2, "Setting red balance to %d", val);

	sensor_settings[RED_BALANCE_IDX] = val;

	err = m5602_write_sensor(sd, OV7660_RED_GAIN, &i2c_data, 1);
	return err;
}

static void ov7660_dump_registers(struct sd *sd)
{
	int address;
	info("Dumping the ov7660 register state");
	for (address = 0; address < 0xa9; address++) {
		u8 value;
		m5602_read_sensor(sd, address, &value, 1);
		info("register 0x%x contains 0x%x",
		     address, value);
	}

	info("ov7660 register state dump complete");

	info("Probing for which registers that are read/write");
	for (address = 0; address < 0xff; address++) {
		u8 old_value, ctrl_value;
		u8 test_value[2] = {0xff, 0xff};

		m5602_read_sensor(sd, address, &old_value, 1);
		m5602_write_sensor(sd, address, test_value, 1);
		m5602_read_sensor(sd, address, &ctrl_value, 1);

		if (ctrl_value == test_value[0])
			info("register 0x%x is writeable", address);
		else
			info("register 0x%x is read only", address);

		/* Restore original value */
		m5602_write_sensor(sd, address, &old_value, 1);
	}
}
