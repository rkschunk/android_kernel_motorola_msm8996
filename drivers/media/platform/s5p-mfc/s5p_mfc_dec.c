/*
 * linux/drivers/media/platform/s5p-mfc/s5p_mfc_dec.c
 *
 * Copyright (C) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 * Kamil Debski, <k.debski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/videodev2.h>
#include <linux/workqueue.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-event.h>
#include <media/videobuf2-core.h>
#include "s5p_mfc_common.h"
#include "s5p_mfc_ctrl.h"
#include "s5p_mfc_debug.h"
#include "s5p_mfc_dec.h"
#include "s5p_mfc_intr.h"
#include "s5p_mfc_opr.h"
#include "s5p_mfc_pm.h"

static struct s5p_mfc_fmt formats[] = {
	{
		.name		= "4:2:0 2 Planes 16x16 Tiles",
		.fourcc		= V4L2_PIX_FMT_NV12MT_16X16,
		.codec_mode	= S5P_MFC_CODEC_NONE,
		.type		= MFC_FMT_RAW,
		.num_planes	= 2,
		.versions	= MFC_V6_BIT | MFC_V7_BIT,
	},
	{
		.name		= "4:2:0 2 Planes 64x32 Tiles",
		.fourcc		= V4L2_PIX_FMT_NV12MT,
		.codec_mode	= S5P_MFC_CODEC_NONE,
		.type		= MFC_FMT_RAW,
		.num_planes	= 2,
		.versions	= MFC_V5_BIT,
	},
	{
		.name		= "4:2:0 2 Planes Y/CbCr",
		.fourcc		= V4L2_PIX_FMT_NV12M,
		.codec_mode	= S5P_MFC_CODEC_NONE,
		.type		= MFC_FMT_RAW,
		.num_planes	= 2,
		.versions	= MFC_V6_BIT | MFC_V7_BIT | MFC_V8_BIT,
	},
	{
		.name		= "4:2:0 2 Planes Y/CrCb",
		.fourcc		= V4L2_PIX_FMT_NV21M,
		.codec_mode	= S5P_MFC_CODEC_NONE,
		.type		= MFC_FMT_RAW,
		.num_planes	= 2,
		.versions	= MFC_V6_BIT | MFC_V7_BIT | MFC_V8_BIT,
	},
	{
		.name		= "H264 Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_H264,
		.codec_mode	= S5P_MFC_CODEC_H264_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V5_BIT | MFC_V6_BIT | MFC_V7_BIT |
								MFC_V8_BIT,
	},
	{
		.name		= "H264/MVC Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_H264_MVC,
		.codec_mode	= S5P_MFC_CODEC_H264_MVC_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V6_BIT | MFC_V7_BIT | MFC_V8_BIT,
	},
	{
		.name		= "H263 Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_H263,
		.codec_mode	= S5P_MFC_CODEC_H263_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V5_BIT | MFC_V6_BIT | MFC_V7_BIT |
								MFC_V8_BIT,
	},
	{
		.name		= "MPEG1 Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_MPEG1,
		.codec_mode	= S5P_MFC_CODEC_MPEG2_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V5_BIT | MFC_V6_BIT | MFC_V7_BIT |
								MFC_V8_BIT,
	},
	{
		.name		= "MPEG2 Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_MPEG2,
		.codec_mode	= S5P_MFC_CODEC_MPEG2_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V5_BIT | MFC_V6_BIT | MFC_V7_BIT |
								MFC_V8_BIT,
	},
	{
		.name		= "MPEG4 Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_MPEG4,
		.codec_mode	= S5P_MFC_CODEC_MPEG4_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V5_BIT | MFC_V6_BIT | MFC_V7_BIT |
								MFC_V8_BIT,
	},
	{
		.name		= "XviD Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_XVID,
		.codec_mode	= S5P_MFC_CODEC_MPEG4_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V5_BIT | MFC_V6_BIT | MFC_V7_BIT |
								MFC_V8_BIT,
	},
	{
		.name		= "VC1 Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_VC1_ANNEX_G,
		.codec_mode	= S5P_MFC_CODEC_VC1_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V5_BIT | MFC_V6_BIT | MFC_V7_BIT |
								MFC_V8_BIT,
	},
	{
		.name		= "VC1 RCV Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_VC1_ANNEX_L,
		.codec_mode	= S5P_MFC_CODEC_VC1RCV_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V5_BIT | MFC_V6_BIT | MFC_V7_BIT |
								MFC_V8_BIT,
	},
	{
		.name		= "VP8 Encoded Stream",
		.fourcc		= V4L2_PIX_FMT_VP8,
		.codec_mode	= S5P_MFC_CODEC_VP8_DEC,
		.type		= MFC_FMT_DEC,
		.num_planes	= 1,
		.versions	= MFC_V6_BIT | MFC_V7_BIT | MFC_V8_BIT,
	},
};

#define NUM_FORMATS ARRAY_SIZE(formats)

/* Find selected format description */
static struct s5p_mfc_fmt *find_format(struct v4l2_format *f, unsigned int t)
{
	unsigned int i;

	for (i = 0; i < NUM_FORMATS; i++) {
		if (formats[i].fourcc == f->fmt.pix_mp.pixelformat &&
		    formats[i].type == t)
			return &formats[i];
	}
	return NULL;
}

static struct mfc_control controls[] = {
	{
		.id = V4L2_CID_MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "H264 Display Delay",
		.minimum = 0,
		.maximum = 16383,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.name = "H264 Display Delay Enable",
		.minimum = 0,
		.maximum = 1,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_DECODER_MPEG4_DEBLOCK_FILTER,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.name = "Mpeg4 Loop Filter Enable",
		.minimum = 0,
		.maximum = 1,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_DECODER_SLICE_INTERFACE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.name = "Slice Interface Enable",
		.minimum = 0,
		.maximum = 1,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Minimum number of cap bufs",
		.minimum = 1,
		.maximum = 32,
		.step = 1,
		.default_value = 1,
		.is_volatile = 1,
	},
};

