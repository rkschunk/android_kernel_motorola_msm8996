/*
 *  linux/arch/arm/mach-clps711x/autcpu12.c
 *
 * (c) 2001 Thomas Gleixner, autronix automation <gleixner@autronix.de>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>

#include <mach/hardware.h>
#include <asm/sizes.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/pgtable.h>
#include <asm/page.h>

#include <asm/mach/map.h>
#include <mach/autcpu12.h>

#include "common.h"

static struct map_desc autcpu12_io_desc[] __initdata = {
	/* Memory-mapped extra io and CS8900A Ethernet chip */
	{
		.virtual	= IO_ADDRESS(AUTCPU12_PHYS_CS8900A),
		.pfn		= __phys_to_pfn(AUTCPU12_PHYS_CS8900A),
		.length		= SZ_1M,
		.type		= MT_DEVICE
	}
};

void __init autcpu12_map_io(void)
{
        clps711x_map_io();
        iotable_init(autcpu12_io_desc, ARRAY_SIZE(autcpu12_io_desc));
}

static struct resource autcpu12_nvram_resource[] __initdata = {
	DEFINE_RES_MEM_NAMED(AUTCPU12_PHYS_NVRAM, SZ_128K, "SRAM"),
};

static struct platform_device autcpu12_nvram_pdev __initdata = {
	.name		= "autcpu12_nvram",
	.id		= -1,
	.resource	= autcpu12_nvram_resource,
	.num_resources	= ARRAY_SIZE(autcpu12_nvram_resource),
};

static void __init autcpu12_init(void)
{
	platform_device_register(&autcpu12_nvram_pdev);
}

MACHINE_START(AUTCPU12, "autronix autcpu12")
	/* Maintainer: Thomas Gleixner */
	.atag_offset	= 0x20000,
	.init_machine	= autcpu12_init,
	.map_io		= autcpu12_map_io,
	.init_irq	= clps711x_init_irq,
	.timer		= &clps711x_timer,
	.restart	= clps711x_restart,
MACHINE_END

