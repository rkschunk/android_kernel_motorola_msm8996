/*
 * saa7111 - Philips SAA7113A video decoder driver version 0.0.3
 *
 * Copyright (C) 1998 Dave Perks <dperks@ibm.net>
 *
 * Slight changes for video timing and attachment output by
 * Wolfgang Scherr <scherr@net4you.net>
 *
 * Changes by Ronald Bultje <rbultje@ronald.bitfreak.net>
 *    - moved over to linux>=2.4.x i2c protocol (1/1/2003)
 *
 * Changes by Michael Hunold <michael@mihu.de>
 *    - implemented DECODER_SET_GPIO, DECODER_INIT, DECODER_SET_VBI_BYPASS
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/signal.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/videodev.h>

MODULE_DESCRIPTION("Philips SAA7113 video decoder driver");
MODULE_AUTHOR("Dave Perks, Jose Ignacio Gijon, Joerg Heckenbach, Mark McClelland, Dwaine Garden");
MODULE_LICENSE("GPL");

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_NAME(s) (s)->name

#include <linux/video_decoder.h>

static int debug = 0;
MODULE_PARM(debug, "i");
MODULE_PARM_DESC(debug, " Set the default Debug level.  Default: 0 (Off) - (0-1)");


#define dprintk(num, format, args...) \
	do { \
		if (debug >= num) \
			printk(format , ##args); \
	} while (0)

/* ----------------------------------------------------------------------- */

struct saa7113 {
	unsigned char reg[32];

	int norm;
	int input;
	int enable;
	int bright;
	int contrast;
	int hue;
	int sat;
};

#define   I2C_SAA7113        0x4A
#define   I2C_SAA7114        0x42

/* ----------------------------------------------------------------------- */

static inline int
saa7113_write (struct i2c_client *client,
	       u8                 reg,
	       u8                 value)
{
	struct saa7113 *decoder = i2c_get_clientdata(client);

	decoder->reg[reg] = value;
	return i2c_smbus_write_byte_data(client, reg, value);
}

static int
saa7113_write_block (struct i2c_client *client,
		     const u8          *data,
		     unsigned int       len)
{
	int ret = -1;
	u8 reg;

	/* the saa7113 has an autoincrement function, use it if
	 * the adapter understands raw I2C */
	if (i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		/* do raw I2C, not smbus compatible */
		struct saa7113 *decoder = i2c_get_clientdata(client);
		struct i2c_msg msg;
		u8 block_data[32];

		msg.addr = client->addr;
		msg.flags = 0;
		while (len >= 2) {
			msg.buf = (char *) block_data;
			msg.len = 0;
			block_data[msg.len++] = reg = data[0];
			do {
				block_data[msg.len++] =
				    decoder->reg[reg++] = data[1];
				len -= 2;
				data += 2;
			} while (len >= 2 && data[0] == reg &&
				 msg.len < 32);
			if ((ret = i2c_transfer(client->adapter,
						&msg, 1)) < 0)
				break;
		}
	} else {
		/* do some slow I2C emulation kind of thing */
		while (len >= 2) {
			reg = *data++;
			if ((ret = saa7113_write(client, reg,
						 *data++)) < 0)
				break;
			len -= 2;
		}
	}

	return ret;
}

static int
saa7113_init_decoder (struct i2c_client *client,
	      struct video_decoder_init *init)
{
	return saa7113_write_block(client, init->data, init->len);
}

static inline int
saa7113_read (struct i2c_client *client,
	      u8                 reg)
{
	return i2c_smbus_read_byte_data(client, reg);
}

/* ----------------------------------------------------------------------- */

