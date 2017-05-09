/* Copyright (c) 2013-2017, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/io.h>
#include <linux/atomic.h>
#include <media/v4l2-subdev.h>
#include <media/msmb_isp.h>
#include "msm_isp_util_32.h"
#include "msm_isp_stats_util_32.h"

static int msm_isp_stats_cfg_ping_pong_address(struct vfe_device *vfe_dev,
	struct msm_vfe_stats_stream *stream_info, uint32_t pingpong_status,
	struct msm_isp_buffer **done_buf)
{
	int rc = -1;
	struct msm_isp_buffer *buf;
	uint32_t pingpong_bit = 0;
	uint32_t bufq_handle = stream_info->bufq_handle;
	uint32_t stats_pingpong_offset;
	uint32_t stats_idx = STATS_IDX(stream_info->stream_handle);

	if (stats_idx >= vfe_dev->hw_info->stats_hw_info->num_stats_type ||
		stats_idx >= MSM_ISP_STATS_MAX) {
		pr_err("%s Invalid stats index %d", __func__, stats_idx);
		return -EINVAL;
	}

	stats_pingpong_offset =
		vfe_dev->hw_info->stats_hw_info->stats_ping_pong_offset[
		stats_idx];

	pingpong_bit = (~(pingpong_status >> stats_pingpong_offset) & 0x1);
	rc = vfe_dev->buf_mgr->ops->get_buf(vfe_dev->buf_mgr,
			vfe_dev->pdev->id, bufq_handle, &buf);
	if (rc < 0) {
		vfe_dev->error_info.stats_framedrop_count[stats_idx]++;
		return rc;
	}

	if (buf->num_planes != 1) {
		pr_err("%s: Invalid buffer\n", __func__);
		rc = -EINVAL;
		goto buf_error;
	}

	vfe_dev->hw_info->vfe_ops.stats_ops.update_ping_pong_addr(
		vfe_dev, stream_info,
		pingpong_status, buf->mapped_info[0].paddr +
		stream_info->buffer_offset);

	if (stream_info->buf[pingpong_bit] && done_buf)
		*done_buf = stream_info->buf[pingpong_bit];

	stream_info->buf[pingpong_bit] = buf;
	return 0;
buf_error:
	vfe_dev->buf_mgr->ops->put_buf(vfe_dev->buf_mgr,
		buf->bufq_handle, buf->buf_idx);
	return rc;
}

void msm_isp_process_stats_irq(struct vfe_device *vfe_dev,
	uint32_t irq_status0, uint32_t irq_status1,
	struct msm_isp_timestamp *ts)
{
	int i, j, rc;
	struct msm_isp32_event_data buf_event;
	struct msm_isp_stats_event *stats_event = &buf_event.u.stats;
	struct msm_isp_buffer *done_buf;
	struct msm_vfe_stats_stream *stream_info = NULL;
	uint32_t pingpong_status;
	uint32_t comp_stats_type_mask = 0, atomic_stats_mask = 0;
	uint32_t stats_comp_mask = 0, stats_irq_mask = 0;
	uint32_t num_stats_comp_mask =
		vfe_dev->hw_info->stats_hw_info->num_stats_comp_mask;
	stats_comp_mask = vfe_dev->hw_info->vfe_ops.stats_ops.
		get_comp_mask(irq_status0, irq_status1);
	stats_irq_mask = vfe_dev->hw_info->vfe_ops.stats_ops.
		get_wm_mask(irq_status0, irq_status1);
	if (!(stats_comp_mask || stats_irq_mask))
		return;
	ISP_DBG("%s: status: 0x%x\n", __func__, irq_status0);

	/*
	 * If any of composite mask is set, clear irq bits from mask,
	 * they will be restored by comp mask
	 */
	if (stats_comp_mask) {
		for (j = 0; j < num_stats_comp_mask; j++) {
			stats_irq_mask &= ~atomic_read(
				&vfe_dev->stats_data.stats_comp_mask[j]);
		}
	}

	for (j = 0; j < num_stats_comp_mask; j++) {
		atomic_stats_mask = atomic_read(
			&vfe_dev->stats_data.stats_comp_mask[j]);
		if (!stats_comp_mask) {
			stats_irq_mask &= ~atomic_stats_mask;
		} else {
			/* restore irq bits from composite mask */
			if (stats_comp_mask & (1 << j))
				stats_irq_mask |= atomic_stats_mask;
		}
		/* if no irq bits set from this composite mask continue*/
		if (!stats_irq_mask)
			continue;
		memset(&buf_event, 0, sizeof(struct msm_isp32_event_data));
		buf_event.timestamp = ts->event_time;
		buf_event.frame_id =
			vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id;
		buf_event.input_intf = VFE_PIX_0;
		pingpong_status = vfe_dev->hw_info->
			vfe_ops.stats_ops.get_pingpong_status(vfe_dev);

		for (i = 0; i < vfe_dev->hw_info->stats_hw_info->num_stats_type;
			i++) {
			if (!(stats_irq_mask & (1 << i)))
				continue;

			stats_irq_mask &= ~(1 << i);
			stream_info = &vfe_dev->stats_data.stream_info[i];
			done_buf = NULL;
			msm_isp_stats_cfg_ping_pong_address(vfe_dev,
				stream_info, pingpong_status, &done_buf);
			if (done_buf) {
				rc = vfe_dev->buf_mgr->ops->buf_divert(
					vfe_dev->buf_mgr, done_buf->bufq_handle,
					done_buf->buf_idx, &ts->buf_time,
					vfe_dev->axi_data.
					src_info[VFE_PIX_0].frame_id);
				if (rc != 0)
					continue;

				stats_event->stats_buf_idxs
					[stream_info->stats_type] =
					done_buf->buf_idx;
				if (!stream_info->composite_flag) {
					stats_event->stats_mask =
						1 << stream_info->stats_type;
					ISP_DBG("%s: stats frameid: 0x%x %d\n",
						__func__, buf_event.frame_id,
						stream_info->stats_type);
					msm_isp_send_event(vfe_dev,
						ISP_EVENT_STATS_NOTIFY +
						stream_info->stats_type,
						&buf_event);
				} else {
					comp_stats_type_mask |=
						1 << stream_info->stats_type;
				}
			}
		}

		if (comp_stats_type_mask) {
			ISP_DBG("%s: comp_stats frameid: 0x%x, 0x%x\n",
				__func__, buf_event.frame_id,
				comp_stats_type_mask);
			stats_event->stats_mask = comp_stats_type_mask;
			msm_isp_send_event(vfe_dev,
				ISP_EVENT_COMP_STATS_NOTIFY, &buf_event);
			comp_stats_type_mask = 0;
		}
	}
}