#define NUM_CTRLS ARRAY_SIZE(controls)

/* Check whether a context should be run on hardware */
static int s5p_mfc_ctx_ready(struct s5p_mfc_ctx *ctx)
{
	/* Context is to parse header */
	if (ctx->src_queue_cnt >= 1 && ctx->state == MFCINST_GOT_INST)
		return 1;
	/* Context is to decode a frame */
	if (ctx->src_queue_cnt >= 1 &&
	    ctx->state == MFCINST_RUNNING &&
	    ctx->dst_queue_cnt >= ctx->pb_count)
		return 1;
	/* Context is to return last frame */
	if (ctx->state == MFCINST_FINISHING &&
	    ctx->dst_queue_cnt >= ctx->pb_count)
		return 1;
	/* Context is to set buffers */
	if (ctx->src_queue_cnt >= 1 &&
	    ctx->state == MFCINST_HEAD_PARSED &&
	    ctx->capture_state == QUEUE_BUFS_MMAPED)
		return 1;
	/* Resolution change */
	if ((ctx->state == MFCINST_RES_CHANGE_INIT ||
		ctx->state == MFCINST_RES_CHANGE_FLUSH) &&
		ctx->dst_queue_cnt >= ctx->pb_count)
		return 1;
	if (ctx->state == MFCINST_RES_CHANGE_END &&
		ctx->src_queue_cnt >= 1)
		return 1;
	mfc_debug(2, "ctx is not ready\n");
	return 0;
}

static struct s5p_mfc_codec_ops decoder_codec_ops = {
	.pre_seq_start		= NULL,
	.post_seq_start		= NULL,
	.pre_frame_start	= NULL,
	.post_frame_start	= NULL,
};

/* Query capabilities of the device */
static int vidioc_querycap(struct file *file, void *priv,
			   struct v4l2_capability *cap)
{
	struct s5p_mfc_dev *dev = video_drvdata(file);

	strncpy(cap->driver, dev->plat_dev->name, sizeof(cap->driver) - 1);
	strncpy(cap->card, dev->plat_dev->name, sizeof(cap->card) - 1);
	cap->bus_info[0] = 0;
	cap->version = KERNEL_VERSION(1, 0, 0);
	/*
	 * This is only a mem-to-mem video device. The capture and output
	 * device capability flags are left only for backward compatibility
	 * and are scheduled for removal.
	 */
	cap->capabilities = V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING |
			    V4L2_CAP_VIDEO_CAPTURE_MPLANE |
			    V4L2_CAP_VIDEO_OUTPUT_MPLANE;
	return 0;
}

/* Enumerate format */
static int vidioc_enum_fmt(struct file *file, struct v4l2_fmtdesc *f,
							bool mplane, bool out)
{
	struct s5p_mfc_dev *dev = video_drvdata(file);
	struct s5p_mfc_fmt *fmt;
	int i, j = 0;

	for (i = 0; i < ARRAY_SIZE(formats); ++i) {
		if (mplane && formats[i].num_planes == 1)
			continue;
		else if (!mplane && formats[i].num_planes > 1)
			continue;
		if (out && formats[i].type != MFC_FMT_DEC)
			continue;
		else if (!out && formats[i].type != MFC_FMT_RAW)
			continue;
		else if ((dev->variant->version_bit & formats[i].versions) == 0)
			continue;

		if (j == f->index)
			break;
		++j;
	}
	if (i == ARRAY_SIZE(formats))
		return -EINVAL;
	fmt = &formats[i];
	strlcpy(f->description, fmt->name, sizeof(f->description));
	f->pixelformat = fmt->fourcc;
	return 0;
}

static int vidioc_enum_fmt_vid_cap(struct file *file, void *pirv,
							struct v4l2_fmtdesc *f)
{
	return vidioc_enum_fmt(file, f, false, false);
}

static int vidioc_enum_fmt_vid_cap_mplane(struct file *file, void *pirv,
							struct v4l2_fmtdesc *f)
{
	return vidioc_enum_fmt(file, f, true, false);
}

static int vidioc_enum_fmt_vid_out(struct file *file, void *priv,
							struct v4l2_fmtdesc *f)
{
	return vidioc_enum_fmt(file, f, false, true);
}

static int vidioc_enum_fmt_vid_out_mplane(struct file *file, void *priv,
							struct v4l2_fmtdesc *f)
{
	return vidioc_enum_fmt(file, f, true, true);
}