static const unsigned char saa7113_i2c_init[] = {
	0x00, 0x00,	/* PH7113_CHIP_VERSION 00 - ID byte */
	0x01, 0x08,	/* PH7113_INCREMENT_DELAY - (1) (1) (1) (1) IDEL3 IDEL2 IDELL1 IDEL0 */
	0x02, 0xc0,	/* PH7113_ANALOG_INPUT_CONTR_1 - FUSE1 FUSE0 GUDL1 GUDL0 MODE3 MODE2 MODE1 MODE0 */
	0x03, 0x23,	/* PH7113_ANALOG_INPUT_CONTR_2 - (1) HLNRS VBSL WPOFF HOLDG GAFIX GAI28 GAI18 */
	0x04, 0x00,	/* PH7113_ANALOG_INPUT_CONTR_3 - GAI17 GAI16 GAI15 GAI14 GAI13 GAI12 GAI11 GAI10 */
	0x05, 0x00,	/* PH7113_ANALOG_INPUT_CONTR_4 - GAI27 GAI26 GAI25 GAI24 GAI23 GAI22 GAI21 GAI20 */
	0x06, 0xeb,	/* PH7113_HORIZONTAL_SYNC_START - HSB7 HSB6 HSB5 HSB4 HSB3 HSB2 HSB1 HSB0 */
	0x07, 0xe0,	/* PH7113_HORIZONTAL_SYNC_STOP - HSS7 HSS6 HSS5 HSS4 HSS3 HSS2 HSS1 HSS0 */
	0x08, 0x88,	/* PH7113_SYNC_CONTROL - AUFD FSEL FOET HTC1 HTC0 HPLL VNOI1 VNOI0 */
	0x09, 0x00,	/* PH7113_LUMINANCE_CONTROL - BYPS PREF BPSS1 BPSS0 VBLB UPTCV APER1 APER0 */
	0x0a, 0x80,	/* PH7113_LUMINANCE_BRIGHTNESS - BRIG7 BRIG6 BRIG5 BRIG4 BRIG3 BRIG2 BRIG1 BRIG0 */
	0x0b, 0x47,	/* PH7113_LUMINANCE_CONTRAST - CONT7 CONT6 CONT5 CONT4 CONT3 CONT2 CONT1 CONT0 */
	0x0c, 0x40,	/* PH7113_CHROMA_SATURATION - SATN7 SATN6 SATN5 SATN4 SATN3 SATN2 SATN1 SATN0 */
	0x0d, 0x00,	/* PH7113_CHROMA_HUE_CONTROL - HUEC7 HUEC6 HUEC5 HUEC4 HUEC3 HUEC2 HUEC1 HUEC0 */
	0x0e, 0x01,	/* PH7113_CHROMA_CONTROL - CDTO CSTD2 CSTD1 CSTD0 DCCF FCTC CHBW1 CHBW0 */
	0x0f, 0xaa,	/* PH7113_CHROMA_GAIN_CONTROL - ACGC CGAIN6 CGAIN5 CGAIN4 CGAIN3 CGAIN2 CGAIN1 CGAIN0 */
	0x10, 0x00,	/* PH7113_FORMAT_DELAY_CONTROL - OFTS1 OFTS0 HDEL1 HDEL0 VRLN YDEL2 YDEL1 YDEL0 */
	0x11, 0x1C,	/* PH7113_OUTPUT_CONTROL_1 - GPSW1 CM99 GPSW0 HLSEL OEYC OERT VIPB COLO */
	0x12, 0x01,	/* PH7113_OUTPUT_CONTROL_2 - RTSE13 RTSE12 RTSE11 RTSE10 RTSE03 RTSE02 RTSE01 RTSE00 */
	0x13, 0x00,	/* PH7113_OUTPUT_CONTROL_3 - ADLSB (1) (1) OLDSB FIDP (1) AOSL1 AOSL0 */
	0x14, 0x00,	/* RESERVED 14 - (1) (1) (1) (1) (1) (1) (1) (1) */
	0x15, 0x00,	/* PH7113_V_GATE1_START - VSTA7 VSTA6 VSTA5 VSTA4 VSTA3 VSTA2 VSTA1 VSTA0 */
	0x16, 0x00,	/* PH7113_V_GATE1_STOP - VSTO7 VSTO6 VSTO5 VSTO4 VSTO3 VSTO2 VSTO1 VSTO0 */
	0x17, 0x00,	/* PH7113_V_GATE1_MSB - (1) (1) (1) (1) (1) (1) VSTO8 VSTA8 */
};

