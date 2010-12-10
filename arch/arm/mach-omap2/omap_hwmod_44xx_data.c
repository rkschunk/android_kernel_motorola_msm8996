/*
 * Hardware modules present on the OMAP44xx chips
 *
 * Copyright (C) 2009-2010 Texas Instruments, Inc.
 * Copyright (C) 2009-2010 Nokia Corporation
 *
 * Paul Walmsley
 * Benoit Cousson
 *
 * This file is automatically generated from the OMAP hardware databases.
 * We respectfully ask that any modifications to this file be coordinated
 * with the public linux-omap@vger.kernel.org mailing list and the
 * authors above to ensure that the autogeneration scripts are kept
 * up-to-date with the file contents.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/io.h>

#include <plat/omap_hwmod.h>
#include <plat/cpu.h>
#include <plat/gpio.h>

#include "omap_hwmod_common_data.h"

#include "cm.h"
#include "prm-regbits-44xx.h"

/* Base offset for all OMAP4 interrupts external to MPUSS */
#define OMAP44XX_IRQ_GIC_START	32

/* Base offset for all OMAP4 dma requests */
#define OMAP44XX_DMA_REQ_START  1

/* Backward references (IPs with Bus Master capability) */
static struct omap_hwmod omap44xx_dmm_hwmod;
static struct omap_hwmod omap44xx_emif_fw_hwmod;
static struct omap_hwmod omap44xx_l3_instr_hwmod;
static struct omap_hwmod omap44xx_l3_main_1_hwmod;
static struct omap_hwmod omap44xx_l3_main_2_hwmod;
static struct omap_hwmod omap44xx_l3_main_3_hwmod;
static struct omap_hwmod omap44xx_l4_abe_hwmod;
static struct omap_hwmod omap44xx_l4_cfg_hwmod;
static struct omap_hwmod omap44xx_l4_per_hwmod;
static struct omap_hwmod omap44xx_l4_wkup_hwmod;
static struct omap_hwmod omap44xx_mpu_hwmod;
static struct omap_hwmod omap44xx_mpu_private_hwmod;

/*
 * Interconnects omap_hwmod structures
 * hwmods that compose the global OMAP interconnect
 */

/*
 * 'dmm' class
 * instance(s): dmm
 */
static struct omap_hwmod_class omap44xx_dmm_hwmod_class = {
	.name = "dmm",
};

