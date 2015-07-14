/*
 * vivid-vid-out.c - video output support functions.
 *
 * Copyright 2014 Cisco Systems, Inc. and/or its affiliates. All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/videodev2.h>
#include <linux/v4l2-dv-timings.h>
#include <media/v4l2-common.h>
#include <media/v4l2-event.h>
#include <media/v4l2-dv-timings.h>

#include "vivid-core.h"
#include "vivid-vid-common.h"
#include "vivid-kthread-out.h"
#include "vivid-vid-out.h"

static int vid_out_queue_setup(struct vb2_queue *vq, const struct v4l2_format *fmt,
		       unsigned *nbuffers, unsigned *nplanes,
		       unsigned sizes[], void *alloc_ctxs[])
{
	struct vivid_dev *dev = vb2_get_drv_priv(vq);
	unsigned planes = dev->fmt_out->planes;
	unsigned h = dev->fmt_out_rect.height;
	unsigned size = dev->bytesperline_out[0] * h;

	if (dev->field_out == V4L2_FIELD_ALTERNATE) {
		/*
		 * You cannot use write() with FIELD_ALTERNATE since the field
		 * information (TOP/BOTTOM) cannot be passed to the kernel.
		 */
		if (vb2_fileio_is_active(vq))
			return -EINVAL;
	}

	if (dev->queue_setup_error) {
		/*
		 * Error injection: test what happens if queue_setup() returns
		 * an error.
		 */
		dev->queue_setup_error = false;
		return -EINVAL;
	}

	if (fmt) {
		const struct v4l2_pix_format_mplane *mp;
		struct v4l2_format mp_fmt;

		if (!V4L2_TYPE_IS_MULTIPLANAR(fmt->type)) {
			fmt_sp2mp(fmt, &mp_fmt);
			fmt = &mp_fmt;
		}
		mp = &fmt->fmt.pix_mp;
		/*
		 * Check if the number of planes in the specified format match
		 * the number of planes in the current format. You can't mix that.
		 */
		if (mp->num_planes != planes)
			return -EINVAL;
		sizes[0] = mp->plane_fmt[0].sizeimage;
		if (planes == 2) {
			sizes[1] = mp->plane_fmt[1].sizeimage;
			if (sizes[0] < dev->bytesperline_out[0] * h ||
			    sizes[1] < dev->bytesperline_out[1] * h)
				return -EINVAL;
		} else if (sizes[0] < size) {
			return -EINVAL;
		}
	} else {
		if (planes == 2) {
			sizes[0] = dev->bytesperline_out[0] * h;
			sizes[1] = dev->bytesperline_out[1] * h;
		} else {
			sizes[0] = size;
		}
	}

	if (vq->num_buffers + *nbuffers < 2)
		*nbuffers = 2 - vq->num_buffers;

	*nplanes = planes;

	/*
	 * videobuf2-vmalloc allocator is context-less so no need to set
	 * alloc_ctxs array.
	 */

	if (planes == 2)
		dprintk(dev, 1, "%s, count=%d, sizes=%u, %u\n", __func__,
			*nbuffers, sizes[0], sizes[1]);
	else
		dprintk(dev, 1, "%s, count=%d, size=%u\n", __func__,
			*nbuffers, sizes[0]);
	return 0;
}

static int vid_out_buf_prepare(struct vb2_buffer *vb)
{
	struct vivid_dev *dev = vb2_get_drv_priv(vb->vb2_queue);
	unsigned long size;
	unsigned planes = dev->fmt_out->planes;
	unsigned p;

	dprintk(dev, 1, "%s\n", __func__);

	if (WARN_ON(NULL == dev->fmt_out))
		return -EINVAL;

	if (dev->buf_prepare_error) {
		/*
		 * Error injection: test what happens if buf_prepare() returns
		 * an error.
		 */
		dev->buf_prepare_error = false;
		return -EINVAL;
	}

	if (dev->field_out != V4L2_FIELD_ALTERNATE)
		vb->v4l2_buf.field = dev->field_out;
	else if (vb->v4l2_buf.field != V4L2_FIELD_TOP &&
		 vb->v4l2_buf.field != V4L2_FIELD_BOTTOM)
		return -EINVAL;

	for (p = 0; p < planes; p++) {
		size = dev->bytesperline_out[p] * dev->fmt_out_rect.height +
			vb->v4l2_planes[p].data_offset;

		if (vb2_get_plane_payload(vb, p) < size) {
			dprintk(dev, 1, "%s the payload is too small for plane %u (%lu < %lu)\n",
					__func__, p, vb2_get_plane_payload(vb, p), size);
			return -EINVAL;
		}
	}

	return 0;
}

static void vid_out_buf_queue(struct vb2_buffer *vb)
{
	struct vivid_dev *dev = vb2_get_drv_priv(vb->vb2_queue);
	struct vivid_buffer *buf = container_of(vb, struct vivid_buffer, vb);

	dprintk(dev, 1, "%s\n", __func__);

	spin_lock(&dev->slock);
	list_add_tail(&buf->list, &dev->vid_out_active);
	spin_unlock(&dev->slock);
}