int msm_isp_stats_create_stream(struct vfe_device *vfe_dev,
	struct msm_vfe_stats_stream_request_cmd *stream_req_cmd)
{
	int rc = -1;
	struct msm_vfe_stats_stream *stream_info = NULL;
	struct msm_vfe_stats_shared_data *stats_data = &vfe_dev->stats_data;
	uint32_t stats_idx;

	if (!(vfe_dev->hw_info->stats_hw_info->stats_capability_mask &
		(1 << stream_req_cmd->stats_type))) {
		pr_err("%s: Stats type not supported\n", __func__);
		return rc;
	}

	stats_idx = vfe_dev->hw_info->vfe_ops.stats_ops.
		get_stats_idx(stream_req_cmd->stats_type);

	if (stats_idx >= vfe_dev->hw_info->stats_hw_info->num_stats_type) {
		pr_err("%s Invalid stats index %d", __func__, stats_idx);
		return -EINVAL;
	}

	stream_info = &stats_data->stream_info[stats_idx];
	if (stream_info->state != STATS_AVAILABLE) {
		pr_err("%s: Stats already requested\n", __func__);
		return rc;
	}

	if (stream_req_cmd->framedrop_pattern >= MAX_SKIP) {
		pr_err("%s: Invalid framedrop pattern\n", __func__);
		return rc;
	}

