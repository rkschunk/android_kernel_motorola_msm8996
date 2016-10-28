/*
 * Copyright (C) STMicroelectronics SA 2014
 * Author: Fabien Dessenne <fabien.dessenne@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef _STI_VID_H_
#define _STI_VID_H_

/**
 * STI VID structure
 *
 * @dev:   driver device
 * @regs:  vid registers
 * @id:    id of the vid
 */
struct sti_vid {
	struct device *dev;
	void __iomem *regs;
	int id;
};

void sti_vid_commit(struct sti_vid *vid,
		    struct drm_plane_state *state);
void sti_vid_disable(struct sti_vid *vid);
struct sti_vid *sti_vid_create(struct device *dev, int id,
			       void __iomem *baseaddr);

#endif