static int vid_out_start_streaming(struct vb2_queue *vq, unsigned count)
{
	struct vivid_dev *dev = vb2_get_drv_priv(vq);
	int err;

	if (vb2_is_streaming(&dev->vb_vid_cap_q))
		dev->can_loop_video = vivid_vid_can_loop(dev);

	if (dev->kthread_vid_out)
		return 0;

	dev->vid_out_seq_count = 0;
	dprintk(dev, 1, "%s\n", __func__);
	if (dev->start_streaming_error) {
		dev->start_streaming_error = false;
		err = -EINVAL;
	} else {
		err = vivid_start_generating_vid_out(dev, &dev->vid_out_streaming);
	}
	if (err) {
		struct vivid_buffer *buf, *tmp;

		list_for_each_entry_safe(buf, tmp, &dev->vid_out_active, list) {
			list_del(&buf->list);
			vb2_buffer_done(&buf->vb, VB2_BUF_STATE_QUEUED);
		}
	}
	return err;
}

/* abort streaming and wait for last buffer */
static void vid_out_stop_streaming(struct vb2_queue *vq)
{
	struct vivid_dev *dev = vb2_get_drv_priv(vq);

	dprintk(dev, 1, "%s\n", __func__);
	vivid_stop_generating_vid_out(dev, &dev->vid_out_streaming);
	dev->can_loop_video = false;
}

const struct vb2_ops vivid_vid_out_qops = {
	.queue_setup		= vid_out_queue_setup,
	.buf_prepare		= vid_out_buf_prepare,
	.buf_queue		= vid_out_buf_queue,
	.start_streaming	= vid_out_start_streaming,
	.stop_streaming		= vid_out_stop_streaming,
	.wait_prepare		= vivid_unlock,
	.wait_finish		= vivid_lock,
};

/*
 * Called whenever the format has to be reset which can occur when
 * changing outputs, standard, timings, etc.
 */
void vivid_update_format_out(struct vivid_dev *dev)
{
	struct v4l2_bt_timings *bt = &dev->dv_timings_out.bt;
	unsigned size;

	switch (dev->output_type[dev->output]) {
	case SVID:
	default:
		dev->field_out = dev->tv_field_out;
		dev->sink_rect.width = 720;
		if (dev->std_out & V4L2_STD_525_60) {
			dev->sink_rect.height = 480;
			dev->timeperframe_vid_out = (struct v4l2_fract) { 1001, 30000 };
			dev->service_set_out = V4L2_SLICED_CAPTION_525;
		} else {
			dev->sink_rect.height = 576;
			dev->timeperframe_vid_out = (struct v4l2_fract) { 1000, 25000 };
			dev->service_set_out = V4L2_SLICED_WSS_625 | V4L2_SLICED_TELETEXT_B;
		}
		dev->colorspace_out = V4L2_COLORSPACE_SMPTE170M;
		break;
	case HDMI:
		dev->sink_rect.width = bt->width;
		dev->sink_rect.height = bt->height;
		size = V4L2_DV_BT_FRAME_WIDTH(bt) * V4L2_DV_BT_FRAME_HEIGHT(bt);
		dev->timeperframe_vid_out = (struct v4l2_fract) {
			size / 100, (u32)bt->pixelclock / 100
		};
		if (bt->interlaced)
			dev->field_out = V4L2_FIELD_ALTERNATE;
		else
			dev->field_out = V4L2_FIELD_NONE;
		if (!dev->dvi_d_out && (bt->standards & V4L2_DV_BT_STD_CEA861)) {
			if (bt->width == 720 && bt->height <= 576)
				dev->colorspace_out = V4L2_COLORSPACE_SMPTE170M;
			else
				dev->colorspace_out = V4L2_COLORSPACE_REC709;
		} else {
			dev->colorspace_out = V4L2_COLORSPACE_SRGB;
		}
		break;
	}
	dev->compose_out = dev->sink_rect;
	dev->compose_bounds_out = dev->sink_rect;
	dev->crop_out = dev->compose_out;
	if (V4L2_FIELD_HAS_T_OR_B(dev->field_out))
		dev->crop_out.height /= 2;
	dev->fmt_out_rect = dev->crop_out;
	dev->bytesperline_out[0] = (dev->sink_rect.width * dev->fmt_out->depth) / 8;
	if (dev->fmt_out->planes == 2)
		dev->bytesperline_out[1] = (dev->sink_rect.width * dev->fmt_out->depth) / 8;
}

/* Map the field to something that is valid for the current output */
static enum v4l2_field vivid_field_out(struct vivid_dev *dev, enum v4l2_field field)
{
	if (vivid_is_svid_out(dev)) {
		switch (field) {
		case V4L2_FIELD_INTERLACED_TB:
		case V4L2_FIELD_INTERLACED_BT:
		case V4L2_FIELD_SEQ_TB:
		case V4L2_FIELD_SEQ_BT:
		case V4L2_FIELD_ALTERNATE:
			return field;
		case V4L2_FIELD_INTERLACED:
		default:
			return V4L2_FIELD_INTERLACED;
		}
	}
	if (vivid_is_hdmi_out(dev))
		return dev->dv_timings_out.bt.interlaced ? V4L2_FIELD_ALTERNATE :
						       V4L2_FIELD_NONE;
	return V4L2_FIELD_NONE;
}