/* dmm interface data */
/* l3_main_1 -> dmm */
static struct omap_hwmod_ocp_if omap44xx_l3_main_1__dmm = {
	.master		= &omap44xx_l3_main_1_hwmod,
	.slave		= &omap44xx_dmm_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* mpu -> dmm */
static struct omap_hwmod_ocp_if omap44xx_mpu__dmm = {
	.master		= &omap44xx_mpu_hwmod,
	.slave		= &omap44xx_dmm_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* dmm slave ports */
static struct omap_hwmod_ocp_if *omap44xx_dmm_slaves[] = {
	&omap44xx_l3_main_1__dmm,
	&omap44xx_mpu__dmm,
};

static struct omap_hwmod_irq_info omap44xx_dmm_irqs[] = {
	{ .irq = 113 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod omap44xx_dmm_hwmod = {
	.name		= "dmm",
	.class		= &omap44xx_dmm_hwmod_class,
	.slaves		= omap44xx_dmm_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_dmm_slaves),
	.mpu_irqs	= omap44xx_dmm_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_dmm_irqs),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/*
 * 'emif_fw' class
 * instance(s): emif_fw
 */
static struct omap_hwmod_class omap44xx_emif_fw_hwmod_class = {
	.name = "emif_fw",
};

/* emif_fw interface data */
/* dmm -> emif_fw */
static struct omap_hwmod_ocp_if omap44xx_dmm__emif_fw = {
	.master		= &omap44xx_dmm_hwmod,
	.slave		= &omap44xx_emif_fw_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l4_cfg -> emif_fw */
static struct omap_hwmod_ocp_if omap44xx_l4_cfg__emif_fw = {
	.master		= &omap44xx_l4_cfg_hwmod,
	.slave		= &omap44xx_emif_fw_hwmod,
	.clk		= "l4_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* emif_fw slave ports */
static struct omap_hwmod_ocp_if *omap44xx_emif_fw_slaves[] = {
	&omap44xx_dmm__emif_fw,
	&omap44xx_l4_cfg__emif_fw,
};

static struct omap_hwmod omap44xx_emif_fw_hwmod = {
	.name		= "emif_fw",
	.class		= &omap44xx_emif_fw_hwmod_class,
	.slaves		= omap44xx_emif_fw_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_emif_fw_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/*
 * 'l3' class
 * instance(s): l3_instr, l3_main_1, l3_main_2, l3_main_3
 */
static struct omap_hwmod_class omap44xx_l3_hwmod_class = {
	.name = "l3",
};

/* l3_instr interface data */
/* l3_main_3 -> l3_instr */
static struct omap_hwmod_ocp_if omap44xx_l3_main_3__l3_instr = {
	.master		= &omap44xx_l3_main_3_hwmod,
	.slave		= &omap44xx_l3_instr_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l3_instr slave ports */
static struct omap_hwmod_ocp_if *omap44xx_l3_instr_slaves[] = {
	&omap44xx_l3_main_3__l3_instr,
};

static struct omap_hwmod omap44xx_l3_instr_hwmod = {
	.name		= "l3_instr",
	.class		= &omap44xx_l3_hwmod_class,
	.slaves		= omap44xx_l3_instr_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_l3_instr_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* l3_main_2 -> l3_main_1 */
static struct omap_hwmod_ocp_if omap44xx_l3_main_2__l3_main_1 = {
	.master		= &omap44xx_l3_main_2_hwmod,
	.slave		= &omap44xx_l3_main_1_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l4_cfg -> l3_main_1 */
static struct omap_hwmod_ocp_if omap44xx_l4_cfg__l3_main_1 = {
	.master		= &omap44xx_l4_cfg_hwmod,
	.slave		= &omap44xx_l3_main_1_hwmod,
	.clk		= "l4_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* mpu -> l3_main_1 */
static struct omap_hwmod_ocp_if omap44xx_mpu__l3_main_1 = {
	.master		= &omap44xx_mpu_hwmod,
	.slave		= &omap44xx_l3_main_1_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l3_main_1 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_l3_main_1_slaves[] = {
	&omap44xx_l3_main_2__l3_main_1,
	&omap44xx_l4_cfg__l3_main_1,
	&omap44xx_mpu__l3_main_1,
};

static struct omap_hwmod omap44xx_l3_main_1_hwmod = {
	.name		= "l3_main_1",
	.class		= &omap44xx_l3_hwmod_class,
	.slaves		= omap44xx_l3_main_1_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_l3_main_1_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* l3_main_2 interface data */
/* l3_main_1 -> l3_main_2 */
static struct omap_hwmod_ocp_if omap44xx_l3_main_1__l3_main_2 = {
	.master		= &omap44xx_l3_main_1_hwmod,
	.slave		= &omap44xx_l3_main_2_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l4_cfg -> l3_main_2 */
static struct omap_hwmod_ocp_if omap44xx_l4_cfg__l3_main_2 = {
	.master		= &omap44xx_l4_cfg_hwmod,
	.slave		= &omap44xx_l3_main_2_hwmod,
	.clk		= "l4_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l3_main_2 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_l3_main_2_slaves[] = {
	&omap44xx_l3_main_1__l3_main_2,
	&omap44xx_l4_cfg__l3_main_2,
};

static struct omap_hwmod omap44xx_l3_main_2_hwmod = {
	.name		= "l3_main_2",
	.class		= &omap44xx_l3_hwmod_class,
	.slaves		= omap44xx_l3_main_2_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_l3_main_2_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* l3_main_3 interface data */
/* l3_main_1 -> l3_main_3 */
static struct omap_hwmod_ocp_if omap44xx_l3_main_1__l3_main_3 = {
	.master		= &omap44xx_l3_main_1_hwmod,
	.slave		= &omap44xx_l3_main_3_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l3_main_2 -> l3_main_3 */
static struct omap_hwmod_ocp_if omap44xx_l3_main_2__l3_main_3 = {
	.master		= &omap44xx_l3_main_2_hwmod,
	.slave		= &omap44xx_l3_main_3_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l4_cfg -> l3_main_3 */
static struct omap_hwmod_ocp_if omap44xx_l4_cfg__l3_main_3 = {
	.master		= &omap44xx_l4_cfg_hwmod,
	.slave		= &omap44xx_l3_main_3_hwmod,
	.clk		= "l4_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l3_main_3 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_l3_main_3_slaves[] = {
	&omap44xx_l3_main_1__l3_main_3,
	&omap44xx_l3_main_2__l3_main_3,
	&omap44xx_l4_cfg__l3_main_3,
};

static struct omap_hwmod omap44xx_l3_main_3_hwmod = {
	.name		= "l3_main_3",
	.class		= &omap44xx_l3_hwmod_class,
	.slaves		= omap44xx_l3_main_3_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_l3_main_3_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/*
 * 'l4' class
 * instance(s): l4_abe, l4_cfg, l4_per, l4_wkup
 */
static struct omap_hwmod_class omap44xx_l4_hwmod_class = {
	.name = "l4",
};

/* l4_abe interface data */
/* l3_main_1 -> l4_abe */
static struct omap_hwmod_ocp_if omap44xx_l3_main_1__l4_abe = {
	.master		= &omap44xx_l3_main_1_hwmod,
	.slave		= &omap44xx_l4_abe_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* mpu -> l4_abe */
static struct omap_hwmod_ocp_if omap44xx_mpu__l4_abe = {
	.master		= &omap44xx_mpu_hwmod,
	.slave		= &omap44xx_l4_abe_hwmod,
	.clk		= "ocp_abe_iclk",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l4_abe slave ports */
static struct omap_hwmod_ocp_if *omap44xx_l4_abe_slaves[] = {
	&omap44xx_l3_main_1__l4_abe,
	&omap44xx_mpu__l4_abe,
};

static struct omap_hwmod omap44xx_l4_abe_hwmod = {
	.name		= "l4_abe",
	.class		= &omap44xx_l4_hwmod_class,
	.slaves		= omap44xx_l4_abe_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_l4_abe_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* l4_cfg interface data */
/* l3_main_1 -> l4_cfg */
static struct omap_hwmod_ocp_if omap44xx_l3_main_1__l4_cfg = {
	.master		= &omap44xx_l3_main_1_hwmod,
	.slave		= &omap44xx_l4_cfg_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l4_cfg slave ports */
static struct omap_hwmod_ocp_if *omap44xx_l4_cfg_slaves[] = {
	&omap44xx_l3_main_1__l4_cfg,
};

static struct omap_hwmod omap44xx_l4_cfg_hwmod = {
	.name		= "l4_cfg",
	.class		= &omap44xx_l4_hwmod_class,
	.slaves		= omap44xx_l4_cfg_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_l4_cfg_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* l4_per interface data */
/* l3_main_2 -> l4_per */
static struct omap_hwmod_ocp_if omap44xx_l3_main_2__l4_per = {
	.master		= &omap44xx_l3_main_2_hwmod,
	.slave		= &omap44xx_l4_per_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l4_per slave ports */
static struct omap_hwmod_ocp_if *omap44xx_l4_per_slaves[] = {
	&omap44xx_l3_main_2__l4_per,
};

static struct omap_hwmod omap44xx_l4_per_hwmod = {
	.name		= "l4_per",
	.class		= &omap44xx_l4_hwmod_class,
	.slaves		= omap44xx_l4_per_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_l4_per_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* l4_wkup interface data */
/* l4_cfg -> l4_wkup */
static struct omap_hwmod_ocp_if omap44xx_l4_cfg__l4_wkup = {
	.master		= &omap44xx_l4_cfg_hwmod,
	.slave		= &omap44xx_l4_wkup_hwmod,
	.clk		= "l4_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* l4_wkup slave ports */
static struct omap_hwmod_ocp_if *omap44xx_l4_wkup_slaves[] = {
	&omap44xx_l4_cfg__l4_wkup,
};

static struct omap_hwmod omap44xx_l4_wkup_hwmod = {
	.name		= "l4_wkup",
	.class		= &omap44xx_l4_hwmod_class,
	.slaves		= omap44xx_l4_wkup_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_l4_wkup_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/*
 * 'i2c' class
 * multimaster high-speed i2c controller
 */

static struct omap_hwmod_class_sysconfig omap44xx_i2c_sysc = {
	.sysc_offs	= 0x0010,
	.syss_offs	= 0x0090,
	.sysc_flags	= (SYSC_HAS_ENAWAKEUP | SYSC_HAS_SIDLEMODE |
			   SYSC_HAS_CLOCKACTIVITY | SYSC_HAS_SOFTRESET |
			   SYSC_HAS_AUTOIDLE),
	.idlemodes	= (SIDLE_FORCE | SIDLE_NO | SIDLE_SMART),
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class omap44xx_i2c_hwmod_class = {
	.name = "i2c",
	.sysc = &omap44xx_i2c_sysc,
};

/* i2c1 */
static struct omap_hwmod omap44xx_i2c1_hwmod;
static struct omap_hwmod_irq_info omap44xx_i2c1_irqs[] = {
	{ .irq = 56 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_dma_info omap44xx_i2c1_sdma_reqs[] = {
	{ .name = "tx", .dma_req = 26 + OMAP44XX_DMA_REQ_START },
	{ .name = "rx", .dma_req = 27 + OMAP44XX_DMA_REQ_START },
};

static struct omap_hwmod_addr_space omap44xx_i2c1_addrs[] = {
	{
		.pa_start	= 0x48070000,
		.pa_end		= 0x480700ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> i2c1 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__i2c1 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_i2c1_hwmod,
	.clk		= "l4_div_ck",
	.addr		= omap44xx_i2c1_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_i2c1_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* i2c1 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_i2c1_slaves[] = {
	&omap44xx_l4_per__i2c1,
};

static struct omap_hwmod omap44xx_i2c1_hwmod = {
	.name		= "i2c1",
	.class		= &omap44xx_i2c_hwmod_class,
	.flags		= HWMOD_INIT_NO_RESET,
	.mpu_irqs	= omap44xx_i2c1_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_i2c1_irqs),
	.sdma_reqs	= omap44xx_i2c1_sdma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(omap44xx_i2c1_sdma_reqs),
	.main_clk	= "i2c1_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_I2C1_CLKCTRL,
		},
	},
	.slaves		= omap44xx_i2c1_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_i2c1_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* i2c2 */
static struct omap_hwmod omap44xx_i2c2_hwmod;
static struct omap_hwmod_irq_info omap44xx_i2c2_irqs[] = {
	{ .irq = 57 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_dma_info omap44xx_i2c2_sdma_reqs[] = {
	{ .name = "tx", .dma_req = 28 + OMAP44XX_DMA_REQ_START },
	{ .name = "rx", .dma_req = 29 + OMAP44XX_DMA_REQ_START },
};

static struct omap_hwmod_addr_space omap44xx_i2c2_addrs[] = {
	{
		.pa_start	= 0x48072000,
		.pa_end		= 0x480720ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> i2c2 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__i2c2 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_i2c2_hwmod,
	.clk		= "l4_div_ck",
	.addr		= omap44xx_i2c2_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_i2c2_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* i2c2 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_i2c2_slaves[] = {
	&omap44xx_l4_per__i2c2,
};

static struct omap_hwmod omap44xx_i2c2_hwmod = {
	.name		= "i2c2",
	.class		= &omap44xx_i2c_hwmod_class,
	.flags		= HWMOD_INIT_NO_RESET,
	.mpu_irqs	= omap44xx_i2c2_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_i2c2_irqs),
	.sdma_reqs	= omap44xx_i2c2_sdma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(omap44xx_i2c2_sdma_reqs),
	.main_clk	= "i2c2_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_I2C2_CLKCTRL,
		},
	},
	.slaves		= omap44xx_i2c2_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_i2c2_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* i2c3 */
static struct omap_hwmod omap44xx_i2c3_hwmod;
static struct omap_hwmod_irq_info omap44xx_i2c3_irqs[] = {
	{ .irq = 61 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_dma_info omap44xx_i2c3_sdma_reqs[] = {
	{ .name = "tx", .dma_req = 24 + OMAP44XX_DMA_REQ_START },
	{ .name = "rx", .dma_req = 25 + OMAP44XX_DMA_REQ_START },
};

static struct omap_hwmod_addr_space omap44xx_i2c3_addrs[] = {
	{
		.pa_start	= 0x48060000,
		.pa_end		= 0x480600ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> i2c3 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__i2c3 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_i2c3_hwmod,
	.clk		= "l4_div_ck",
	.addr		= omap44xx_i2c3_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_i2c3_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* i2c3 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_i2c3_slaves[] = {
	&omap44xx_l4_per__i2c3,
};

static struct omap_hwmod omap44xx_i2c3_hwmod = {
	.name		= "i2c3",
	.class		= &omap44xx_i2c_hwmod_class,
	.flags		= HWMOD_INIT_NO_RESET,
	.mpu_irqs	= omap44xx_i2c3_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_i2c3_irqs),
	.sdma_reqs	= omap44xx_i2c3_sdma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(omap44xx_i2c3_sdma_reqs),
	.main_clk	= "i2c3_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_I2C3_CLKCTRL,
		},
	},
	.slaves		= omap44xx_i2c3_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_i2c3_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* i2c4 */
static struct omap_hwmod omap44xx_i2c4_hwmod;
static struct omap_hwmod_irq_info omap44xx_i2c4_irqs[] = {
	{ .irq = 62 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_dma_info omap44xx_i2c4_sdma_reqs[] = {
	{ .name = "tx", .dma_req = 123 + OMAP44XX_DMA_REQ_START },
	{ .name = "rx", .dma_req = 124 + OMAP44XX_DMA_REQ_START },
};

static struct omap_hwmod_addr_space omap44xx_i2c4_addrs[] = {
	{
		.pa_start	= 0x48350000,
		.pa_end		= 0x483500ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> i2c4 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__i2c4 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_i2c4_hwmod,
	.clk		= "l4_div_ck",
	.addr		= omap44xx_i2c4_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_i2c4_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* i2c4 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_i2c4_slaves[] = {
	&omap44xx_l4_per__i2c4,
};

static struct omap_hwmod omap44xx_i2c4_hwmod = {
	.name		= "i2c4",
	.class		= &omap44xx_i2c_hwmod_class,
	.flags		= HWMOD_INIT_NO_RESET,
	.mpu_irqs	= omap44xx_i2c4_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_i2c4_irqs),
	.sdma_reqs	= omap44xx_i2c4_sdma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(omap44xx_i2c4_sdma_reqs),
	.main_clk	= "i2c4_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_I2C4_CLKCTRL,
		},
	},
	.slaves		= omap44xx_i2c4_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_i2c4_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/*
 * 'mpu_bus' class
 * instance(s): mpu_private
 */
static struct omap_hwmod_class omap44xx_mpu_bus_hwmod_class = {
	.name = "mpu_bus",
};

/* mpu_private interface data */
/* mpu -> mpu_private */
static struct omap_hwmod_ocp_if omap44xx_mpu__mpu_private = {
	.master		= &omap44xx_mpu_hwmod,
	.slave		= &omap44xx_mpu_private_hwmod,
	.clk		= "l3_div_ck",
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* mpu_private slave ports */
static struct omap_hwmod_ocp_if *omap44xx_mpu_private_slaves[] = {
	&omap44xx_mpu__mpu_private,
};

static struct omap_hwmod omap44xx_mpu_private_hwmod = {
	.name		= "mpu_private",
	.class		= &omap44xx_mpu_bus_hwmod_class,
	.slaves		= omap44xx_mpu_private_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_mpu_private_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/*
 * 'mpu' class
 * mpu sub-system
 */

static struct omap_hwmod_class omap44xx_mpu_hwmod_class = {
	.name = "mpu",
};

/* mpu */
static struct omap_hwmod_irq_info omap44xx_mpu_irqs[] = {
	{ .name = "pl310", .irq = 0 + OMAP44XX_IRQ_GIC_START },
	{ .name = "cti0", .irq = 1 + OMAP44XX_IRQ_GIC_START },
	{ .name = "cti1", .irq = 2 + OMAP44XX_IRQ_GIC_START },
};

/* mpu master ports */
static struct omap_hwmod_ocp_if *omap44xx_mpu_masters[] = {
	&omap44xx_mpu__l3_main_1,
	&omap44xx_mpu__l4_abe,
	&omap44xx_mpu__dmm,
};

static struct omap_hwmod omap44xx_mpu_hwmod = {
	.name		= "mpu",
	.class		= &omap44xx_mpu_hwmod_class,
	.flags		= (HWMOD_INIT_NO_IDLE | HWMOD_INIT_NO_RESET),
	.mpu_irqs	= omap44xx_mpu_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_mpu_irqs),
	.main_clk	= "dpll_mpu_m2_ck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_MPU_MPU_CLKCTRL,
		},
	},
	.masters	= omap44xx_mpu_masters,
	.masters_cnt	= ARRAY_SIZE(omap44xx_mpu_masters),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/*
 * 'wd_timer' class
 * 32-bit watchdog upward counter that generates a pulse on the reset pin on
 * overflow condition
 */

static struct omap_hwmod_class_sysconfig omap44xx_wd_timer_sysc = {
	.rev_offs	= 0x0000,
	.sysc_offs	= 0x0010,
	.syss_offs	= 0x0014,
	.sysc_flags	= (SYSC_HAS_SIDLEMODE | SYSC_HAS_EMUFREE |
			   SYSC_HAS_SOFTRESET),
	.idlemodes	= (SIDLE_FORCE | SIDLE_NO | SIDLE_SMART),
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

/*
 * 'uart' class
 * universal asynchronous receiver/transmitter (uart)
 */

static struct omap_hwmod_class_sysconfig omap44xx_uart_sysc = {
	.rev_offs	= 0x0050,
	.sysc_offs	= 0x0054,
	.syss_offs	= 0x0058,
	.sysc_flags	= (SYSC_HAS_ENAWAKEUP | SYSC_HAS_SIDLEMODE |
			   SYSC_HAS_SOFTRESET | SYSC_HAS_AUTOIDLE),
	.idlemodes	= (SIDLE_FORCE | SIDLE_NO | SIDLE_SMART),
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class omap44xx_wd_timer_hwmod_class = {
	.name = "wd_timer",
	.sysc = &omap44xx_wd_timer_sysc,
};

/* wd_timer2 */
static struct omap_hwmod omap44xx_wd_timer2_hwmod;
static struct omap_hwmod_irq_info omap44xx_wd_timer2_irqs[] = {
	{ .irq = 80 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_addr_space omap44xx_wd_timer2_addrs[] = {
	{
		.pa_start	= 0x4a314000,
		.pa_end		= 0x4a31407f,
		.flags		= ADDR_TYPE_RT
	},
};

static struct omap_hwmod_class omap44xx_uart_hwmod_class = {
	.name = "uart",
	.sysc = &omap44xx_uart_sysc,
};

/* uart1 */
static struct omap_hwmod omap44xx_uart1_hwmod;
static struct omap_hwmod_irq_info omap44xx_uart1_irqs[] = {
	{ .irq = 72 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_dma_info omap44xx_uart1_sdma_reqs[] = {
	{ .name = "tx", .dma_req = 48 + OMAP44XX_DMA_REQ_START },
	{ .name = "rx", .dma_req = 49 + OMAP44XX_DMA_REQ_START },
};

static struct omap_hwmod_addr_space omap44xx_uart1_addrs[] = {
	{
		.pa_start	= 0x4806a000,
		.pa_end		= 0x4806a0ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> uart1 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__uart1 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_uart1_hwmod,
	.clk		= "l4_div_ck",
	.addr		= omap44xx_uart1_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_uart1_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* uart1 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_uart1_slaves[] = {
	&omap44xx_l4_per__uart1,
};

static struct omap_hwmod omap44xx_uart1_hwmod = {
	.name		= "uart1",
	.class		= &omap44xx_uart_hwmod_class,
	.mpu_irqs	= omap44xx_uart1_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_uart1_irqs),
	.sdma_reqs	= omap44xx_uart1_sdma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(omap44xx_uart1_sdma_reqs),
	.main_clk	= "uart1_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_UART1_CLKCTRL,
		},
	},
	.slaves		= omap44xx_uart1_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_uart1_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* uart2 */
static struct omap_hwmod omap44xx_uart2_hwmod;
static struct omap_hwmod_irq_info omap44xx_uart2_irqs[] = {
	{ .irq = 73 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_dma_info omap44xx_uart2_sdma_reqs[] = {
	{ .name = "tx", .dma_req = 50 + OMAP44XX_DMA_REQ_START },
	{ .name = "rx", .dma_req = 51 + OMAP44XX_DMA_REQ_START },
};

static struct omap_hwmod_addr_space omap44xx_uart2_addrs[] = {
	{
		.pa_start	= 0x4806c000,
		.pa_end		= 0x4806c0ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_wkup -> wd_timer2 */
static struct omap_hwmod_ocp_if omap44xx_l4_wkup__wd_timer2 = {
	.master		= &omap44xx_l4_wkup_hwmod,
	.slave		= &omap44xx_wd_timer2_hwmod,
	.clk		= "l4_wkup_clk_mux_ck",
	.addr		= omap44xx_wd_timer2_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_wd_timer2_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* wd_timer2 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_wd_timer2_slaves[] = {
	&omap44xx_l4_wkup__wd_timer2,
};

static struct omap_hwmod omap44xx_wd_timer2_hwmod = {
	.name		= "wd_timer2",
	.class		= &omap44xx_wd_timer_hwmod_class,
	.mpu_irqs	= omap44xx_wd_timer2_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_wd_timer2_irqs),
	.main_clk	= "wd_timer2_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_WKUP_WDT2_CLKCTRL,
		},
	},
	.slaves		= omap44xx_wd_timer2_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_wd_timer2_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* wd_timer3 */
static struct omap_hwmod omap44xx_wd_timer3_hwmod;
static struct omap_hwmod_irq_info omap44xx_wd_timer3_irqs[] = {
	{ .irq = 36 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_addr_space omap44xx_wd_timer3_addrs[] = {
	{
		.pa_start	= 0x40130000,
		.pa_end		= 0x4013007f,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> uart2 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__uart2 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_uart2_hwmod,
	.clk		= "l4_div_ck",
	.addr		= omap44xx_uart2_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_uart2_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* uart2 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_uart2_slaves[] = {
	&omap44xx_l4_per__uart2,
};

static struct omap_hwmod omap44xx_uart2_hwmod = {
	.name		= "uart2",
	.class		= &omap44xx_uart_hwmod_class,
	.mpu_irqs	= omap44xx_uart2_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_uart2_irqs),
	.sdma_reqs	= omap44xx_uart2_sdma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(omap44xx_uart2_sdma_reqs),
	.main_clk	= "uart2_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_UART2_CLKCTRL,
		},
	},
	.slaves		= omap44xx_uart2_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_uart2_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* uart3 */
static struct omap_hwmod omap44xx_uart3_hwmod;
static struct omap_hwmod_irq_info omap44xx_uart3_irqs[] = {
	{ .irq = 74 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_dma_info omap44xx_uart3_sdma_reqs[] = {
	{ .name = "tx", .dma_req = 52 + OMAP44XX_DMA_REQ_START },
	{ .name = "rx", .dma_req = 53 + OMAP44XX_DMA_REQ_START },
};

static struct omap_hwmod_addr_space omap44xx_uart3_addrs[] = {
	{
		.pa_start	= 0x48020000,
		.pa_end		= 0x480200ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_abe -> wd_timer3 */
static struct omap_hwmod_ocp_if omap44xx_l4_abe__wd_timer3 = {
	.master		= &omap44xx_l4_abe_hwmod,
	.slave		= &omap44xx_wd_timer3_hwmod,
	.clk		= "ocp_abe_iclk",
	.addr		= omap44xx_wd_timer3_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_wd_timer3_addrs),
	.user		= OCP_USER_MPU,
};

/* l4_abe -> wd_timer3 (dma) */
static struct omap_hwmod_addr_space omap44xx_wd_timer3_dma_addrs[] = {
	{
		.pa_start	= 0x49030000,
		.pa_end		= 0x4903007f,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> uart3 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__uart3 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_uart3_hwmod,
	.clk		= "l4_div_ck",
	.addr		= omap44xx_uart3_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_uart3_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* uart3 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_uart3_slaves[] = {
	&omap44xx_l4_per__uart3,
};

static struct omap_hwmod omap44xx_uart3_hwmod = {
	.name		= "uart3",
	.class		= &omap44xx_uart_hwmod_class,
	.flags		= (HWMOD_INIT_NO_IDLE | HWMOD_INIT_NO_RESET),
	.mpu_irqs	= omap44xx_uart3_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_uart3_irqs),
	.sdma_reqs	= omap44xx_uart3_sdma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(omap44xx_uart3_sdma_reqs),
	.main_clk	= "uart3_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_UART3_CLKCTRL,
		},
	},
	.slaves		= omap44xx_uart3_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_uart3_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* uart4 */
static struct omap_hwmod omap44xx_uart4_hwmod;
static struct omap_hwmod_irq_info omap44xx_uart4_irqs[] = {
	{ .irq = 70 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_dma_info omap44xx_uart4_sdma_reqs[] = {
	{ .name = "tx", .dma_req = 54 + OMAP44XX_DMA_REQ_START },
	{ .name = "rx", .dma_req = 55 + OMAP44XX_DMA_REQ_START },
};

static struct omap_hwmod_addr_space omap44xx_uart4_addrs[] = {
	{
		.pa_start	= 0x4806e000,
		.pa_end		= 0x4806e0ff,
		.flags		= ADDR_TYPE_RT
	},
};

static struct omap_hwmod_ocp_if omap44xx_l4_abe__wd_timer3_dma = {
	.master		= &omap44xx_l4_abe_hwmod,
	.slave		= &omap44xx_wd_timer3_hwmod,
	.clk		= "ocp_abe_iclk",
	.addr		= omap44xx_wd_timer3_dma_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_wd_timer3_dma_addrs),
	.user		= OCP_USER_SDMA,
};

/* wd_timer3 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_wd_timer3_slaves[] = {
	&omap44xx_l4_abe__wd_timer3,
	&omap44xx_l4_abe__wd_timer3_dma,
};

static struct omap_hwmod omap44xx_wd_timer3_hwmod = {
	.name		= "wd_timer3",
	.class		= &omap44xx_wd_timer_hwmod_class,
	.mpu_irqs	= omap44xx_wd_timer3_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_wd_timer3_irqs),
	.main_clk	= "wd_timer3_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM1_ABE_WDT3_CLKCTRL,
		},
	},
	.slaves		= omap44xx_wd_timer3_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_wd_timer3_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* l4_per -> uart4 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__uart4 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_uart4_hwmod,
	.clk		= "l4_div_ck",
	.addr		= omap44xx_uart4_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_uart4_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* uart4 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_uart4_slaves[] = {
	&omap44xx_l4_per__uart4,
};

static struct omap_hwmod omap44xx_uart4_hwmod = {
	.name		= "uart4",
	.class		= &omap44xx_uart_hwmod_class,
	.mpu_irqs	= omap44xx_uart4_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_uart4_irqs),
	.sdma_reqs	= omap44xx_uart4_sdma_reqs,
	.sdma_reqs_cnt	= ARRAY_SIZE(omap44xx_uart4_sdma_reqs),
	.main_clk	= "uart4_fck",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_UART4_CLKCTRL,
		},
	},
	.slaves		= omap44xx_uart4_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_uart4_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/*
 * 'gpio' class
 * general purpose io module
 */

static struct omap_hwmod_class_sysconfig omap44xx_gpio_sysc = {
	.rev_offs	= 0x0000,
	.sysc_offs	= 0x0010,
	.syss_offs	= 0x0114,
	.sysc_flags	= (SYSC_HAS_ENAWAKEUP | SYSC_HAS_SIDLEMODE |
			   SYSC_HAS_SOFTRESET | SYSC_HAS_AUTOIDLE),
	.idlemodes	= (SIDLE_FORCE | SIDLE_NO | SIDLE_SMART),
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class omap44xx_gpio_hwmod_class = {
	.name = "gpio",
	.sysc = &omap44xx_gpio_sysc,
	.rev = 2,
};

/* gpio dev_attr */
static struct omap_gpio_dev_attr gpio_dev_attr = {
	.bank_width = 32,
	.dbck_flag = true,
};

/* gpio1 */
static struct omap_hwmod omap44xx_gpio1_hwmod;
static struct omap_hwmod_irq_info omap44xx_gpio1_irqs[] = {
	{ .irq = 29 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_addr_space omap44xx_gpio1_addrs[] = {
	{
		.pa_start	= 0x4a310000,
		.pa_end		= 0x4a3101ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_wkup -> gpio1 */
static struct omap_hwmod_ocp_if omap44xx_l4_wkup__gpio1 = {
	.master		= &omap44xx_l4_wkup_hwmod,
	.slave		= &omap44xx_gpio1_hwmod,
	.addr		= omap44xx_gpio1_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_gpio1_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* gpio1 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_gpio1_slaves[] = {
	&omap44xx_l4_wkup__gpio1,
};

static struct omap_hwmod_opt_clk gpio1_opt_clks[] = {
	{ .role = "dbclk", .clk = "sys_32k_ck" },
};

static struct omap_hwmod omap44xx_gpio1_hwmod = {
	.name		= "gpio1",
	.class		= &omap44xx_gpio_hwmod_class,
	.mpu_irqs	= omap44xx_gpio1_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_gpio1_irqs),
	.main_clk	= "gpio1_ick",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_WKUP_GPIO1_CLKCTRL,
		},
	},
	.opt_clks	= gpio1_opt_clks,
	.opt_clks_cnt = ARRAY_SIZE(gpio1_opt_clks),
	.dev_attr	= &gpio_dev_attr,
	.slaves		= omap44xx_gpio1_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_gpio1_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* gpio2 */
static struct omap_hwmod omap44xx_gpio2_hwmod;
static struct omap_hwmod_irq_info omap44xx_gpio2_irqs[] = {
	{ .irq = 30 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_addr_space omap44xx_gpio2_addrs[] = {
	{
		.pa_start	= 0x48055000,
		.pa_end		= 0x480551ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> gpio2 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__gpio2 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_gpio2_hwmod,
	.addr		= omap44xx_gpio2_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_gpio2_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* gpio2 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_gpio2_slaves[] = {
	&omap44xx_l4_per__gpio2,
};

static struct omap_hwmod_opt_clk gpio2_opt_clks[] = {
	{ .role = "dbclk", .clk = "sys_32k_ck" },
};

static struct omap_hwmod omap44xx_gpio2_hwmod = {
	.name		= "gpio2",
	.class		= &omap44xx_gpio_hwmod_class,
	.mpu_irqs	= omap44xx_gpio2_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_gpio2_irqs),
	.main_clk	= "gpio2_ick",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_GPIO2_CLKCTRL,
		},
	},
	.opt_clks	= gpio2_opt_clks,
	.opt_clks_cnt = ARRAY_SIZE(gpio2_opt_clks),
	.dev_attr	= &gpio_dev_attr,
	.slaves		= omap44xx_gpio2_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_gpio2_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* gpio3 */
static struct omap_hwmod omap44xx_gpio3_hwmod;
static struct omap_hwmod_irq_info omap44xx_gpio3_irqs[] = {
	{ .irq = 31 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_addr_space omap44xx_gpio3_addrs[] = {
	{
		.pa_start	= 0x48057000,
		.pa_end		= 0x480571ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> gpio3 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__gpio3 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_gpio3_hwmod,
	.addr		= omap44xx_gpio3_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_gpio3_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* gpio3 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_gpio3_slaves[] = {
	&omap44xx_l4_per__gpio3,
};

static struct omap_hwmod_opt_clk gpio3_opt_clks[] = {
	{ .role = "dbclk", .clk = "sys_32k_ck" },
};

static struct omap_hwmod omap44xx_gpio3_hwmod = {
	.name		= "gpio3",
	.class		= &omap44xx_gpio_hwmod_class,
	.mpu_irqs	= omap44xx_gpio3_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_gpio3_irqs),
	.main_clk	= "gpio3_ick",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_GPIO3_CLKCTRL,
		},
	},
	.opt_clks	= gpio3_opt_clks,
	.opt_clks_cnt = ARRAY_SIZE(gpio3_opt_clks),
	.dev_attr	= &gpio_dev_attr,
	.slaves		= omap44xx_gpio3_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_gpio3_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* gpio4 */
static struct omap_hwmod omap44xx_gpio4_hwmod;
static struct omap_hwmod_irq_info omap44xx_gpio4_irqs[] = {
	{ .irq = 32 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_addr_space omap44xx_gpio4_addrs[] = {
	{
		.pa_start	= 0x48059000,
		.pa_end		= 0x480591ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> gpio4 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__gpio4 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_gpio4_hwmod,
	.addr		= omap44xx_gpio4_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_gpio4_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* gpio4 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_gpio4_slaves[] = {
	&omap44xx_l4_per__gpio4,
};

static struct omap_hwmod_opt_clk gpio4_opt_clks[] = {
	{ .role = "dbclk", .clk = "sys_32k_ck" },
};

static struct omap_hwmod omap44xx_gpio4_hwmod = {
	.name		= "gpio4",
	.class		= &omap44xx_gpio_hwmod_class,
	.mpu_irqs	= omap44xx_gpio4_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_gpio4_irqs),
	.main_clk	= "gpio4_ick",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_GPIO4_CLKCTRL,
		},
	},
	.opt_clks	= gpio4_opt_clks,
	.opt_clks_cnt = ARRAY_SIZE(gpio4_opt_clks),
	.dev_attr	= &gpio_dev_attr,
	.slaves		= omap44xx_gpio4_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_gpio4_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* gpio5 */
static struct omap_hwmod omap44xx_gpio5_hwmod;
static struct omap_hwmod_irq_info omap44xx_gpio5_irqs[] = {
	{ .irq = 33 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_addr_space omap44xx_gpio5_addrs[] = {
	{
		.pa_start	= 0x4805b000,
		.pa_end		= 0x4805b1ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> gpio5 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__gpio5 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_gpio5_hwmod,
	.addr		= omap44xx_gpio5_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_gpio5_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* gpio5 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_gpio5_slaves[] = {
	&omap44xx_l4_per__gpio5,
};

static struct omap_hwmod_opt_clk gpio5_opt_clks[] = {
	{ .role = "dbclk", .clk = "sys_32k_ck" },
};

static struct omap_hwmod omap44xx_gpio5_hwmod = {
	.name		= "gpio5",
	.class		= &omap44xx_gpio_hwmod_class,
	.mpu_irqs	= omap44xx_gpio5_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_gpio5_irqs),
	.main_clk	= "gpio5_ick",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_GPIO5_CLKCTRL,
		},
	},
	.opt_clks	= gpio5_opt_clks,
	.opt_clks_cnt = ARRAY_SIZE(gpio5_opt_clks),
	.dev_attr	= &gpio_dev_attr,
	.slaves		= omap44xx_gpio5_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_gpio5_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};

/* gpio6 */
static struct omap_hwmod omap44xx_gpio6_hwmod;
static struct omap_hwmod_irq_info omap44xx_gpio6_irqs[] = {
	{ .irq = 34 + OMAP44XX_IRQ_GIC_START },
};

static struct omap_hwmod_addr_space omap44xx_gpio6_addrs[] = {
	{
		.pa_start	= 0x4805d000,
		.pa_end		= 0x4805d1ff,
		.flags		= ADDR_TYPE_RT
	},
};

/* l4_per -> gpio6 */
static struct omap_hwmod_ocp_if omap44xx_l4_per__gpio6 = {
	.master		= &omap44xx_l4_per_hwmod,
	.slave		= &omap44xx_gpio6_hwmod,
	.addr		= omap44xx_gpio6_addrs,
	.addr_cnt	= ARRAY_SIZE(omap44xx_gpio6_addrs),
	.user		= OCP_USER_MPU | OCP_USER_SDMA,
};

/* gpio6 slave ports */
static struct omap_hwmod_ocp_if *omap44xx_gpio6_slaves[] = {
	&omap44xx_l4_per__gpio6,
};

static struct omap_hwmod_opt_clk gpio6_opt_clks[] = {
	{ .role = "dbclk", .clk = "sys_32k_ck" },
};

static struct omap_hwmod omap44xx_gpio6_hwmod = {
	.name		= "gpio6",
	.class		= &omap44xx_gpio_hwmod_class,
	.mpu_irqs	= omap44xx_gpio6_irqs,
	.mpu_irqs_cnt	= ARRAY_SIZE(omap44xx_gpio6_irqs),
	.main_clk	= "gpio6_ick",
	.prcm = {
		.omap4 = {
			.clkctrl_reg = OMAP4430_CM_L4PER_GPIO6_CLKCTRL,
		},
	},
	.opt_clks	= gpio6_opt_clks,
	.opt_clks_cnt = ARRAY_SIZE(gpio6_opt_clks),
	.dev_attr	= &gpio_dev_attr,
	.slaves		= omap44xx_gpio6_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap44xx_gpio6_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP4430),
};
static __initdata struct omap_hwmod *omap44xx_hwmods[] = {
	/* dmm class */
	&omap44xx_dmm_hwmod,
	/* emif_fw class */
	&omap44xx_emif_fw_hwmod,
	/* l3 class */
	&omap44xx_l3_instr_hwmod,
	&omap44xx_l3_main_1_hwmod,
	&omap44xx_l3_main_2_hwmod,
	&omap44xx_l3_main_3_hwmod,
	/* l4 class */
	&omap44xx_l4_abe_hwmod,
	&omap44xx_l4_cfg_hwmod,
	&omap44xx_l4_per_hwmod,
	&omap44xx_l4_wkup_hwmod,
	/* i2c class */
	&omap44xx_i2c1_hwmod,
	&omap44xx_i2c2_hwmod,
	&omap44xx_i2c3_hwmod,
	&omap44xx_i2c4_hwmod,
	/* mpu_bus class */
	&omap44xx_mpu_private_hwmod,

	/* gpio class */
	&omap44xx_gpio1_hwmod,
	&omap44xx_gpio2_hwmod,
	&omap44xx_gpio3_hwmod,
	&omap44xx_gpio4_hwmod,
	&omap44xx_gpio5_hwmod,
	&omap44xx_gpio6_hwmod,

	/* mpu class */
	&omap44xx_mpu_hwmod,
	/* wd_timer class */
	&omap44xx_wd_timer2_hwmod,
	&omap44xx_wd_timer3_hwmod,

	/* uart class */
	&omap44xx_uart1_hwmod,
	&omap44xx_uart2_hwmod,
	&omap44xx_uart3_hwmod,
	&omap44xx_uart4_hwmod,
	NULL,
};

int __init omap44xx_hwmod_init(void)
{
	return omap_hwmod_init(omap44xx_hwmods);
}