/* Get format */
static int vidioc_g_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);
	struct v4l2_pix_format_mplane *pix_mp;

	mfc_debug_enter();
	pix_mp = &f->fmt.pix_mp;
	if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE &&
	    (ctx->state == MFCINST_GOT_INST || ctx->state ==
						MFCINST_RES_CHANGE_END)) {
		/* If the MFC is parsing the header,
		 * so wait until it is finished */
		s5p_mfc_clean_ctx_int_flags(ctx);
		s5p_mfc_wait_for_done_ctx(ctx, S5P_MFC_R2H_CMD_SEQ_DONE_RET,
									0);
	}
	if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE &&
	    ctx->state >= MFCINST_HEAD_PARSED &&
	    ctx->state < MFCINST_ABORT) {
		/* This is run on CAPTURE (decode output) */
		/* Width and height are set to the dimensions
		   of the movie, the buffer is bigger and
		   further processing stages should crop to this
		   rectangle. */
		pix_mp->width = ctx->buf_width;
		pix_mp->height = ctx->buf_height;
		pix_mp->field = V4L2_FIELD_NONE;
		pix_mp->num_planes = 2;
		/* Set pixelformat to the format in which MFC
		   outputs the decoded frame */
		pix_mp->pixelformat = ctx->dst_fmt->fourcc;
		pix_mp->plane_fmt[0].bytesperline = ctx->buf_width;
		pix_mp->plane_fmt[0].sizeimage = ctx->luma_size;
		pix_mp->plane_fmt[1].bytesperline = ctx->buf_width;
		pix_mp->plane_fmt[1].sizeimage = ctx->chroma_size;
	} else if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		/* This is run on OUTPUT
		   The buffer contains compressed image
		   so width and height have no meaning */
		pix_mp->width = 0;
		pix_mp->height = 0;
		pix_mp->field = V4L2_FIELD_NONE;
		pix_mp->plane_fmt[0].bytesperline = ctx->dec_src_buf_size;
		pix_mp->plane_fmt[0].sizeimage = ctx->dec_src_buf_size;
		pix_mp->pixelformat = ctx->src_fmt->fourcc;
		pix_mp->num_planes = ctx->src_fmt->num_planes;
	} else {
		mfc_err("Format could not be read\n");
		mfc_debug(2, "%s-- with error\n", __func__);
		return -EINVAL;
	}
	mfc_debug_leave();
	return 0;
}

/* Try format */
static int vidioc_try_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
	struct s5p_mfc_dev *dev = video_drvdata(file);
	struct s5p_mfc_fmt *fmt;

	mfc_debug(2, "Type is %d\n", f->type);
	if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		fmt = find_format(f, MFC_FMT_DEC);
		if (!fmt) {
			mfc_err("Unsupported format for source.\n");
			return -EINVAL;
		}
		if (fmt->codec_mode == S5P_FIMV_CODEC_NONE) {
			mfc_err("Unknown codec\n");
			return -EINVAL;
		}
		if ((dev->variant->version_bit & fmt->versions) == 0) {
			mfc_err("Unsupported format by this MFC version.\n");
			return -EINVAL;
		}
	} else if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		fmt = find_format(f, MFC_FMT_RAW);
		if (!fmt) {
			mfc_err("Unsupported format for destination.\n");
			return -EINVAL;
		}
		if ((dev->variant->version_bit & fmt->versions) == 0) {
			mfc_err("Unsupported format by this MFC version.\n");
			return -EINVAL;
		}
	}

	return 0;
}

/* Set format */
static int vidioc_s_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
	struct s5p_mfc_dev *dev = video_drvdata(file);
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);
	int ret = 0;
	struct v4l2_pix_format_mplane *pix_mp;

	mfc_debug_enter();
	ret = vidioc_try_fmt(file, priv, f);
	pix_mp = &f->fmt.pix_mp;
	if (ret)
		return ret;
	if (ctx->vq_src.streaming || ctx->vq_dst.streaming) {
		v4l2_err(&dev->v4l2_dev, "%s queue busy\n", __func__);
		ret = -EBUSY;
		goto out;
	}
	if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		/* dst_fmt is validated by call to vidioc_try_fmt */
		ctx->dst_fmt = find_format(f, MFC_FMT_RAW);
		ret = 0;
		goto out;
	} else if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		/* src_fmt is validated by call to vidioc_try_fmt */
		ctx->src_fmt = find_format(f, MFC_FMT_DEC);
		ctx->codec_mode = ctx->src_fmt->codec_mode;
		mfc_debug(2, "The codec number is: %d\n", ctx->codec_mode);
		pix_mp->height = 0;
		pix_mp->width = 0;
		if (pix_mp->plane_fmt[0].sizeimage)
			ctx->dec_src_buf_size = pix_mp->plane_fmt[0].sizeimage;
		else
			pix_mp->plane_fmt[0].sizeimage = ctx->dec_src_buf_size =
								DEF_CPB_SIZE;
		pix_mp->plane_fmt[0].bytesperline = 0;
		ctx->state = MFCINST_INIT;
		ret = 0;
		goto out;
	} else {
		mfc_err("Wrong type error for S_FMT : %d", f->type);
		ret = -EINVAL;
		goto out;
	}

out:
	mfc_debug_leave();
	return ret;
}

static int reqbufs_output(struct s5p_mfc_dev *dev, struct s5p_mfc_ctx *ctx,
				struct v4l2_requestbuffers *reqbufs)
{
	int ret = 0;

	s5p_mfc_clock_on();

	if (reqbufs->count == 0) {
		mfc_debug(2, "Freeing buffers\n");
		ret = vb2_reqbufs(&ctx->vq_src, reqbufs);
		if (ret)
			goto out;
		s5p_mfc_close_mfc_inst(dev, ctx);
		ctx->src_bufs_cnt = 0;
		ctx->output_state = QUEUE_FREE;
	} else if (ctx->output_state == QUEUE_FREE) {
		/* Can only request buffers when we have a valid format set. */
		WARN_ON(ctx->src_bufs_cnt != 0);
		if (ctx->state != MFCINST_INIT) {
			mfc_err("Reqbufs called in an invalid state\n");
			ret = -EINVAL;
			goto out;
		}

		mfc_debug(2, "Allocating %d buffers for OUTPUT queue\n",
				reqbufs->count);
		ret = vb2_reqbufs(&ctx->vq_src, reqbufs);
		if (ret)
			goto out;

		ret = s5p_mfc_open_mfc_inst(dev, ctx);
		if (ret) {
			reqbufs->count = 0;
			vb2_reqbufs(&ctx->vq_src, reqbufs);
			goto out;
		}

		ctx->output_state = QUEUE_BUFS_REQUESTED;
	} else {
		mfc_err("Buffers have already been requested\n");
		ret = -EINVAL;
	}
out:
	s5p_mfc_clock_off();
	if (ret)
		mfc_err("Failed allocating buffers for OUTPUT queue\n");
	return ret;
}