static enum tpg_pixel_aspect vivid_get_pixel_aspect(const struct vivid_dev *dev)
{
	if (vivid_is_svid_out(dev))
		return (dev->std_out & V4L2_STD_525_60) ?
			TPG_PIXEL_ASPECT_NTSC : TPG_PIXEL_ASPECT_PAL;

	if (vivid_is_hdmi_out(dev) &&
	    dev->sink_rect.width == 720 && dev->sink_rect.height <= 576)
		return dev->sink_rect.height == 480 ?
			TPG_PIXEL_ASPECT_NTSC : TPG_PIXEL_ASPECT_PAL;

	return TPG_PIXEL_ASPECT_SQUARE;
}

int vivid_g_fmt_vid_out(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);
	struct v4l2_pix_format_mplane *mp = &f->fmt.pix_mp;
	unsigned p;

	mp->width        = dev->fmt_out_rect.width;
	mp->height       = dev->fmt_out_rect.height;
	mp->field        = dev->field_out;
	mp->pixelformat  = dev->fmt_out->fourcc;
	mp->colorspace   = dev->colorspace_out;
	mp->num_planes = dev->fmt_out->planes;
	for (p = 0; p < mp->num_planes; p++) {
		mp->plane_fmt[p].bytesperline = dev->bytesperline_out[p];
		mp->plane_fmt[p].sizeimage =
			mp->plane_fmt[p].bytesperline * mp->height;
	}
	return 0;
}

int vivid_try_fmt_vid_out(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);
	struct v4l2_bt_timings *bt = &dev->dv_timings_out.bt;
	struct v4l2_pix_format_mplane *mp = &f->fmt.pix_mp;
	struct v4l2_plane_pix_format *pfmt = mp->plane_fmt;
	const struct vivid_fmt *fmt;
	unsigned bytesperline, max_bpl;
	unsigned factor = 1;
	unsigned w, h;
	unsigned p;

	fmt = vivid_get_format(dev, mp->pixelformat);
	if (!fmt) {
		dprintk(dev, 1, "Fourcc format (0x%08x) unknown.\n",
			mp->pixelformat);
		mp->pixelformat = V4L2_PIX_FMT_YUYV;
		fmt = vivid_get_format(dev, mp->pixelformat);
	}

	mp->field = vivid_field_out(dev, mp->field);
	if (vivid_is_svid_out(dev)) {
		w = 720;
		h = (dev->std_out & V4L2_STD_525_60) ? 480 : 576;
	} else {
		w = dev->sink_rect.width;
		h = dev->sink_rect.height;
	}
	if (V4L2_FIELD_HAS_T_OR_B(mp->field))
		factor = 2;
	if (!dev->has_scaler_out && !dev->has_crop_out && !dev->has_compose_out) {
		mp->width = w;
		mp->height = h / factor;
	} else {
		struct v4l2_rect r = { 0, 0, mp->width, mp->height * factor };

		rect_set_min_size(&r, &vivid_min_rect);
		rect_set_max_size(&r, &vivid_max_rect);
		if (dev->has_scaler_out && !dev->has_crop_out) {
			struct v4l2_rect max_r = { 0, 0, MAX_ZOOM * w, MAX_ZOOM * h };

			rect_set_max_size(&r, &max_r);
		} else if (!dev->has_scaler_out && dev->has_compose_out && !dev->has_crop_out) {
			rect_set_max_size(&r, &dev->sink_rect);
		} else if (!dev->has_scaler_out && !dev->has_compose_out) {
			rect_set_min_size(&r, &dev->sink_rect);
		}
		mp->width = r.width;
		mp->height = r.height / factor;
	}

	/* This driver supports custom bytesperline values */

	/* Calculate the minimum supported bytesperline value */
	bytesperline = (mp->width * fmt->depth) >> 3;
	/* Calculate the maximum supported bytesperline value */
	max_bpl = (MAX_ZOOM * MAX_WIDTH * fmt->depth) >> 3;
	mp->num_planes = fmt->planes;
	for (p = 0; p < mp->num_planes; p++) {
		if (pfmt[p].bytesperline > max_bpl)
			pfmt[p].bytesperline = max_bpl;
		if (pfmt[p].bytesperline < bytesperline)
			pfmt[p].bytesperline = bytesperline;
		pfmt[p].sizeimage = pfmt[p].bytesperline * mp->height;
		memset(pfmt[p].reserved, 0, sizeof(pfmt[p].reserved));
	}
	if (vivid_is_svid_out(dev))
		mp->colorspace = V4L2_COLORSPACE_SMPTE170M;
	else if (dev->dvi_d_out || !(bt->standards & V4L2_DV_BT_STD_CEA861))
		mp->colorspace = V4L2_COLORSPACE_SRGB;
	else if (bt->width == 720 && bt->height <= 576)
		mp->colorspace = V4L2_COLORSPACE_SMPTE170M;
	else if (mp->colorspace != V4L2_COLORSPACE_SMPTE170M &&
		 mp->colorspace != V4L2_COLORSPACE_REC709 &&
		 mp->colorspace != V4L2_COLORSPACE_SRGB)
		mp->colorspace = V4L2_COLORSPACE_REC709;
	memset(mp->reserved, 0, sizeof(mp->reserved));
	return 0;
}

