/*
 *   ALSA driver for TEA5757/5759 Philips AM/FM radio tuner chips
 *
 *	Copyright (c) 2004 Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <asm/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <media/v4l2-device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <sound/tea575x-tuner.h>

MODULE_AUTHOR("Jaroslav Kysela <perex@perex.cz>");
MODULE_DESCRIPTION("Routines for control of TEA5757/5759 Philips AM/FM radio tuner chips");
MODULE_LICENSE("GPL");

#define FREQ_LO		 (76U * 16000)
#define FREQ_HI		(108U * 16000)

/*
 * definitions
 */

#define TEA575X_BIT_SEARCH	(1<<24)		/* 1 = search action, 0 = tuned */
#define TEA575X_BIT_UPDOWN	(1<<23)		/* 0 = search down, 1 = search up */
#define TEA575X_BIT_MONO	(1<<22)		/* 0 = stereo, 1 = mono */
#define TEA575X_BIT_BAND_MASK	(3<<20)
#define TEA575X_BIT_BAND_FM	(0<<20)
#define TEA575X_BIT_BAND_MW	(1<<20)
#define TEA575X_BIT_BAND_LW	(1<<21)
#define TEA575X_BIT_BAND_SW	(1<<22)
#define TEA575X_BIT_PORT_0	(1<<19)		/* user bit */
#define TEA575X_BIT_PORT_1	(1<<18)		/* user bit */
#define TEA575X_BIT_SEARCH_MASK	(3<<16)		/* search level */
#define TEA575X_BIT_SEARCH_5_28	     (0<<16)	/* FM >5uV, AM >28uV */
#define TEA575X_BIT_SEARCH_10_40     (1<<16)	/* FM >10uV, AM > 40uV */
#define TEA575X_BIT_SEARCH_30_63     (2<<16)	/* FM >30uV, AM > 63uV */
#define TEA575X_BIT_SEARCH_150_1000  (3<<16)	/* FM > 150uV, AM > 1000uV */
#define TEA575X_BIT_DUMMY	(1<<15)		/* buffer */
#define TEA575X_BIT_FREQ_MASK	0x7fff

/*
 * lowlevel part
 */

static void snd_tea575x_write(struct snd_tea575x *tea, unsigned int val)
{
	u16 l;
	u8 data;

	tea->ops->set_direction(tea, 1);
	udelay(16);

	for (l = 25; l > 0; l--) {
		data = (val >> 24) & TEA575X_DATA;
		val <<= 1;			/* shift data */
		tea->ops->set_pins(tea, data | TEA575X_WREN);
		udelay(2);
		tea->ops->set_pins(tea, data | TEA575X_WREN | TEA575X_CLK);
		udelay(2);
		tea->ops->set_pins(tea, data | TEA575X_WREN);
		udelay(2);
	}

	if (!tea->mute)
		tea->ops->set_pins(tea, 0);
}

static u32 snd_tea575x_read(struct snd_tea575x *tea)
{
	u16 l, rdata;
	u32 data = 0;

	tea->ops->set_direction(tea, 0);
	tea->ops->set_pins(tea, 0);
	udelay(16);

	for (l = 24; l--;) {
		tea->ops->set_pins(tea, TEA575X_CLK);
		udelay(2);
		if (!l)
			tea->tuned = tea->ops->get_pins(tea) & TEA575X_MOST ? 0 : 1;
		tea->ops->set_pins(tea, 0);
		udelay(2);
		data <<= 1;			/* shift data */
		rdata = tea->ops->get_pins(tea);
		if (!l)
			tea->stereo = (rdata & TEA575X_MOST) ?  0 : 1;
		if (rdata & TEA575X_DATA)
			data++;
		udelay(2);
	}

	if (tea->mute)
		tea->ops->set_pins(tea, TEA575X_WREN);

	return data;
}

static u32 snd_tea575x_get_freq(struct snd_tea575x *tea)
{
	u32 freq = snd_tea575x_read(tea) & TEA575X_BIT_FREQ_MASK;

	if (freq == 0)
		return freq;

	/* freq *= 12.5 */
	freq *= 125;
	freq /= 10;
	/* crystal fixup */
	if (tea->tea5759)
		freq += TEA575X_FMIF;
	else
		freq -= TEA575X_FMIF;

	return clamp(freq * 16, FREQ_LO, FREQ_HI); /* from kHz */
}

static void snd_tea575x_set_freq(struct snd_tea575x *tea)
{
	u32 freq = tea->freq;

	freq /= 16;		/* to kHz */
	/* crystal fixup */
	if (tea->tea5759)
		freq -= TEA575X_FMIF;
	else
		freq += TEA575X_FMIF;
	/* freq /= 12.5 */
	freq *= 10;
	freq /= 125;

	tea->val &= ~TEA575X_BIT_FREQ_MASK;
	tea->val |= freq & TEA575X_BIT_FREQ_MASK;
	snd_tea575x_write(tea, tea->val);
}