	if (stream_req_cmd->irq_subsample_pattern >= MAX_SKIP) {
		pr_err("%s: Invalid irq subsample pattern\n", __func__);
		return rc;
	}

	stream_info->session_id = stream_req_cmd->session_id;
	stream_info->stream_id = stream_req_cmd->stream_id;
	stream_info->composite_flag = stream_req_cmd->composite_flag;
	stream_info->stats_type = stream_req_cmd->stats_type;
	stream_info->buffer_offset = stream_req_cmd->buffer_offset;
	stream_info->framedrop_pattern = stream_req_cmd->framedrop_pattern;
	stream_info->init_stats_frame_drop = stream_req_cmd->init_frame_drop;
	stream_info->irq_subsample_pattern =
		stream_req_cmd->irq_subsample_pattern;
	stream_info->state = STATS_INACTIVE;

	if ((vfe_dev->stats_data.stream_handle_cnt << 8) == 0)
		vfe_dev->stats_data.stream_handle_cnt++;

	stream_req_cmd->stream_handle =
		(++vfe_dev->stats_data.stream_handle_cnt) << 8 | stats_idx;

	stream_info->stream_handle = stream_req_cmd->stream_handle;
	return 0;
}

int msm_isp_request_stats_stream(struct vfe_device *vfe_dev, void *arg)
{
	int rc = -1;
	struct msm_vfe_stats_stream_request_cmd *stream_req_cmd = arg;
	struct msm_vfe_stats_stream *stream_info = NULL;
	struct msm_vfe_stats_shared_data *stats_data = &vfe_dev->stats_data;
	uint32_t framedrop_period;
	uint32_t stats_idx;

	rc = msm_isp_stats_create_stream(vfe_dev, stream_req_cmd);
	if (rc < 0) {
		pr_err("%s: create stream failed\n", __func__);
		return rc;
	}

	stats_idx = STATS_IDX(stream_req_cmd->stream_handle);

	if (stats_idx >= vfe_dev->hw_info->stats_hw_info->num_stats_type) {
		pr_err("%s Invalid stats index %d", __func__, stats_idx);
		return -EINVAL;
	}

	stream_info = &stats_data->stream_info[stats_idx];

	framedrop_period = msm_isp_get_framedrop_period(
	   stream_req_cmd->framedrop_pattern);

	if (stream_req_cmd->framedrop_pattern == SKIP_ALL)
		stream_info->framedrop_pattern = 0x0;
	else
		stream_info->framedrop_pattern = 0x1;
	stream_info->framedrop_period = framedrop_period - 1;

	if (!stream_info->composite_flag)
		vfe_dev->hw_info->vfe_ops.stats_ops.
			cfg_wm_irq_mask(vfe_dev, stream_info);

	if (stream_info->init_stats_frame_drop == 0)
		vfe_dev->hw_info->vfe_ops.stats_ops.cfg_wm_reg(vfe_dev,
			stream_info);

	return rc;
}

int msm_isp_release_stats_stream(struct vfe_device *vfe_dev, void *arg)
{
	int rc = -1;
	struct msm_vfe_stats_stream_cfg_cmd stream_cfg_cmd;
	struct msm_vfe_stats_stream_release_cmd *stream_release_cmd = arg;
	struct msm_vfe_stats_shared_data *stats_data = &vfe_dev->stats_data;
	int stats_idx = STATS_IDX(stream_release_cmd->stream_handle);
	struct msm_vfe_stats_stream *stream_info = NULL;

	if (stats_idx >= vfe_dev->hw_info->stats_hw_info->num_stats_type) {
		pr_err("%s Invalid stats index %d", __func__, stats_idx);
		return -EINVAL;
	}

	stream_info = &stats_data->stream_info[stats_idx];
	if (stream_info->state == STATS_AVAILABLE) {
		pr_err("%s: stream already release\n", __func__);
		return rc;
	} else if (stream_info->state != STATS_INACTIVE) {
		stream_cfg_cmd.enable = 0;
		stream_cfg_cmd.num_streams = 1;
		stream_cfg_cmd.stream_handle[0] =
			stream_release_cmd->stream_handle;
		rc = msm_isp_cfg_stats_stream(vfe_dev, &stream_cfg_cmd);
	}

	if (!stream_info->composite_flag)
		vfe_dev->hw_info->vfe_ops.stats_ops.
			clear_wm_irq_mask(vfe_dev, stream_info);

	vfe_dev->hw_info->vfe_ops.stats_ops.clear_wm_reg(vfe_dev, stream_info);
	memset(stream_info, 0, sizeof(struct msm_vfe_stats_stream));
	return 0;
}