int vivid_s_fmt_vid_out(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct v4l2_pix_format_mplane *mp = &f->fmt.pix_mp;
	struct vivid_dev *dev = video_drvdata(file);
	struct v4l2_rect *crop = &dev->crop_out;
	struct v4l2_rect *compose = &dev->compose_out;
	struct vb2_queue *q = &dev->vb_vid_out_q;
	int ret = vivid_try_fmt_vid_out(file, priv, f);
	unsigned factor = 1;

	if (ret < 0)
		return ret;

	if (vb2_is_busy(q) &&
	    (vivid_is_svid_out(dev) ||
	     mp->width != dev->fmt_out_rect.width ||
	     mp->height != dev->fmt_out_rect.height ||
	     mp->pixelformat != dev->fmt_out->fourcc ||
	     mp->field != dev->field_out)) {
		dprintk(dev, 1, "%s device busy\n", __func__);
		return -EBUSY;
	}

	/*
	 * Allow for changing the colorspace on the fly. Useful for testing
	 * purposes, and it is something that HDMI transmitters are able
	 * to do.
	 */
	if (vb2_is_busy(q))
		goto set_colorspace;

	dev->fmt_out = vivid_get_format(dev, mp->pixelformat);
	if (V4L2_FIELD_HAS_T_OR_B(mp->field))
		factor = 2;

	if (dev->has_scaler_out || dev->has_crop_out || dev->has_compose_out) {
		struct v4l2_rect r = { 0, 0, mp->width, mp->height };

		if (dev->has_scaler_out) {
			if (dev->has_crop_out)
				rect_map_inside(crop, &r);
			else
				*crop = r;
			if (dev->has_compose_out && !dev->has_crop_out) {
				struct v4l2_rect min_r = {
					0, 0,
					r.width / MAX_ZOOM,
					factor * r.height / MAX_ZOOM
				};
				struct v4l2_rect max_r = {
					0, 0,
					r.width * MAX_ZOOM,
					factor * r.height * MAX_ZOOM
				};

				rect_set_min_size(compose, &min_r);
				rect_set_max_size(compose, &max_r);
				rect_map_inside(compose, &dev->compose_bounds_out);
			} else if (dev->has_compose_out) {
				struct v4l2_rect min_r = {
					0, 0,
					crop->width / MAX_ZOOM,
					factor * crop->height / MAX_ZOOM
				};
				struct v4l2_rect max_r = {
					0, 0,
					crop->width * MAX_ZOOM,
					factor * crop->height * MAX_ZOOM
				};

				rect_set_min_size(compose, &min_r);
				rect_set_max_size(compose, &max_r);
				rect_map_inside(compose, &dev->compose_bounds_out);
			}
		} else if (dev->has_compose_out && !dev->has_crop_out) {
			rect_set_size_to(crop, &r);
			r.height *= factor;
			rect_set_size_to(compose, &r);
			rect_map_inside(compose, &dev->compose_bounds_out);
		} else if (!dev->has_compose_out) {
			rect_map_inside(crop, &r);
			r.height /= factor;
			rect_set_size_to(compose, &r);
		} else {
			r.height *= factor;
			rect_set_max_size(compose, &r);
			rect_map_inside(compose, &dev->compose_bounds_out);
			crop->top *= factor;
			crop->height *= factor;
			rect_set_size_to(crop, compose);
			rect_map_inside(crop, &r);
			crop->top /= factor;
			crop->height /= factor;
		}
	} else {
		struct v4l2_rect r = { 0, 0, mp->width, mp->height };

		rect_set_size_to(crop, &r);
		r.height /= factor;
		rect_set_size_to(compose, &r);
	}

	dev->fmt_out_rect.width = mp->width;
	dev->fmt_out_rect.height = mp->height;
	dev->bytesperline_out[0] = mp->plane_fmt[0].bytesperline;
	if (mp->num_planes > 1)
		dev->bytesperline_out[1] = mp->plane_fmt[1].bytesperline;
	dev->field_out = mp->field;
	if (vivid_is_svid_out(dev))
		dev->tv_field_out = mp->field;

set_colorspace:
	dev->colorspace_out = mp->colorspace;
	if (dev->loop_video) {
		vivid_send_source_change(dev, SVID);
		vivid_send_source_change(dev, HDMI);
	}
	return 0;
}

int vidioc_g_fmt_vid_out_mplane(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!dev->multiplanar)
		return -ENOTTY;
	return vivid_g_fmt_vid_out(file, priv, f);
}

int vidioc_try_fmt_vid_out_mplane(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!dev->multiplanar)
		return -ENOTTY;
	return vivid_try_fmt_vid_out(file, priv, f);
}