/*
 * Linux Video interface
 */

static int vidioc_querycap(struct file *file, void  *priv,
					struct v4l2_capability *v)
{
	struct snd_tea575x *tea = video_drvdata(file);

	strlcpy(v->driver, tea->v4l2_dev->name, sizeof(v->driver));
	strlcpy(v->card, tea->card, sizeof(v->card));
	strlcat(v->card, tea->tea5759 ? " TEA5759" : " TEA5757", sizeof(v->card));
	strlcpy(v->bus_info, tea->bus_info, sizeof(v->bus_info));
	v->device_caps = V4L2_CAP_TUNER | V4L2_CAP_RADIO;
	if (!tea->cannot_read_data)
		v->device_caps |= V4L2_CAP_HW_FREQ_SEEK;
	v->capabilities = v->device_caps | V4L2_CAP_DEVICE_CAPS;
	return 0;
}

static int vidioc_g_tuner(struct file *file, void *priv,
					struct v4l2_tuner *v)
{
	struct snd_tea575x *tea = video_drvdata(file);

	if (v->index > 0)
		return -EINVAL;

	snd_tea575x_read(tea);

	strcpy(v->name, "FM");
	v->type = V4L2_TUNER_RADIO;
	v->capability = V4L2_TUNER_CAP_LOW | V4L2_TUNER_CAP_STEREO;
	v->rangelow = FREQ_LO;
	v->rangehigh = FREQ_HI;
	v->rxsubchans = tea->stereo ? V4L2_TUNER_SUB_STEREO : V4L2_TUNER_SUB_MONO;
	v->audmode = (tea->val & TEA575X_BIT_MONO) ?
		V4L2_TUNER_MODE_MONO : V4L2_TUNER_MODE_STEREO;
	v->signal = tea->tuned ? 0xffff : 0;
	return 0;
}

static int vidioc_s_tuner(struct file *file, void *priv,
					struct v4l2_tuner *v)
{
	struct snd_tea575x *tea = video_drvdata(file);

	if (v->index)
		return -EINVAL;
	tea->val &= ~TEA575X_BIT_MONO;
	if (v->audmode == V4L2_TUNER_MODE_MONO)
		tea->val |= TEA575X_BIT_MONO;
	snd_tea575x_write(tea, tea->val);
	return 0;
}

static int vidioc_g_frequency(struct file *file, void *priv,
					struct v4l2_frequency *f)
{
	struct snd_tea575x *tea = video_drvdata(file);

	if (f->tuner != 0)
		return -EINVAL;
	f->type = V4L2_TUNER_RADIO;
	f->frequency = tea->freq;
	return 0;
}

static int vidioc_s_frequency(struct file *file, void *priv,
					struct v4l2_frequency *f)
{
	struct snd_tea575x *tea = video_drvdata(file);

	if (f->tuner != 0 || f->type != V4L2_TUNER_RADIO)
		return -EINVAL;

	tea->val &= ~TEA575X_BIT_SEARCH;
	tea->freq = clamp(f->frequency, FREQ_LO, FREQ_HI);
	snd_tea575x_set_freq(tea);
	return 0;
}

static int vidioc_s_hw_freq_seek(struct file *file, void *fh,
					struct v4l2_hw_freq_seek *a)
{
	struct snd_tea575x *tea = video_drvdata(file);
	unsigned long timeout;
	int i;

	if (tea->cannot_read_data)
		return -ENOTTY;
	if (a->tuner || a->wrap_around)
		return -EINVAL;

	/* clear the frequency, HW will fill it in */
	tea->val &= ~TEA575X_BIT_FREQ_MASK;
	tea->val |= TEA575X_BIT_SEARCH;
	if (a->seek_upward)
		tea->val |= TEA575X_BIT_UPDOWN;
	else
		tea->val &= ~TEA575X_BIT_UPDOWN;
	snd_tea575x_write(tea, tea->val);
	timeout = jiffies + msecs_to_jiffies(10000);
	for (;;) {
		if (time_after(jiffies, timeout))
			break;
		if (schedule_timeout_interruptible(msecs_to_jiffies(10))) {
			/* some signal arrived, stop search */
			tea->val &= ~TEA575X_BIT_SEARCH;
			snd_tea575x_set_freq(tea);
			return -ERESTARTSYS;
		}
		if (!(snd_tea575x_read(tea) & TEA575X_BIT_SEARCH)) {
			u32 freq;

			/* Found a frequency, wait until it can be read */
			for (i = 0; i < 100; i++) {
				msleep(10);
				freq = snd_tea575x_get_freq(tea);
				if (freq) /* available */
					break;
			}
			if (freq == 0) /* shouldn't happen */
				break;
			/*
			 * if we moved by less than 50 kHz, or in the wrong
			 * direction, continue seeking
			 */
			if (abs(tea->freq - freq) < 16 * 50 ||
					(a->seek_upward && freq < tea->freq) ||
					(!a->seek_upward && freq > tea->freq)) {
				snd_tea575x_write(tea, tea->val);
				continue;
			}
			tea->freq = freq;
			tea->val &= ~TEA575X_BIT_SEARCH;
			return 0;
		}
	}
	tea->val &= ~TEA575X_BIT_SEARCH;
	snd_tea575x_set_freq(tea);
	return -EAGAIN;
}