static int reqbufs_capture(struct s5p_mfc_dev *dev, struct s5p_mfc_ctx *ctx,
				struct v4l2_requestbuffers *reqbufs)
{
	int ret = 0;

	s5p_mfc_clock_on();

	if (reqbufs->count == 0) {
		mfc_debug(2, "Freeing buffers\n");
		ret = vb2_reqbufs(&ctx->vq_dst, reqbufs);
		if (ret)
			goto out;
		s5p_mfc_hw_call(dev->mfc_ops, release_codec_buffers, ctx);
		ctx->dst_bufs_cnt = 0;
	} else if (ctx->capture_state == QUEUE_FREE) {
		WARN_ON(ctx->dst_bufs_cnt != 0);
		mfc_debug(2, "Allocating %d buffers for CAPTURE queue\n",
				reqbufs->count);
		ret = vb2_reqbufs(&ctx->vq_dst, reqbufs);
		if (ret)
			goto out;

		ctx->capture_state = QUEUE_BUFS_REQUESTED;
		ctx->total_dpb_count = reqbufs->count;

		ret = s5p_mfc_hw_call(dev->mfc_ops, alloc_codec_buffers, ctx);
		if (ret) {
			mfc_err("Failed to allocate decoding buffers\n");
			reqbufs->count = 0;
			vb2_reqbufs(&ctx->vq_dst, reqbufs);
			ret = -ENOMEM;
			ctx->capture_state = QUEUE_FREE;
			goto out;
		}

		WARN_ON(ctx->dst_bufs_cnt != ctx->total_dpb_count);
		ctx->capture_state = QUEUE_BUFS_MMAPED;

		if (s5p_mfc_ctx_ready(ctx))
			set_work_bit_irqsave(ctx);
		s5p_mfc_hw_call(dev->mfc_ops, try_run, dev);
		s5p_mfc_wait_for_done_ctx(ctx, S5P_MFC_R2H_CMD_INIT_BUFFERS_RET,
					  0);
	} else {
		mfc_err("Buffers have already been requested\n");
		ret = -EINVAL;
	}
out:
	s5p_mfc_clock_off();
	if (ret)
		mfc_err("Failed allocating buffers for CAPTURE queue\n");
	return ret;
}

/* Reqeust buffers */
static int vidioc_reqbufs(struct file *file, void *priv,
					  struct v4l2_requestbuffers *reqbufs)
{
	struct s5p_mfc_dev *dev = video_drvdata(file);
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);

	if (reqbufs->memory != V4L2_MEMORY_MMAP) {
		mfc_err("Only V4L2_MEMORY_MAP is supported\n");
		return -EINVAL;
	}

	if (reqbufs->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		return reqbufs_output(dev, ctx, reqbufs);
	} else if (reqbufs->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		return reqbufs_capture(dev, ctx, reqbufs);
	} else {
		mfc_err("Invalid type requested\n");
		return -EINVAL;
	}
}

/* Query buffer */
static int vidioc_querybuf(struct file *file, void *priv,
						   struct v4l2_buffer *buf)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);
	int ret;
	int i;

	if (buf->memory != V4L2_MEMORY_MMAP) {
		mfc_err("Only mmaped buffers can be used\n");
		return -EINVAL;
	}
	mfc_debug(2, "State: %d, buf->type: %d\n", ctx->state, buf->type);
	if (ctx->state == MFCINST_GOT_INST &&
			buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		ret = vb2_querybuf(&ctx->vq_src, buf);
	} else if (ctx->state == MFCINST_RUNNING &&
			buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		ret = vb2_querybuf(&ctx->vq_dst, buf);
		for (i = 0; i < buf->length; i++)
			buf->m.planes[i].m.mem_offset += DST_QUEUE_OFF_BASE;
	} else {
		mfc_err("vidioc_querybuf called in an inappropriate state\n");
		ret = -EINVAL;
	}
	mfc_debug_leave();
	return ret;
}

/* Queue a buffer */
static int vidioc_qbuf(struct file *file, void *priv, struct v4l2_buffer *buf)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);

	if (ctx->state == MFCINST_ERROR) {
		mfc_err("Call on QBUF after unrecoverable error\n");
		return -EIO;
	}
	if (buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		return vb2_qbuf(&ctx->vq_src, buf);
	else if (buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		return vb2_qbuf(&ctx->vq_dst, buf);
	return -EINVAL;
}

/* Dequeue a buffer */
static int vidioc_dqbuf(struct file *file, void *priv, struct v4l2_buffer *buf)
{
	const struct v4l2_event ev = {
		.type = V4L2_EVENT_EOS
	};
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);
	int ret;

	if (ctx->state == MFCINST_ERROR) {
		mfc_err("Call on DQBUF after unrecoverable error\n");
		return -EIO;
	}
	if (buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		ret = vb2_dqbuf(&ctx->vq_src, buf, file->f_flags & O_NONBLOCK);
	else if (buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		ret = vb2_dqbuf(&ctx->vq_dst, buf, file->f_flags & O_NONBLOCK);
		if (ret == 0 && ctx->state == MFCINST_FINISHED &&
				list_empty(&ctx->vq_dst.done_list))
			v4l2_event_queue_fh(&ctx->fh, &ev);
	} else {
		ret = -EINVAL;
	}
	return ret;
}

/* Export DMA buffer */
static int vidioc_expbuf(struct file *file, void *priv,
	struct v4l2_exportbuffer *eb)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);

	if (eb->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		return vb2_expbuf(&ctx->vq_src, eb);
	if (eb->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		return vb2_expbuf(&ctx->vq_dst, eb);
	return -EINVAL;
}

/* Stream on */
static int vidioc_streamon(struct file *file, void *priv,
			   enum v4l2_buf_type type)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);
	int ret = -EINVAL;

	mfc_debug_enter();
	if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		ret = vb2_streamon(&ctx->vq_src, type);
	else if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		ret = vb2_streamon(&ctx->vq_dst, type);
	mfc_debug_leave();
	return ret;
}