int vidioc_s_fmt_vid_out_mplane(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!dev->multiplanar)
		return -ENOTTY;
	return vivid_s_fmt_vid_out(file, priv, f);
}

int vidioc_g_fmt_vid_out(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (dev->multiplanar)
		return -ENOTTY;
	return fmt_sp2mp_func(file, priv, f, vivid_g_fmt_vid_out);
}

int vidioc_try_fmt_vid_out(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (dev->multiplanar)
		return -ENOTTY;
	return fmt_sp2mp_func(file, priv, f, vivid_try_fmt_vid_out);
}

int vidioc_s_fmt_vid_out(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (dev->multiplanar)
		return -ENOTTY;
	return fmt_sp2mp_func(file, priv, f, vivid_s_fmt_vid_out);
}

int vivid_vid_out_g_selection(struct file *file, void *priv,
			      struct v4l2_selection *sel)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!dev->has_crop_out && !dev->has_compose_out)
		return -ENOTTY;
	if (sel->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	sel->r.left = sel->r.top = 0;
	switch (sel->target) {
	case V4L2_SEL_TGT_CROP:
		if (!dev->has_crop_out)
			return -EINVAL;
		sel->r = dev->crop_out;
		break;
	case V4L2_SEL_TGT_CROP_DEFAULT:
		if (!dev->has_crop_out)
			return -EINVAL;
		sel->r = dev->fmt_out_rect;
		break;
	case V4L2_SEL_TGT_CROP_BOUNDS:
		if (!dev->has_crop_out)
			return -EINVAL;
		sel->r = vivid_max_rect;
		break;
	case V4L2_SEL_TGT_COMPOSE:
		if (!dev->has_compose_out)
			return -EINVAL;
		sel->r = dev->compose_out;
		break;
	case V4L2_SEL_TGT_COMPOSE_DEFAULT:
	case V4L2_SEL_TGT_COMPOSE_BOUNDS:
		if (!dev->has_compose_out)
			return -EINVAL;
		sel->r = dev->sink_rect;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int vivid_vid_out_s_selection(struct file *file, void *fh, struct v4l2_selection *s)
{
	struct vivid_dev *dev = video_drvdata(file);
	struct v4l2_rect *crop = &dev->crop_out;
	struct v4l2_rect *compose = &dev->compose_out;
	unsigned factor = V4L2_FIELD_HAS_T_OR_B(dev->field_out) ? 2 : 1;
	int ret;

	if (!dev->has_crop_out && !dev->has_compose_out)
		return -ENOTTY;
	if (s->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	switch (s->target) {
	case V4L2_SEL_TGT_CROP:
		if (!dev->has_crop_out)
			return -EINVAL;
		ret = vivid_vid_adjust_sel(s->flags, &s->r);
		if (ret)
			return ret;
		rect_set_min_size(&s->r, &vivid_min_rect);
		rect_set_max_size(&s->r, &dev->fmt_out_rect);
		if (dev->has_scaler_out) {
			struct v4l2_rect max_rect = {
				0, 0,
				dev->sink_rect.width * MAX_ZOOM,
				(dev->sink_rect.height / factor) * MAX_ZOOM
			};

			rect_set_max_size(&s->r, &max_rect);
			if (dev->has_compose_out) {
				struct v4l2_rect min_rect = {
					0, 0,
					s->r.width / MAX_ZOOM,
					(s->r.height * factor) / MAX_ZOOM
				};
				struct v4l2_rect max_rect = {
					0, 0,
					s->r.width * MAX_ZOOM,
					(s->r.height * factor) * MAX_ZOOM
				};

				rect_set_min_size(compose, &min_rect);
				rect_set_max_size(compose, &max_rect);
				rect_map_inside(compose, &dev->compose_bounds_out);
			}
		} else if (dev->has_compose_out) {
			s->r.top *= factor;
			s->r.height *= factor;
			rect_set_max_size(&s->r, &dev->sink_rect);
			rect_set_size_to(compose, &s->r);
			rect_map_inside(compose, &dev->compose_bounds_out);
			s->r.top /= factor;
			s->r.height /= factor;
		} else {
			rect_set_size_to(&s->r, &dev->sink_rect);
			s->r.height /= factor;
		}
		rect_map_inside(&s->r, &dev->fmt_out_rect);
		*crop = s->r;
		break;
	case V4L2_SEL_TGT_COMPOSE:
		if (!dev->has_compose_out)
			return -EINVAL;
		ret = vivid_vid_adjust_sel(s->flags, &s->r);
		if (ret)
			return ret;
		rect_set_min_size(&s->r, &vivid_min_rect);
		rect_set_max_size(&s->r, &dev->sink_rect);
		rect_map_inside(&s->r, &dev->compose_bounds_out);
		s->r.top /= factor;
		s->r.height /= factor;
		if (dev->has_scaler_out) {
			struct v4l2_rect fmt = dev->fmt_out_rect;
			struct v4l2_rect max_rect = {
				0, 0,
				s->r.width * MAX_ZOOM,
				s->r.height * MAX_ZOOM
			};
			struct v4l2_rect min_rect = {
				0, 0,
				s->r.width / MAX_ZOOM,
				s->r.height / MAX_ZOOM
			};

			rect_set_min_size(&fmt, &min_rect);
			if (!dev->has_crop_out)
				rect_set_max_size(&fmt, &max_rect);
			if (!rect_same_size(&dev->fmt_out_rect, &fmt) &&
			    vb2_is_busy(&dev->vb_vid_out_q))
				return -EBUSY;
			if (dev->has_crop_out) {
				rect_set_min_size(crop, &min_rect);
				rect_set_max_size(crop, &max_rect);
			}
			dev->fmt_out_rect = fmt;
		} else if (dev->has_crop_out) {
			struct v4l2_rect fmt = dev->fmt_out_rect;

			rect_set_min_size(&fmt, &s->r);
			if (!rect_same_size(&dev->fmt_out_rect, &fmt) &&
			    vb2_is_busy(&dev->vb_vid_out_q))
				return -EBUSY;
			dev->fmt_out_rect = fmt;
			rect_set_size_to(crop, &s->r);
			rect_map_inside(crop, &dev->fmt_out_rect);
		} else {
			if (!rect_same_size(&s->r, &dev->fmt_out_rect) &&
			    vb2_is_busy(&dev->vb_vid_out_q))
				return -EBUSY;
			rect_set_size_to(&dev->fmt_out_rect, &s->r);
			rect_set_size_to(crop, &s->r);
			crop->height /= factor;
			rect_map_inside(crop, &dev->fmt_out_rect);
		}
		s->r.top *= factor;
		s->r.height *= factor;
		if (dev->bitmap_out && (compose->width != s->r.width ||
					compose->height != s->r.height)) {
			kfree(dev->bitmap_out);
			dev->bitmap_out = NULL;
		}
		*compose = s->r;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int vivid_vid_out_cropcap(struct file *file, void *priv,
			      struct v4l2_cropcap *cap)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (cap->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	switch (vivid_get_pixel_aspect(dev)) {
	case TPG_PIXEL_ASPECT_NTSC:
		cap->pixelaspect.numerator = 11;
		cap->pixelaspect.denominator = 10;
		break;
	case TPG_PIXEL_ASPECT_PAL:
		cap->pixelaspect.numerator = 54;
		cap->pixelaspect.denominator = 59;
		break;
	case TPG_PIXEL_ASPECT_SQUARE:
		cap->pixelaspect.numerator = 1;
		cap->pixelaspect.denominator = 1;
		break;
	}
	return 0;
}

int vidioc_g_fmt_vid_out_overlay(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);
	const struct v4l2_rect *compose = &dev->compose_out;
	struct v4l2_window *win = &f->fmt.win;
	unsigned clipcount = win->clipcount;

	if (!dev->has_fb)
		return -EINVAL;
	win->w.top = dev->overlay_out_top;
	win->w.left = dev->overlay_out_left;
	win->w.width = compose->width;
	win->w.height = compose->height;
	win->clipcount = dev->clipcount_out;
	win->field = V4L2_FIELD_ANY;
	win->chromakey = dev->chromakey_out;
	win->global_alpha = dev->global_alpha_out;
	if (clipcount > dev->clipcount_out)
		clipcount = dev->clipcount_out;
	if (dev->bitmap_out == NULL)
		win->bitmap = NULL;
	else if (win->bitmap) {
		if (copy_to_user(win->bitmap, dev->bitmap_out,
		    ((dev->compose_out.width + 7) / 8) * dev->compose_out.height))
			return -EFAULT;
	}
	if (clipcount && win->clips) {
		if (copy_to_user(win->clips, dev->clips_out,
				 clipcount * sizeof(dev->clips_out[0])))
			return -EFAULT;
	}
	return 0;
}

int vidioc_try_fmt_vid_out_overlay(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);
	const struct v4l2_rect *compose = &dev->compose_out;
	struct v4l2_window *win = &f->fmt.win;
	int i, j;

	if (!dev->has_fb)
		return -EINVAL;
	win->w.left = clamp_t(int, win->w.left,
			      -dev->display_width, dev->display_width);
	win->w.top = clamp_t(int, win->w.top,
			     -dev->display_height, dev->display_height);
	win->w.width = compose->width;
	win->w.height = compose->height;
	/*
	 * It makes no sense for an OSD to overlay only top or bottom fields,
	 * so always set this to ANY.
	 */
	win->field = V4L2_FIELD_ANY;
	if (win->clipcount && !win->clips)
		win->clipcount = 0;
	if (win->clipcount > MAX_CLIPS)
		win->clipcount = MAX_CLIPS;
	if (win->clipcount) {
		if (copy_from_user(dev->try_clips_out, win->clips,
				   win->clipcount * sizeof(dev->clips_out[0])))
			return -EFAULT;
		for (i = 0; i < win->clipcount; i++) {
			struct v4l2_rect *r = &dev->try_clips_out[i].c;

			r->top = clamp_t(s32, r->top, 0, dev->display_height - 1);
			r->height = clamp_t(s32, r->height, 1, dev->display_height - r->top);
			r->left = clamp_t(u32, r->left, 0, dev->display_width - 1);
			r->width = clamp_t(u32, r->width, 1, dev->display_width - r->left);
		}
		/*
		 * Yeah, so sue me, it's an O(n^2) algorithm. But n is a small
		 * number and it's typically a one-time deal.
		 */
		for (i = 0; i < win->clipcount - 1; i++) {
			struct v4l2_rect *r1 = &dev->try_clips_out[i].c;

			for (j = i + 1; j < win->clipcount; j++) {
				struct v4l2_rect *r2 = &dev->try_clips_out[j].c;

				if (rect_overlap(r1, r2))
					return -EINVAL;
			}
		}
		if (copy_to_user(win->clips, dev->try_clips_out,
				 win->clipcount * sizeof(dev->clips_out[0])))
			return -EFAULT;
	}
	return 0;
}

int vidioc_s_fmt_vid_out_overlay(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vivid_dev *dev = video_drvdata(file);
	const struct v4l2_rect *compose = &dev->compose_out;
	struct v4l2_window *win = &f->fmt.win;
	int ret = vidioc_try_fmt_vid_out_overlay(file, priv, f);
	unsigned bitmap_size = ((compose->width + 7) / 8) * compose->height;
	unsigned clips_size = win->clipcount * sizeof(dev->clips_out[0]);
	void *new_bitmap = NULL;

	if (ret)
		return ret;

	if (win->bitmap) {
		new_bitmap = memdup_user(win->bitmap, bitmap_size);

		if (IS_ERR(new_bitmap))
			return PTR_ERR(new_bitmap);
	}

	dev->overlay_out_top = win->w.top;
	dev->overlay_out_left = win->w.left;
	kfree(dev->bitmap_out);
	dev->bitmap_out = new_bitmap;
	dev->clipcount_out = win->clipcount;
	if (dev->clipcount_out)
		memcpy(dev->clips_out, dev->try_clips_out, clips_size);
	dev->chromakey_out = win->chromakey;
	dev->global_alpha_out = win->global_alpha;
	return ret;
}

int vivid_vid_out_overlay(struct file *file, void *fh, unsigned i)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (i && !dev->fmt_out->can_do_overlay) {
		dprintk(dev, 1, "unsupported output format for output overlay\n");
		return -EINVAL;
	}

	dev->overlay_out_enabled = i;
	return 0;
}

int vivid_vid_out_g_fbuf(struct file *file, void *fh,
				struct v4l2_framebuffer *a)
{
	struct vivid_dev *dev = video_drvdata(file);

	a->capability = V4L2_FBUF_CAP_EXTERNOVERLAY |
			V4L2_FBUF_CAP_BITMAP_CLIPPING |
			V4L2_FBUF_CAP_LIST_CLIPPING |
			V4L2_FBUF_CAP_CHROMAKEY |
			V4L2_FBUF_CAP_SRC_CHROMAKEY |
			V4L2_FBUF_CAP_GLOBAL_ALPHA |
			V4L2_FBUF_CAP_LOCAL_ALPHA |
			V4L2_FBUF_CAP_LOCAL_INV_ALPHA;
	a->flags = V4L2_FBUF_FLAG_OVERLAY | dev->fbuf_out_flags;
	a->base = (void *)dev->video_pbase;
	a->fmt.width = dev->display_width;
	a->fmt.height = dev->display_height;
	if (dev->fb_defined.green.length == 5)
		a->fmt.pixelformat = V4L2_PIX_FMT_ARGB555;
	else
		a->fmt.pixelformat = V4L2_PIX_FMT_RGB565;
	a->fmt.bytesperline = dev->display_byte_stride;
	a->fmt.sizeimage = a->fmt.height * a->fmt.bytesperline;
	a->fmt.field = V4L2_FIELD_NONE;
	a->fmt.colorspace = V4L2_COLORSPACE_SRGB;
	a->fmt.priv = 0;
	return 0;
}

int vivid_vid_out_s_fbuf(struct file *file, void *fh,
				const struct v4l2_framebuffer *a)
{
	struct vivid_dev *dev = video_drvdata(file);
	const unsigned chroma_flags = V4L2_FBUF_FLAG_CHROMAKEY |
				      V4L2_FBUF_FLAG_SRC_CHROMAKEY;
	const unsigned alpha_flags = V4L2_FBUF_FLAG_GLOBAL_ALPHA |
				     V4L2_FBUF_FLAG_LOCAL_ALPHA |
				     V4L2_FBUF_FLAG_LOCAL_INV_ALPHA;


	if ((a->flags & chroma_flags) == chroma_flags)
		return -EINVAL;
	switch (a->flags & alpha_flags) {
	case 0:
	case V4L2_FBUF_FLAG_GLOBAL_ALPHA:
	case V4L2_FBUF_FLAG_LOCAL_ALPHA:
	case V4L2_FBUF_FLAG_LOCAL_INV_ALPHA:
		break;
	default:
		return -EINVAL;
	}
	dev->fbuf_out_flags &= ~(chroma_flags | alpha_flags);
	dev->fbuf_out_flags = a->flags & (chroma_flags | alpha_flags);
	return 0;
}

static const struct v4l2_audioout vivid_audio_outputs[] = {
	{ 0, "Line-Out 1" },
	{ 1, "Line-Out 2" },
};

int vidioc_enum_output(struct file *file, void *priv,
				struct v4l2_output *out)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (out->index >= dev->num_outputs)
		return -EINVAL;

	out->type = V4L2_OUTPUT_TYPE_ANALOG;
	switch (dev->output_type[out->index]) {
	case SVID:
		snprintf(out->name, sizeof(out->name), "S-Video %u",
				dev->output_name_counter[out->index]);
		out->std = V4L2_STD_ALL;
		if (dev->has_audio_outputs)
			out->audioset = (1 << ARRAY_SIZE(vivid_audio_outputs)) - 1;
		out->capabilities = V4L2_OUT_CAP_STD;
		break;
	case HDMI:
		snprintf(out->name, sizeof(out->name), "HDMI %u",
				dev->output_name_counter[out->index]);
		out->capabilities = V4L2_OUT_CAP_DV_TIMINGS;
		break;
	}
	return 0;
}

int vidioc_g_output(struct file *file, void *priv, unsigned *o)
{
	struct vivid_dev *dev = video_drvdata(file);

	*o = dev->output;
	return 0;
}

int vidioc_s_output(struct file *file, void *priv, unsigned o)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (o >= dev->num_outputs)
		return -EINVAL;

	if (o == dev->output)
		return 0;

	if (vb2_is_busy(&dev->vb_vid_out_q) || vb2_is_busy(&dev->vb_vbi_out_q))
		return -EBUSY;

	dev->output = o;
	dev->tv_audio_output = 0;
	if (dev->output_type[o] == SVID)
		dev->vid_out_dev.tvnorms = V4L2_STD_ALL;
	else
		dev->vid_out_dev.tvnorms = 0;

	dev->vbi_out_dev.tvnorms = dev->vid_out_dev.tvnorms;
	vivid_update_format_out(dev);
	return 0;
}