static int msm_isp_init_stats_ping_pong_reg(
	struct vfe_device *vfe_dev,
	struct msm_vfe_stats_stream *stream_info)
{
	int rc = 0;

	stream_info->bufq_handle =
		vfe_dev->buf_mgr->ops->get_bufq_handle(
		vfe_dev->buf_mgr, stream_info->session_id,
		stream_info->stream_id);
	if (stream_info->bufq_handle == 0) {
		pr_err("%s: no buf configured for stream: 0x%x\n",
			__func__, stream_info->stream_handle);
		return -EINVAL;
	}

	rc = msm_isp_stats_cfg_ping_pong_address(vfe_dev,
		stream_info, VFE_PING_FLAG, NULL);
	if (rc < 0) {
		pr_err("%s: No free buffer for ping\n", __func__);
		return rc;
	}
	rc = msm_isp_stats_cfg_ping_pong_address(vfe_dev,
		stream_info, VFE_PONG_FLAG, NULL);
	if (rc < 0) {
		pr_err("%s: No free buffer for pong\n", __func__);
		return rc;
	}
	return rc;
}

static void msm_isp_deinit_stats_ping_pong_reg(
	struct vfe_device *vfe_dev,
	struct msm_vfe_stats_stream *stream_info)
{
	int i;
	struct msm_isp_buffer *buf;

	for (i = 0; i < 2; i++) {
		buf = stream_info->buf[i];
		if (buf)
			vfe_dev->buf_mgr->ops->put_buf(vfe_dev->buf_mgr,
				buf->bufq_handle, buf->buf_idx);
	}
}

void msm_isp_update_stats_framedrop_reg(struct vfe_device *vfe_dev)
{
	int i;
	struct msm_vfe_stats_shared_data *stats_data = &vfe_dev->stats_data;
	struct msm_vfe_stats_stream *stream_info = NULL;

	for (i = 0; i < vfe_dev->hw_info->stats_hw_info->num_stats_type; i++) {
		stream_info = &stats_data->stream_info[i];
		if (stream_info->state != STATS_ACTIVE)
			continue;

		if (stream_info->init_stats_frame_drop) {
			stream_info->init_stats_frame_drop--;
			if (stream_info->init_stats_frame_drop == 0) {
				vfe_dev->hw_info->vfe_ops.stats_ops.cfg_wm_reg(
					vfe_dev, stream_info);
			}
		}
	}
}

void msm_isp_stats_stream_update(struct vfe_device *vfe_dev)
{
	int i;
	uint32_t stats_mask = 0, comp_stats_mask = 0;
	uint32_t enable = 0;
	struct msm_vfe_stats_shared_data *stats_data = &vfe_dev->stats_data;

	for (i = 0; i < vfe_dev->hw_info->stats_hw_info->num_stats_type; i++) {
		if (stats_data->stream_info[i].state == STATS_START_PENDING ||
				stats_data->stream_info[i].state ==
					STATS_STOP_PENDING) {
			stats_mask |= i;
			enable = stats_data->stream_info[i].state ==
				STATS_START_PENDING ? 1 : 0;
			stats_data->stream_info[i].state =
				stats_data->stream_info[i].state ==
				STATS_START_PENDING ?
				STATS_STARTING : STATS_STOPPING;
			vfe_dev->hw_info->vfe_ops.stats_ops.enable_module(
				vfe_dev, BIT(i), enable);
			vfe_dev->hw_info->vfe_ops.stats_ops.cfg_comp_mask(
			   vfe_dev, BIT(i), enable);
		} else if (stats_data->stream_info[i].state == STATS_STARTING ||
			stats_data->stream_info[i].state == STATS_STOPPING) {
			if (stats_data->stream_info[i].composite_flag)
				comp_stats_mask |= i;
			stats_data->stream_info[i].state =
				stats_data->stream_info[i].state ==
				STATS_STARTING ? STATS_ACTIVE : STATS_INACTIVE;
		}
	}
	atomic_sub(1, &stats_data->stats_update);
	if (!atomic_read(&stats_data->stats_update))
		complete(&vfe_dev->stats_config_complete);
}