/* Stream off, which equals to a pause */
static int vidioc_streamoff(struct file *file, void *priv,
			    enum v4l2_buf_type type)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);

	if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		return vb2_streamoff(&ctx->vq_src, type);
	else if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		return vb2_streamoff(&ctx->vq_dst, type);
	return -EINVAL;
}

/* Set controls - v4l2 control framework */
static int s5p_mfc_dec_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct s5p_mfc_ctx *ctx = ctrl_to_ctx(ctrl);

	switch (ctrl->id) {
	case V4L2_CID_MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY:
		ctx->display_delay = ctrl->val;
		break;
	case V4L2_CID_MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY_ENABLE:
		ctx->display_delay_enable = ctrl->val;
		break;
	case V4L2_CID_MPEG_VIDEO_DECODER_MPEG4_DEBLOCK_FILTER:
		ctx->loop_filter_mpeg4 = ctrl->val;
		break;
	case V4L2_CID_MPEG_VIDEO_DECODER_SLICE_INTERFACE:
		ctx->slice_interface = ctrl->val;
		break;
	default:
		mfc_err("Invalid control 0x%08x\n", ctrl->id);
		return -EINVAL;
	}
	return 0;
}

static int s5p_mfc_dec_g_v_ctrl(struct v4l2_ctrl *ctrl)
{
	struct s5p_mfc_ctx *ctx = ctrl_to_ctx(ctrl);
	struct s5p_mfc_dev *dev = ctx->dev;

	switch (ctrl->id) {
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
		if (ctx->state >= MFCINST_HEAD_PARSED &&
		    ctx->state < MFCINST_ABORT) {
			ctrl->val = ctx->pb_count;
			break;
		} else if (ctx->state != MFCINST_INIT) {
			v4l2_err(&dev->v4l2_dev, "Decoding not initialised\n");
			return -EINVAL;
		}
		/* Should wait for the header to be parsed */
		s5p_mfc_clean_ctx_int_flags(ctx);
		s5p_mfc_wait_for_done_ctx(ctx,
				S5P_MFC_R2H_CMD_SEQ_DONE_RET, 0);
		if (ctx->state >= MFCINST_HEAD_PARSED &&
		    ctx->state < MFCINST_ABORT) {
			ctrl->val = ctx->pb_count;
		} else {
			v4l2_err(&dev->v4l2_dev, "Decoding not initialised\n");
			return -EINVAL;
		}
		break;
	}
	return 0;
}


static const struct v4l2_ctrl_ops s5p_mfc_dec_ctrl_ops = {
	.s_ctrl = s5p_mfc_dec_s_ctrl,
	.g_volatile_ctrl = s5p_mfc_dec_g_v_ctrl,
};

/* Get cropping information */
static int vidioc_g_crop(struct file *file, void *priv,
		struct v4l2_crop *cr)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);
	struct s5p_mfc_dev *dev = ctx->dev;
	u32 left, right, top, bottom;

	if (ctx->state != MFCINST_HEAD_PARSED &&
	ctx->state != MFCINST_RUNNING && ctx->state != MFCINST_FINISHING
					&& ctx->state != MFCINST_FINISHED) {
			mfc_err("Cannont set crop\n");
			return -EINVAL;
		}
	if (ctx->src_fmt->fourcc == V4L2_PIX_FMT_H264) {
		left = s5p_mfc_hw_call(dev->mfc_ops, get_crop_info_h, ctx);
		right = left >> S5P_FIMV_SHARED_CROP_RIGHT_SHIFT;
		left = left & S5P_FIMV_SHARED_CROP_LEFT_MASK;
		top = s5p_mfc_hw_call(dev->mfc_ops, get_crop_info_v, ctx);
		bottom = top >> S5P_FIMV_SHARED_CROP_BOTTOM_SHIFT;
		top = top & S5P_FIMV_SHARED_CROP_TOP_MASK;
		cr->c.left = left;
		cr->c.top = top;
		cr->c.width = ctx->img_width - left - right;
		cr->c.height = ctx->img_height - top - bottom;
		mfc_debug(2, "Cropping info [h264]: l=%d t=%d "
			"w=%d h=%d (r=%d b=%d fw=%d fh=%d\n", left, top,
			cr->c.width, cr->c.height, right, bottom,
			ctx->buf_width, ctx->buf_height);
	} else {
		cr->c.left = 0;
		cr->c.top = 0;
		cr->c.width = ctx->img_width;
		cr->c.height = ctx->img_height;
		mfc_debug(2, "Cropping info: w=%d h=%d fw=%d "
			"fh=%d\n", cr->c.width,	cr->c.height, ctx->buf_width,
							ctx->buf_height);
	}
	return 0;
}