static int tea575x_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct snd_tea575x *tea = container_of(ctrl->handler, struct snd_tea575x, ctrl_handler);

	switch (ctrl->id) {
	case V4L2_CID_AUDIO_MUTE:
		tea->mute = ctrl->val;
		snd_tea575x_set_freq(tea);
		return 0;
	}

	return -EINVAL;
}

static const struct v4l2_file_operations tea575x_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= video_ioctl2,
	.open           = v4l2_fh_open,
	.release        = v4l2_fh_release,
	.poll           = v4l2_ctrl_poll,
};

static const struct v4l2_ioctl_ops tea575x_ioctl_ops = {
	.vidioc_querycap    = vidioc_querycap,
	.vidioc_g_tuner     = vidioc_g_tuner,
	.vidioc_s_tuner     = vidioc_s_tuner,
	.vidioc_g_frequency = vidioc_g_frequency,
	.vidioc_s_frequency = vidioc_s_frequency,
	.vidioc_s_hw_freq_seek = vidioc_s_hw_freq_seek,
	.vidioc_log_status  = v4l2_ctrl_log_status,
	.vidioc_subscribe_event = v4l2_ctrl_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
};

static const struct video_device tea575x_radio = {
	.fops           = &tea575x_fops,
	.ioctl_ops 	= &tea575x_ioctl_ops,
	.release        = video_device_release_empty,
};

static const struct v4l2_ctrl_ops tea575x_ctrl_ops = {
	.s_ctrl = tea575x_s_ctrl,
};

/*
 * initialize all the tea575x chips
 */
int snd_tea575x_init(struct snd_tea575x *tea)
{
	int retval;

	tea->mute = true;

	/* Not all devices can or know how to read the data back.
	   Such devices can set cannot_read_data to true. */
	if (!tea->cannot_read_data) {
		snd_tea575x_write(tea, 0x55AA);
		if (snd_tea575x_read(tea) != 0x55AA)
			return -ENODEV;
	}

	tea->val = TEA575X_BIT_BAND_FM | TEA575X_BIT_SEARCH_5_28;
	tea->freq = 90500 * 16;		/* 90.5Mhz default */
	snd_tea575x_set_freq(tea);

	tea->vd = tea575x_radio;
	video_set_drvdata(&tea->vd, tea);
	mutex_init(&tea->mutex);
	strlcpy(tea->vd.name, tea->v4l2_dev->name, sizeof(tea->vd.name));
	tea->vd.lock = &tea->mutex;
	tea->vd.v4l2_dev = tea->v4l2_dev;
	tea->vd.ctrl_handler = &tea->ctrl_handler;
	set_bit(V4L2_FL_USE_FH_PRIO, &tea->vd.flags);

	v4l2_ctrl_handler_init(&tea->ctrl_handler, 1);
	v4l2_ctrl_new_std(&tea->ctrl_handler, &tea575x_ctrl_ops, V4L2_CID_AUDIO_MUTE, 0, 1, 1, 1);
	retval = tea->ctrl_handler.error;
	if (retval) {
		v4l2_err(tea->v4l2_dev, "can't initialize controls\n");
		v4l2_ctrl_handler_free(&tea->ctrl_handler);
		return retval;
	}

	if (tea->ext_init) {
		retval = tea->ext_init(tea);
		if (retval) {
			v4l2_ctrl_handler_free(&tea->ctrl_handler);
			return retval;
		}
	}

	v4l2_ctrl_handler_setup(&tea->ctrl_handler);

	retval = video_register_device(&tea->vd, VFL_TYPE_RADIO, tea->radio_nr);
	if (retval) {
		v4l2_err(tea->v4l2_dev, "can't register video device!\n");
		v4l2_ctrl_handler_free(&tea->ctrl_handler);
		return retval;
	}

	return 0;
}

void snd_tea575x_exit(struct snd_tea575x *tea)
{
	video_unregister_device(&tea->vd);
	v4l2_ctrl_handler_free(&tea->ctrl_handler);
}

static int __init alsa_tea575x_module_init(void)
{
	return 0;
}

static void __exit alsa_tea575x_module_exit(void)
{
}

module_init(alsa_tea575x_module_init)
module_exit(alsa_tea575x_module_exit)

EXPORT_SYMBOL(snd_tea575x_init);
EXPORT_SYMBOL(snd_tea575x_exit);