static int msm_isp_stats_wait_for_cfg_done(struct vfe_device *vfe_dev)
{
	int rc;

	init_completion(&vfe_dev->stats_config_complete);
	atomic_set(&vfe_dev->stats_data.stats_update, 2);
	rc = wait_for_completion_timeout(
		&vfe_dev->stats_config_complete,
		msecs_to_jiffies(VFE_MAX_CFG_TIMEOUT));
	if (rc == 0) {
		pr_err("%s: wait timeout\n", __func__);
		rc = -1;
	} else {
		rc = 0;
	}
	return rc;
}

static int msm_isp_stats_update_cgc_override(struct vfe_device *vfe_dev,
	struct msm_vfe_stats_stream_cfg_cmd *stream_cfg_cmd)
{
	int i;
	uint32_t stats_mask = 0, idx;

	if (stream_cfg_cmd->num_streams > MSM_ISP_STATS_MAX) {
		pr_err("%s invalid num_streams %d\n", __func__,
			stream_cfg_cmd->num_streams);
		return -EINVAL;
	}

	for (i = 0; i < stream_cfg_cmd->num_streams; i++) {
		idx = STATS_IDX(stream_cfg_cmd->stream_handle[i]);

		if (idx >= vfe_dev->hw_info->stats_hw_info->num_stats_type) {
			pr_err("%s Invalid stats index %d", __func__, idx);
			return -EINVAL;
		}
		stats_mask |= 1 << idx;
	}

	if (vfe_dev->hw_info->vfe_ops.stats_ops.update_cgc_override) {
		vfe_dev->hw_info->vfe_ops.stats_ops.update_cgc_override(
			vfe_dev, stats_mask, stream_cfg_cmd->enable);
	}
	return 0;
}

static int msm_isp_start_stats_stream(struct vfe_device *vfe_dev,
	struct msm_vfe_stats_stream_cfg_cmd *stream_cfg_cmd)
{
	int i, rc = 0;
	uint32_t stats_mask = 0, idx;
	uint32_t comp_stats_mask[MAX_NUM_STATS_COMP_MASK] = {0};
	uint32_t num_stats_comp_mask = 0;
	struct msm_vfe_stats_stream *stream_info;
	struct msm_vfe_stats_shared_data *stats_data = &vfe_dev->stats_data;

	if (stream_cfg_cmd->num_streams > MSM_ISP_STATS_MAX) {
		pr_err("%s invalid num_streams %d\n", __func__,
			stream_cfg_cmd->num_streams);
		return -EINVAL;
	}