int vidioc_decoder_cmd(struct file *file, void *priv,
						struct v4l2_decoder_cmd *cmd)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(priv);
	struct s5p_mfc_dev *dev = ctx->dev;
	struct s5p_mfc_buf *buf;
	unsigned long flags;

	switch (cmd->cmd) {
	case V4L2_ENC_CMD_STOP:
		if (cmd->flags != 0)
			return -EINVAL;

		if (!ctx->vq_src.streaming)
			return -EINVAL;

		spin_lock_irqsave(&dev->irqlock, flags);
		if (list_empty(&ctx->src_queue)) {
			mfc_err("EOS: empty src queue, entering finishing state");
			ctx->state = MFCINST_FINISHING;
			if (s5p_mfc_ctx_ready(ctx))
				set_work_bit_irqsave(ctx);
			spin_unlock_irqrestore(&dev->irqlock, flags);
			s5p_mfc_hw_call(dev->mfc_ops, try_run, dev);
		} else {
			mfc_err("EOS: marking last buffer of stream");
			buf = list_entry(ctx->src_queue.prev,
						struct s5p_mfc_buf, list);
			if (buf->flags & MFC_BUF_FLAG_USED)
				ctx->state = MFCINST_FINISHING;
			else
				buf->flags |= MFC_BUF_FLAG_EOS;
			spin_unlock_irqrestore(&dev->irqlock, flags);
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int vidioc_subscribe_event(struct v4l2_fh *fh,
				const struct  v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_EOS:
		return v4l2_event_subscribe(fh, sub, 2, NULL);
	case V4L2_EVENT_SOURCE_CHANGE:
		return v4l2_src_change_event_subscribe(fh, sub);
	default:
		return -EINVAL;
	}
}


/* v4l2_ioctl_ops */
static const struct v4l2_ioctl_ops s5p_mfc_dec_ioctl_ops = {
	.vidioc_querycap = vidioc_querycap,
	.vidioc_enum_fmt_vid_cap = vidioc_enum_fmt_vid_cap,
	.vidioc_enum_fmt_vid_cap_mplane = vidioc_enum_fmt_vid_cap_mplane,
	.vidioc_enum_fmt_vid_out = vidioc_enum_fmt_vid_out,
	.vidioc_enum_fmt_vid_out_mplane = vidioc_enum_fmt_vid_out_mplane,
	.vidioc_g_fmt_vid_cap_mplane = vidioc_g_fmt,
	.vidioc_g_fmt_vid_out_mplane = vidioc_g_fmt,
	.vidioc_try_fmt_vid_cap_mplane = vidioc_try_fmt,
	.vidioc_try_fmt_vid_out_mplane = vidioc_try_fmt,
	.vidioc_s_fmt_vid_cap_mplane = vidioc_s_fmt,
	.vidioc_s_fmt_vid_out_mplane = vidioc_s_fmt,
	.vidioc_reqbufs = vidioc_reqbufs,
	.vidioc_querybuf = vidioc_querybuf,
	.vidioc_qbuf = vidioc_qbuf,
	.vidioc_dqbuf = vidioc_dqbuf,
	.vidioc_expbuf = vidioc_expbuf,
	.vidioc_streamon = vidioc_streamon,
	.vidioc_streamoff = vidioc_streamoff,
	.vidioc_g_crop = vidioc_g_crop,
	.vidioc_decoder_cmd = vidioc_decoder_cmd,
	.vidioc_subscribe_event = vidioc_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
};

static int s5p_mfc_queue_setup(struct vb2_queue *vq,
			const struct v4l2_format *fmt, unsigned int *buf_count,
			unsigned int *plane_count, unsigned int psize[],
			void *allocators[])
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(vq->drv_priv);
	struct s5p_mfc_dev *dev = ctx->dev;

	/* Video output for decoding (source)
	 * this can be set after getting an instance */
	if (ctx->state == MFCINST_INIT &&
	    vq->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		/* A single plane is required for input */
		*plane_count = 1;
		if (*buf_count < 1)
			*buf_count = 1;
		if (*buf_count > MFC_MAX_BUFFERS)
			*buf_count = MFC_MAX_BUFFERS;
	/* Video capture for decoding (destination)
	 * this can be set after the header was parsed */
	} else if (ctx->state == MFCINST_HEAD_PARSED &&
		   vq->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		/* Output plane count is 2 - one for Y and one for CbCr */
		*plane_count = 2;
		/* Setup buffer count */
		if (*buf_count < ctx->pb_count)
			*buf_count = ctx->pb_count;
		if (*buf_count > ctx->pb_count + MFC_MAX_EXTRA_DPB)
			*buf_count = ctx->pb_count + MFC_MAX_EXTRA_DPB;
		if (*buf_count > MFC_MAX_BUFFERS)
			*buf_count = MFC_MAX_BUFFERS;
	} else {
		mfc_err("State seems invalid. State = %d, vq->type = %d\n",
							ctx->state, vq->type);
		return -EINVAL;
	}
	mfc_debug(2, "Buffer count=%d, plane count=%d\n",
						*buf_count, *plane_count);
	if (ctx->state == MFCINST_HEAD_PARSED &&
	    vq->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		psize[0] = ctx->luma_size;
		psize[1] = ctx->chroma_size;

		if (IS_MFCV6_PLUS(dev))
			allocators[0] =
				ctx->dev->alloc_ctx[MFC_BANK1_ALLOC_CTX];
		else
			allocators[0] =
				ctx->dev->alloc_ctx[MFC_BANK2_ALLOC_CTX];
		allocators[1] = ctx->dev->alloc_ctx[MFC_BANK1_ALLOC_CTX];
	} else if (vq->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE &&
		   ctx->state == MFCINST_INIT) {
		psize[0] = ctx->dec_src_buf_size;
		allocators[0] = ctx->dev->alloc_ctx[MFC_BANK1_ALLOC_CTX];
	} else {
		mfc_err("This video node is dedicated to decoding. Decoding not initialized\n");
		return -EINVAL;
	}
	return 0;
}

