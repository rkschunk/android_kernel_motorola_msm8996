/*
 * ad2s120x.c simple support for the ADI Resolver to Digital Converters: AD2S1200/1205
 *
 * Copyright (c) 2010-2010 Analog Devices Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/module.h>

#include "../iio.h"
#include "../sysfs.h"

#define DRV_NAME "ad2s120x"

/* input pin sample and rdvel is controlled by driver */
#define AD2S120X_PN	2

/* input clock on serial interface */
#define AD2S120X_HZ	8192000
/* clock period in nano second */
#define AD2S120X_TSCLK	(1000000000/AD2S120X_HZ)

struct ad2s120x_state {
	struct mutex lock;
	struct spi_device *sdev;
	int sample;
	int rdvel;
	u8 rx[2] ____cacheline_aligned;
};

static int ad2s1200_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val,
			   int *val2,
			   long m)
{
	int ret = 0;
	s16 vel;
	struct ad2s120x_state *st = iio_priv(indio_dev);

	mutex_lock(&st->lock);
	gpio_set_value(st->sample, 0);
	/* delay (6 * AD2S120X_TSCLK + 20) nano seconds */
	udelay(1);
	gpio_set_value(st->sample, 1);
	gpio_set_value(st->rdvel, !!(chan->type == IIO_ANGL));
	ret = spi_read(st->sdev, st->rx, 2);
	if (ret < 0) {
		mutex_unlock(&st->lock);
		return ret;
	}

	switch (chan->type) {
	case IIO_ANGL:
		*val = (((u16)(st->rx[0])) << 4) | ((st->rx[1] & 0xF0) >> 4);
		break;
	case IIO_ANGL_VEL:
		vel = (((s16)(st->rx[0])) << 4) | ((st->rx[1] & 0xF0) >> 4);
		vel = (vel << 4) >> 4;
		*val = vel;
	default:
		mutex_unlock(&st->lock);
		return -EINVAL;
	}
	/* delay (2 * AD2S120X_TSCLK + 20) ns for sample pulse */
	udelay(1);
	mutex_unlock(&st->lock);
	return IIO_VAL_INT;
}

static const struct iio_chan_spec ad2s1200_channels[] = {
	{
		.type = IIO_ANGL,
		.indexed = 1,
		.channel = 0,
	}, {
		.type = IIO_ANGL_VEL,
		.indexed = 1,
		.channel = 0,
	}
};

static const struct iio_info ad2s120x_info = {
	.read_raw = &ad2s1200_read_raw,
	.driver_module = THIS_MODULE,
};

static int __devinit ad2s120x_probe(struct spi_device *spi)
{
	struct ad2s120x_state *st;
	struct iio_dev *indio_dev;
	int pn, ret = 0;
	unsigned short *pins = spi->dev.platform_data;

	for (pn = 0; pn < AD2S120X_PN; pn++)
		if (gpio_request_one(pins[pn], GPIOF_DIR_OUT, DRV_NAME)) {
			pr_err("%s: request gpio pin %d failed\n",
						DRV_NAME, pins[pn]);
			goto error_ret;
		}
	indio_dev = iio_allocate_device(sizeof(*st));
	if (indio_dev == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}
	spi_set_drvdata(spi, indio_dev);
	st = iio_priv(indio_dev);
	mutex_init(&st->lock);
	st->sdev = spi;
	st->sample = pins[0];
	st->rdvel = pins[1];

	indio_dev->dev.parent = &spi->dev;
	indio_dev->info = &ad2s120x_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = ad2s1200_channels;
	indio_dev->num_channels = ARRAY_SIZE(ad2s1200_channels);

	ret = iio_device_register(indio_dev);
	if (ret)
		goto error_free_dev;

	spi->max_speed_hz = AD2S120X_HZ;
	spi->mode = SPI_MODE_3;
	spi_setup(spi);

	return 0;

error_free_dev:
	iio_free_device(indio_dev);
error_ret:
	for (--pn; pn >= 0; pn--)
		gpio_free(pins[pn]);
	return ret;
}

static int __devexit ad2s120x_remove(struct spi_device *spi)
{
	iio_device_unregister(spi_get_drvdata(spi));

	return 0;
}

static struct spi_driver ad2s120x_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ad2s120x_probe,
	.remove = __devexit_p(ad2s120x_remove),
};

static __init int ad2s120x_spi_init(void)
{
	return spi_register_driver(&ad2s120x_driver);
}
module_init(ad2s120x_spi_init);

static __exit void ad2s120x_spi_exit(void)
{
	spi_unregister_driver(&ad2s120x_driver);
}
module_exit(ad2s120x_spi_exit);

MODULE_AUTHOR("Graff Yang <graff.yang@gmail.com>");
MODULE_DESCRIPTION("Analog Devices AD2S1200/1205 Resolver to Digital SPI driver");
MODULE_LICENSE("GPL v2");