	num_stats_comp_mask =
		vfe_dev->hw_info->stats_hw_info->num_stats_comp_mask;
	rc = vfe_dev->hw_info->vfe_ops.stats_ops.check_streams(
		stats_data->stream_info);
	if (rc < 0)
		return rc;
	for (i = 0; i < stream_cfg_cmd->num_streams; i++) {
		idx = STATS_IDX(stream_cfg_cmd->stream_handle[i]);

		if (idx >= vfe_dev->hw_info->stats_hw_info->num_stats_type) {
			pr_err("%s Invalid stats index %d", __func__, idx);
			return -EINVAL;
		}

		stream_info = &stats_data->stream_info[idx];
		if (stream_info->stream_handle !=
				stream_cfg_cmd->stream_handle[i]) {
			pr_err("%s: Invalid stream handle: 0x%x received\n",
				__func__, stream_cfg_cmd->stream_handle[i]);
			continue;
		}

		if (stream_info->composite_flag > num_stats_comp_mask) {
			pr_err("%s: comp grp %d exceed max %d\n",
				__func__, stream_info->composite_flag,
				num_stats_comp_mask);
			return -EINVAL;
		}
		rc = msm_isp_init_stats_ping_pong_reg(vfe_dev, stream_info);
		if (rc < 0) {
			pr_err("%s: No buffer for stream%d type:%d stmID:0x%x\n",
				__func__, idx, stream_info->stats_type,
				stream_info->stream_id);
			return rc;
		}

		if (vfe_dev->axi_data.src_info[VFE_PIX_0].active)
			stream_info->state = STATS_START_PENDING;
		else
			stream_info->state = STATS_ACTIVE;

		stats_data->num_active_stream++;
		stats_mask |= 1 << idx;

		if (stream_info->composite_flag > 0)
			comp_stats_mask[stream_info->composite_flag-1] |=
				1 << idx;

		ISP_DBG("%s: stats_mask %x %x active streams %d\n",
			__func__, comp_stats_mask[0],
			comp_stats_mask[1],
			stats_data->num_active_stream);

	}

	if (vfe_dev->axi_data.src_info[VFE_PIX_0].active) {
		rc = msm_isp_stats_wait_for_cfg_done(vfe_dev);
	} else {
		vfe_dev->hw_info->vfe_ops.stats_ops.enable_module(
			vfe_dev, stats_mask, stream_cfg_cmd->enable);
		for (i = 0; i < num_stats_comp_mask; i++) {
			vfe_dev->hw_info->vfe_ops.stats_ops.cfg_comp_mask(
			 vfe_dev, comp_stats_mask[i], 1);
		}
	}
	return rc;
}

static int msm_isp_stop_stats_stream(struct vfe_device *vfe_dev,
	struct msm_vfe_stats_stream_cfg_cmd *stream_cfg_cmd)
{
	int i, rc = 0;
	uint32_t stats_mask = 0, idx;
	uint32_t comp_stats_mask[MAX_NUM_STATS_COMP_MASK] = {0};
	uint32_t num_stats_comp_mask = 0;
	struct msm_vfe_stats_stream *stream_info;
	struct msm_vfe_stats_shared_data *stats_data = &vfe_dev->stats_data;

	num_stats_comp_mask =
		vfe_dev->hw_info->stats_hw_info->num_stats_comp_mask;

	for (i = 0; i < stream_cfg_cmd->num_streams; i++) {

		idx = STATS_IDX(stream_cfg_cmd->stream_handle[i]);

		if (idx >= vfe_dev->hw_info->stats_hw_info->num_stats_type) {
			pr_err("%s Invalid stats index %d", __func__, idx);
			return -EINVAL;
		}

		stream_info = &stats_data->stream_info[idx];
		if (stream_info->stream_handle !=
				stream_cfg_cmd->stream_handle[i]) {
			pr_err("%s: Invalid stream handle: 0x%x received\n",
				__func__, stream_cfg_cmd->stream_handle[i]);
			continue;
		}

		if (stream_info->composite_flag > num_stats_comp_mask) {
			pr_err("%s: comp grp %d exceed max %d\n",
				__func__, stream_info->composite_flag,
				num_stats_comp_mask);
			return -EINVAL;
		}

		if (vfe_dev->axi_data.src_info[VFE_PIX_0].active)
			stream_info->state = STATS_STOP_PENDING;
		else
			stream_info->state = STATS_INACTIVE;

		stats_data->num_active_stream--;
		stats_mask |= 1 << idx;

		if (stream_info->composite_flag > 0)
			comp_stats_mask[stream_info->composite_flag-1] |=
				1 << idx;

		ISP_DBG("%s: stats_mask %x %x active streams %d\n",
			__func__, comp_stats_mask[0],
			comp_stats_mask[1],
			stats_data->num_active_stream);
	}

