/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

#ifndef __ARCH_ARM_MACH_MSM_CORE_H
#define __ARCH_ARM_MACH_MSM_CORE_H
#ifdef CONFIG_APSS_CORE_EA
void set_cpu_throttled(struct cpumask *mask, bool throttling);
#else
static inline void set_cpu_throttled(struct cpumask *mask, bool throttling) {}
#endif
#endif