int vidioc_enumaudout(struct file *file, void *fh, struct v4l2_audioout *vout)
{
	if (vout->index >= ARRAY_SIZE(vivid_audio_outputs))
		return -EINVAL;
	*vout = vivid_audio_outputs[vout->index];
	return 0;
}

int vidioc_g_audout(struct file *file, void *fh, struct v4l2_audioout *vout)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_svid_out(dev))
		return -EINVAL;
	*vout = vivid_audio_outputs[dev->tv_audio_output];
	return 0;
}

int vidioc_s_audout(struct file *file, void *fh, const struct v4l2_audioout *vout)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_svid_out(dev))
		return -EINVAL;
	if (vout->index >= ARRAY_SIZE(vivid_audio_outputs))
		return -EINVAL;
	dev->tv_audio_output = vout->index;
	return 0;
}

int vivid_vid_out_s_std(struct file *file, void *priv, v4l2_std_id id)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_svid_out(dev))
		return -ENODATA;
	if (dev->std_out == id)
		return 0;
	if (vb2_is_busy(&dev->vb_vid_out_q) || vb2_is_busy(&dev->vb_vbi_out_q))
		return -EBUSY;
	dev->std_out = id;
	vivid_update_format_out(dev);
	return 0;
}

int vivid_vid_out_s_dv_timings(struct file *file, void *_fh,
				    struct v4l2_dv_timings *timings)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (!vivid_is_hdmi_out(dev))
		return -ENODATA;
	if (vb2_is_busy(&dev->vb_vid_out_q))
		return -EBUSY;
	if (!v4l2_find_dv_timings_cap(timings, &vivid_dv_timings_cap,
				0, NULL, NULL))
		return -EINVAL;
	if (v4l2_match_dv_timings(timings, &dev->dv_timings_out, 0))
		return 0;
	dev->dv_timings_out = *timings;
	vivid_update_format_out(dev);
	return 0;
}

int vivid_vid_out_g_parm(struct file *file, void *priv,
			  struct v4l2_streamparm *parm)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (parm->type != (dev->multiplanar ?
			   V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE :
			   V4L2_BUF_TYPE_VIDEO_OUTPUT))
		return -EINVAL;

	parm->parm.output.capability   = V4L2_CAP_TIMEPERFRAME;
	parm->parm.output.timeperframe = dev->timeperframe_vid_out;
	parm->parm.output.writebuffers  = 1;
return 0;
}

int vidioc_subscribe_event(struct v4l2_fh *fh,
			const struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_CTRL:
		return v4l2_ctrl_subscribe_event(fh, sub);
	case V4L2_EVENT_SOURCE_CHANGE:
		if (fh->vdev->vfl_dir == VFL_DIR_RX)
			return v4l2_src_change_event_subscribe(fh, sub);
		break;
	default:
		break;
	}
	return -EINVAL;
}