static int
saa7113_command (struct i2c_client *client,
		 unsigned int       cmd,
		 void              *arg)
{
	struct saa7113 *decoder = i2c_get_clientdata(client);

	switch (cmd) {

	case 0:
	case DECODER_INIT:
	{
		struct video_decoder_init *init = arg;
		if (NULL != init)
			return saa7113_init_decoder(client, init);
		else {
			struct video_decoder_init vdi;
			vdi.data = saa7113_i2c_init;
			vdi.len = sizeof(saa7113_i2c_init);
			return saa7113_init_decoder(client, &vdi);
		}
	}

	case DECODER_DUMP:
	{
		int i;

		for (i = 0; i < 32; i += 16) {
			int j;

			printk(KERN_DEBUG "%s: %03x", I2C_NAME(client), i);
			for (j = 0; j < 16; ++j) {
				printk(" %02x",
				       saa7113_read(client, i + j));
			}
			printk("\n");
		}
	}
		break;

	case DECODER_GET_CAPABILITIES:
	{
		struct video_decoder_capability *cap = arg;

		cap->flags = VIDEO_DECODER_PAL |
			     VIDEO_DECODER_NTSC |
			     VIDEO_DECODER_SECAM |
			     VIDEO_DECODER_AUTO |
			     VIDEO_DECODER_CCIR;
		cap->inputs = 8;
		cap->outputs = 1;
	}
		break;

	case DECODER_GET_STATUS:
	{
		int *iarg = arg;
		int status;
		int res;

		status = saa7113_read(client, 0x1f);
		dprintk(1, KERN_DEBUG "%s status: 0x%02x\n", I2C_NAME(client),
			status);
		res = 0;
		if ((status & (1 << 6)) == 0) {
			res |= DECODER_STATUS_GOOD;
		}
		switch (decoder->norm) {
		case VIDEO_MODE_NTSC:
			res |= DECODER_STATUS_NTSC;
			break;
		case VIDEO_MODE_PAL:
			res |= DECODER_STATUS_PAL;
			break;
		case VIDEO_MODE_SECAM:
			res |= DECODER_STATUS_SECAM;
			break;
		default:
		case VIDEO_MODE_AUTO:
			if ((status & (1 << 5)) != 0) {
				res |= DECODER_STATUS_NTSC;
			} else {
				res |= DECODER_STATUS_PAL;
			}
			break;
		}
		if ((status & (1 << 0)) != 0) {
			res |= DECODER_STATUS_COLOR;
		}
		*iarg = res;
	}
		break;

	case DECODER_SET_GPIO:
	{
		int *iarg = arg;
		if (0 != *iarg) {
			saa7113_write(client, 0x11,
				(decoder->reg[0x11] | 0x80));
		} else {
			saa7113_write(client, 0x11,
				(decoder->reg[0x11] & 0x7f));
		}
		break;
	}

	case DECODER_SET_VBI_BYPASS:
	{
		int *iarg = arg;
		if (0 != *iarg) {
			saa7113_write(client, 0x13,
				(decoder->reg[0x13] & 0xf0) | 0x0a);
		} else {
			saa7113_write(client, 0x13,
				(decoder->reg[0x13] & 0xf0));
		}
		break;
	}

	case DECODER_SET_NORM:
	{
		int *iarg = arg;

		switch (*iarg) {

		case VIDEO_MODE_NTSC:
			saa7113_write(client, 0x08,
				      (decoder->reg[0x08] & 0x3f) | 0x40);
			saa7113_write(client, 0x0e,
				      (decoder->reg[0x0e] & 0x8f));
			break;

		case VIDEO_MODE_PAL:
			saa7113_write(client, 0x08,
				      (decoder->reg[0x08] & 0x3f) | 0x00);
			saa7113_write(client, 0x0e,
				      (decoder->reg[0x0e] & 0x8f));
			break;

		case VIDEO_MODE_SECAM:
			saa7113_write(client, 0x08,
				      (decoder->reg[0x0e] & 0x3f) | 0x00);
			saa7113_write(client, 0x0e,
				      (decoder->reg[0x0e] & 0x8f) | 0x50);
			break;

		case VIDEO_MODE_AUTO:
			saa7113_write(client, 0x08,
				      (decoder->reg[0x08] & 0x3f) | 0x80);
			saa7113_write(client, 0x0e,
				      (decoder->reg[0x0e] & 0x8f));
			break;

		default:
			return -EINVAL;

		}
		decoder->norm = *iarg;
	}
		break;

	case DECODER_SET_INPUT:
	{
		int *iarg = arg;
		if (*iarg < 0 || *iarg > 9) {
			return -EINVAL;
		}
		if (decoder->input != *iarg) {
			decoder->input = *iarg;
			/* select mode */
			saa7113_write(client, 0x02,
				      (decoder->reg[0x02] & 0xf0) | decoder->input);
			/* bypass chrominance trap for modes 4..7 */
			saa7113_write(client, 0x09,
				      (decoder->reg[0x09] & 0x7f) | ((decoder->input > 3) ? 0x80 : 0));
		}
	}
		break;

	case DECODER_SET_OUTPUT:
	{
		int *iarg = arg;

		/* not much choice of outputs */
		if (*iarg != 0) {
			return -EINVAL;
		}
	}
		break;

	case DECODER_ENABLE_OUTPUT:
	{
		int *iarg = arg;
		int enable = (*iarg != 0);

		if (decoder->enable != enable) {
			decoder->enable = enable;

			/* RJ: If output should be disabled (for
			 * playing videos), we also need a open PLL.
			 * The input is set to 0 (where no input
			 * source is connected), although this
			 * is not necessary.
			 *
			 * If output should be enabled, we have to
			 * reverse the above.
			 */

			if (decoder->enable) {
				saa7113_write(client, 0x02,
					      (decoder->
					       reg[0x02] & 0xf8) |
					      decoder->input);
				saa7113_write(client, 0x08,
					      (decoder->reg[0x08] & 0xfb));
				saa7113_write(client, 0x11,
					      (decoder->
					       reg[0x11] & 0xf3) | 0x0c);
			} else {
				saa7113_write(client, 0x02,
					      (decoder->reg[0x02] & 0xf8));
				saa7113_write(client, 0x08,
					      (decoder->
					       reg[0x08] & 0xfb) | 0x04);
				saa7113_write(client, 0x11,
					      (decoder->reg[0x11] & 0xf3));
			}
		}
	}
		break;

	case DECODER_SET_PICTURE:
	{
		struct video_picture *pic = arg;

		if (decoder->bright != pic->brightness) {
			/* We want 0 to 255 we get 0-65535 */
			decoder->bright = pic->brightness;
			saa7113_write(client, 0x0a, decoder->bright >> 8);
		}
		if (decoder->contrast != pic->contrast) {
			/* We want 0 to 127 we get 0-65535 */
			decoder->contrast = pic->contrast;
			saa7113_write(client, 0x0b,
				      decoder->contrast >> 9);
		}
		if (decoder->sat != pic->colour) {
			/* We want 0 to 127 we get 0-65535 */
			decoder->sat = pic->colour;
			saa7113_write(client, 0x0c, decoder->sat >> 9);
		}
		if (decoder->hue != pic->hue) {
			/* We want -128 to 127 we get 0-65535 */
			decoder->hue = pic->hue;
			saa7113_write(client, 0x0d,
				      (decoder->hue - 32768) >> 8);
		}
	}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/* ----------------------------------------------------------------------- */

/*
 * Generic i2c probe
 * concerning the addresses: i2c wants 7 bit (without the r/w bit), so '>>1'
 */

/* standard i2c insmod options */
static unsigned short normal_i2c[] = {
	I2C_SAA7113>>1,         /* saa7113 */
	I2C_SAA7114>>1,         /* saa7114 */
	I2C_CLIENT_END
};

I2C_CLIENT_INSMOD;


static struct i2c_driver i2c_driver_saa7113;

static int
saa7113_detect_client (struct i2c_adapter *adapter,
		       int                 address,
		       int                 kind)
{
	int i;
	struct i2c_client *client;
	struct saa7113 *decoder;
	struct video_decoder_init vdi;

	dprintk(1,
		KERN_INFO
		"saa7113.c: detecting saa7113 client on address 0x%x\n",
		address << 1);

	/* Check if the adapter supports the needed features */
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return 0;

	client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (client == 0)
		return -ENOMEM;
	memset(client, 0, sizeof(struct i2c_client));
	client->addr = address;
	client->adapter = adapter;
	client->driver = &i2c_driver_saa7113;
	client->flags = I2C_CLIENT_ALLOW_USE;
	strlcpy(I2C_NAME(client), "saa7113", sizeof(I2C_NAME(client)));
	decoder = kmalloc(sizeof(struct saa7113), GFP_KERNEL);
	if (decoder == NULL) {
		kfree(client);
		return -ENOMEM;
	}
	memset(decoder, 0, sizeof(struct saa7113));
	decoder->norm = VIDEO_MODE_NTSC;
	decoder->input = 0;
	decoder->enable = 1;
	decoder->bright = 32768;
	decoder->contrast = 32768;
	decoder->hue = 32768;
	decoder->sat = 32768;
	i2c_set_clientdata(client, decoder);

	i = i2c_attach_client(client);
	if (i) {
		kfree(client);
		kfree(decoder);
		return i;
	}

	vdi.data = saa7113_i2c_init;
	vdi.len = sizeof(saa7113_i2c_init);
	i = saa7113_init_decoder(client, &vdi);
	if (i < 0) {
		dprintk(1, KERN_ERR "%s_attach error: init status %d\n",
			I2C_NAME(client), i);
	} else {
		dprintk(1,
			KERN_INFO
			"%s_attach: chip version %x at address 0x%x\n",
			I2C_NAME(client), saa7113_read(client, 0x00) >> 4,
			client->addr << 1);
	}

	return 0;
}

static int
saa7113_attach_adapter (struct i2c_adapter *adapter)
{
	dprintk(1,
		KERN_INFO
		"saa7113.c: starting probe for adapter %s (0x%x)\n",
		I2C_NAME(adapter), adapter->id);
	return i2c_probe(adapter, &addr_data, &saa7113_detect_client);
}

static int
saa7113_detach_client (struct i2c_client *client)
{
	struct saa7113 *decoder = i2c_get_clientdata(client);
	int err;

	err = i2c_detach_client(client);
	if (err) {
		return err;
	}

	kfree(decoder);
	kfree(client);

	return 0;
}

/* ----------------------------------------------------------------------- */

static struct i2c_driver i2c_driver_saa7113 = {
	.owner = THIS_MODULE,
	.name = "saa7113",

	.id = I2C_DRIVERID_SAA7113,
	.flags = I2C_DF_NOTIFY,

	.attach_adapter = saa7113_attach_adapter,
	.detach_client = saa7113_detach_client,
	.command = saa7113_command,
};

static int __init
saa7113_init (void)
{
	return i2c_add_driver(&i2c_driver_saa7113);
}

static void __exit
saa7113_exit (void)
{
	i2c_del_driver(&i2c_driver_saa7113);
}

module_init(saa7113_init);
module_exit(saa7113_exit);