static void s5p_mfc_unlock(struct vb2_queue *q)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(q->drv_priv);
	struct s5p_mfc_dev *dev = ctx->dev;

	mutex_unlock(&dev->mfc_mutex);
}

static void s5p_mfc_lock(struct vb2_queue *q)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(q->drv_priv);
	struct s5p_mfc_dev *dev = ctx->dev;

	mutex_lock(&dev->mfc_mutex);
}

static int s5p_mfc_buf_init(struct vb2_buffer *vb)
{
	struct vb2_queue *vq = vb->vb2_queue;
	struct s5p_mfc_ctx *ctx = fh_to_ctx(vq->drv_priv);
	unsigned int i;

	if (vq->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		if (ctx->capture_state == QUEUE_BUFS_MMAPED)
			return 0;
		for (i = 0; i <= ctx->src_fmt->num_planes ; i++) {
			if (IS_ERR_OR_NULL(ERR_PTR(
					vb2_dma_contig_plane_dma_addr(vb, i)))) {
				mfc_err("Plane mem not allocated\n");
				return -EINVAL;
			}
		}
		if (vb2_plane_size(vb, 0) < ctx->luma_size ||
			vb2_plane_size(vb, 1) < ctx->chroma_size) {
			mfc_err("Plane buffer (CAPTURE) is too small\n");
			return -EINVAL;
		}
		i = vb->v4l2_buf.index;
		ctx->dst_bufs[i].b = vb;
		ctx->dst_bufs[i].cookie.raw.luma =
					vb2_dma_contig_plane_dma_addr(vb, 0);
		ctx->dst_bufs[i].cookie.raw.chroma =
					vb2_dma_contig_plane_dma_addr(vb, 1);
		ctx->dst_bufs_cnt++;
	} else if (vq->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		if (IS_ERR_OR_NULL(ERR_PTR(
					vb2_dma_contig_plane_dma_addr(vb, 0)))) {
			mfc_err("Plane memory not allocated\n");
			return -EINVAL;
		}
		if (vb2_plane_size(vb, 0) < ctx->dec_src_buf_size) {
			mfc_err("Plane buffer (OUTPUT) is too small\n");
			return -EINVAL;
		}

		i = vb->v4l2_buf.index;
		ctx->src_bufs[i].b = vb;
		ctx->src_bufs[i].cookie.stream =
					vb2_dma_contig_plane_dma_addr(vb, 0);
		ctx->src_bufs_cnt++;
	} else {
		mfc_err("s5p_mfc_buf_init: unknown queue type\n");
		return -EINVAL;
	}
	return 0;
}

static int s5p_mfc_start_streaming(struct vb2_queue *q, unsigned int count)
{
	struct s5p_mfc_ctx *ctx = fh_to_ctx(q->drv_priv);
	struct s5p_mfc_dev *dev = ctx->dev;

	v4l2_ctrl_handler_setup(&ctx->ctrl_handler);
	if (ctx->state == MFCINST_FINISHING ||
		ctx->state == MFCINST_FINISHED)
		ctx->state = MFCINST_RUNNING;
	/* If context is ready then dev = work->data;schedule it to run */
	if (s5p_mfc_ctx_ready(ctx))
		set_work_bit_irqsave(ctx);
	s5p_mfc_hw_call(dev->mfc_ops, try_run, dev);
	return 0;
}

static void s5p_mfc_stop_streaming(struct vb2_queue *q)
{
	unsigned long flags;
	struct s5p_mfc_ctx *ctx = fh_to_ctx(q->drv_priv);
	struct s5p_mfc_dev *dev = ctx->dev;
	int aborted = 0;

	if ((ctx->state == MFCINST_FINISHING ||
		ctx->state ==  MFCINST_RUNNING) &&
		dev->curr_ctx == ctx->num && dev->hw_lock) {
		ctx->state = MFCINST_ABORT;
		s5p_mfc_wait_for_done_ctx(ctx,
					S5P_MFC_R2H_CMD_FRAME_DONE_RET, 0);
		aborted = 1;
	}
	if (q->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		spin_lock_irqsave(&dev->irqlock, flags);
		s5p_mfc_hw_call(dev->mfc_ops, cleanup_queue, &ctx->dst_queue,
				&ctx->vq_dst);
		INIT_LIST_HEAD(&ctx->dst_queue);
		ctx->dst_queue_cnt = 0;
		ctx->dpb_flush_flag = 1;
		ctx->dec_dst_flag = 0;
		spin_unlock_irqrestore(&dev->irqlock, flags);
		if (IS_MFCV6_PLUS(dev) && (ctx->state == MFCINST_RUNNING)) {
			ctx->state = MFCINST_FLUSH;
			set_work_bit_irqsave(ctx);
			s5p_mfc_clean_ctx_int_flags(ctx);
			s5p_mfc_hw_call(dev->mfc_ops, try_run, dev);
			if (s5p_mfc_wait_for_done_ctx(ctx,
				S5P_MFC_R2H_CMD_DPB_FLUSH_RET, 0))
				mfc_err("Err flushing buffers\n");
		}
	}
	if (q->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		spin_lock_irqsave(&dev->irqlock, flags);
		s5p_mfc_hw_call(dev->mfc_ops, cleanup_queue, &ctx->src_queue,
				&ctx->vq_src);
		INIT_LIST_HEAD(&ctx->src_queue);
		ctx->src_queue_cnt = 0;
		spin_unlock_irqrestore(&dev->irqlock, flags);
	}
	if (aborted)
		ctx->state = MFCINST_RUNNING;
}