	if (vfe_dev->axi_data.src_info[VFE_PIX_0].active) {
		rc = msm_isp_stats_wait_for_cfg_done(vfe_dev);
	} else {
		vfe_dev->hw_info->vfe_ops.stats_ops.enable_module(
			vfe_dev, stats_mask, stream_cfg_cmd->enable);
		for (i = 0; i < num_stats_comp_mask; i++) {
			vfe_dev->hw_info->vfe_ops.stats_ops.cfg_comp_mask(
			   vfe_dev, comp_stats_mask[i], 0);
		}
	}

	for (i = 0; i < stream_cfg_cmd->num_streams; i++) {
		idx = STATS_IDX(stream_cfg_cmd->stream_handle[i]);

		if (idx >= vfe_dev->hw_info->stats_hw_info->num_stats_type) {
			pr_err("%s Invalid stats index %d", __func__, idx);
			return -EINVAL;
		}

		stream_info = &stats_data->stream_info[idx];
		msm_isp_deinit_stats_ping_pong_reg(vfe_dev, stream_info);
	}
	return rc;
}

int msm_isp_cfg_stats_stream(struct vfe_device *vfe_dev, void *arg)
{
	int rc = 0;
	struct msm_vfe_stats_stream_cfg_cmd *stream_cfg_cmd = arg;

	if (vfe_dev->stats_data.num_active_stream == 0)
		vfe_dev->hw_info->vfe_ops.stats_ops.cfg_ub(vfe_dev);

	if (stream_cfg_cmd->enable) {
		msm_isp_stats_update_cgc_override(vfe_dev, stream_cfg_cmd);

		rc = msm_isp_start_stats_stream(vfe_dev, stream_cfg_cmd);
	} else {
		rc = msm_isp_stop_stats_stream(vfe_dev, stream_cfg_cmd);

		msm_isp_stats_update_cgc_override(vfe_dev, stream_cfg_cmd);
	}

	return rc;
}

int msm_isp_update_stats_stream(struct vfe_device *vfe_dev, void *arg)
{
	int rc = 0, i;
	struct msm_vfe_stats_stream *stream_info;
	struct msm_vfe_stats_shared_data *stats_data = &vfe_dev->stats_data;
	struct msm_vfe_axi_stream_update_cmd *update_cmd = arg;
	struct msm_vfe_axi_stream_cfg_update_info *update_info = NULL;

	/*validate request*/
	for (i = 0; i < update_cmd->num_streams; i++) {
		update_info = &update_cmd->update_info[i];
		/*check array reference bounds*/
		if (STATS_IDX(update_info->stream_handle)
			> vfe_dev->hw_info->stats_hw_info->num_stats_type) {
			pr_err("%s: stats idx %d out of bound!", __func__,
				STATS_IDX(update_info->stream_handle));
			return -EINVAL;
		}
	}

	for (i = 0; i < update_cmd->num_streams; i++) {
		update_info = &update_cmd->update_info[i];
		stream_info = &stats_data->stream_info[
				STATS_IDX(update_info->stream_handle)];
		if (stream_info->stream_handle !=
			update_info->stream_handle) {
			pr_err("%s: stats stream handle %x %x mismatch!\n",
				__func__, stream_info->stream_handle,
				update_info->stream_handle);
			continue;
		}

		switch (update_cmd->update_type) {
		case UPDATE_STREAM_STATS_FRAMEDROP_PATTERN: {
			uint32_t framedrop_period =
				msm_isp_get_framedrop_period(
				   update_info->skip_pattern);
			if (update_info->skip_pattern == SKIP_ALL)
				stream_info->framedrop_pattern = 0x0;
			else
				stream_info->framedrop_pattern = 0x1;
			stream_info->framedrop_period = framedrop_period - 1;
			if (stream_info->init_stats_frame_drop == 0)
				vfe_dev->hw_info->vfe_ops.stats_ops.cfg_wm_reg(
					vfe_dev, stream_info);
			break;
		}

		default:
			pr_err("%s: Invalid update type\n", __func__);
			return -EINVAL;
		}
	}
	return rc;
}