static void s5p_mfc_buf_queue(struct vb2_buffer *vb)
{
	struct vb2_queue *vq = vb->vb2_queue;
	struct s5p_mfc_ctx *ctx = fh_to_ctx(vq->drv_priv);
	struct s5p_mfc_dev *dev = ctx->dev;
	unsigned long flags;
	struct s5p_mfc_buf *mfc_buf;

	if (vq->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		mfc_buf = &ctx->src_bufs[vb->v4l2_buf.index];
		mfc_buf->flags &= ~MFC_BUF_FLAG_USED;
		spin_lock_irqsave(&dev->irqlock, flags);
		list_add_tail(&mfc_buf->list, &ctx->src_queue);
		ctx->src_queue_cnt++;
		spin_unlock_irqrestore(&dev->irqlock, flags);
	} else if (vq->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		mfc_buf = &ctx->dst_bufs[vb->v4l2_buf.index];
		mfc_buf->flags &= ~MFC_BUF_FLAG_USED;
		/* Mark destination as available for use by MFC */
		spin_lock_irqsave(&dev->irqlock, flags);
		set_bit(vb->v4l2_buf.index, &ctx->dec_dst_flag);
		list_add_tail(&mfc_buf->list, &ctx->dst_queue);
		ctx->dst_queue_cnt++;
		spin_unlock_irqrestore(&dev->irqlock, flags);
	} else {
		mfc_err("Unsupported buffer type (%d)\n", vq->type);
	}
	if (s5p_mfc_ctx_ready(ctx))
		set_work_bit_irqsave(ctx);
	s5p_mfc_hw_call(dev->mfc_ops, try_run, dev);
}

static struct vb2_ops s5p_mfc_dec_qops = {
	.queue_setup		= s5p_mfc_queue_setup,
	.wait_prepare		= s5p_mfc_unlock,
	.wait_finish		= s5p_mfc_lock,
	.buf_init		= s5p_mfc_buf_init,
	.start_streaming	= s5p_mfc_start_streaming,
	.stop_streaming		= s5p_mfc_stop_streaming,
	.buf_queue		= s5p_mfc_buf_queue,
};

struct s5p_mfc_codec_ops *get_dec_codec_ops(void)
{
	return &decoder_codec_ops;
}

struct vb2_ops *get_dec_queue_ops(void)
{
	return &s5p_mfc_dec_qops;
}

const struct v4l2_ioctl_ops *get_dec_v4l2_ioctl_ops(void)
{
	return &s5p_mfc_dec_ioctl_ops;
}

#define IS_MFC51_PRIV(x) ((V4L2_CTRL_ID2CLASS(x) == V4L2_CTRL_CLASS_MPEG) \
						&& V4L2_CTRL_DRIVER_PRIV(x))

int s5p_mfc_dec_ctrls_setup(struct s5p_mfc_ctx *ctx)
{
	struct v4l2_ctrl_config cfg;
	int i;

	v4l2_ctrl_handler_init(&ctx->ctrl_handler, NUM_CTRLS);
	if (ctx->ctrl_handler.error) {
		mfc_err("v4l2_ctrl_handler_init failed\n");
		return ctx->ctrl_handler.error;
	}

	for (i = 0; i < NUM_CTRLS; i++) {
		if (IS_MFC51_PRIV(controls[i].id)) {
			memset(&cfg, 0, sizeof(struct v4l2_ctrl_config));
			cfg.ops = &s5p_mfc_dec_ctrl_ops;
			cfg.id = controls[i].id;
			cfg.min = controls[i].minimum;
			cfg.max = controls[i].maximum;
			cfg.def = controls[i].default_value;
			cfg.name = controls[i].name;
			cfg.type = controls[i].type;

			cfg.step = controls[i].step;
			cfg.menu_skip_mask = 0;

			ctx->ctrls[i] = v4l2_ctrl_new_custom(&ctx->ctrl_handler,
					&cfg, NULL);
		} else {
			ctx->ctrls[i] = v4l2_ctrl_new_std(&ctx->ctrl_handler,
					&s5p_mfc_dec_ctrl_ops,
					controls[i].id, controls[i].minimum,
					controls[i].maximum, controls[i].step,
					controls[i].default_value);
		}
		if (ctx->ctrl_handler.error) {
			mfc_err("Adding control (%d) failed\n", i);
			return ctx->ctrl_handler.error;
		}
		if (controls[i].is_volatile && ctx->ctrls[i])
			ctx->ctrls[i]->flags |= V4L2_CTRL_FLAG_VOLATILE;
	}
	return 0;
}

void s5p_mfc_dec_ctrls_delete(struct s5p_mfc_ctx *ctx)
{
	int i;

	v4l2_ctrl_handler_free(&ctx->ctrl_handler);
	for (i = 0; i < NUM_CTRLS; i++)
		ctx->ctrls[i] = NULL;
}

void s5p_mfc_dec_init(struct s5p_mfc_ctx *ctx)
{
	struct v4l2_format f;
	f.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	ctx->src_fmt = find_format(&f, MFC_FMT_DEC);
	if (IS_MFCV8(ctx->dev))
		f.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12M;
	else if (IS_MFCV6_PLUS(ctx->dev))
		f.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12MT_16X16;
	else
		f.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12MT;
	ctx->dst_fmt = find_format(&f, MFC_FMT_RAW);
	mfc_debug(2, "Default src_fmt is %x, dest_fmt is %x\n",
			(unsigned int)ctx->src_fmt, (unsigned int)ctx->dst_fmt);
}

