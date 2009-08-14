/*
 * KFR2R09 board support code
 *
 * Copyright (C) 2009 Magnus Damm
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/onenand.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <video/sh_mobile_lcdc.h>
#include <asm/clock.h>
#include <asm/machvec.h>
#include <asm/io.h>
#include <asm/sh_keysc.h>
#include <cpu/sh7724.h>
#include <mach/kfr2r09.h>

static struct mtd_partition kfr2r09_nor_flash_partitions[] =
{
	{
		.name = "boot",
		.offset = 0,
		.size = (4 * 1024 * 1024),
		.mask_flags = MTD_WRITEABLE,	/* Read-only */
	},
	{
		.name = "other",
		.offset = MTDPART_OFS_APPEND,
		.size = MTDPART_SIZ_FULL,
	},
};

static struct physmap_flash_data kfr2r09_nor_flash_data = {
	.width		= 2,
	.parts		= kfr2r09_nor_flash_partitions,
	.nr_parts	= ARRAY_SIZE(kfr2r09_nor_flash_partitions),
};

static struct resource kfr2r09_nor_flash_resources[] = {
	[0] = {
		.name		= "NOR Flash",
		.start		= 0x00000000,
		.end		= 0x03ffffff,
		.flags		= IORESOURCE_MEM,
	}
};

static struct platform_device kfr2r09_nor_flash_device = {
	.name		= "physmap-flash",
	.resource	= kfr2r09_nor_flash_resources,
	.num_resources	= ARRAY_SIZE(kfr2r09_nor_flash_resources),
	.dev		= {
		.platform_data = &kfr2r09_nor_flash_data,
	},
};

static struct resource kfr2r09_nand_flash_resources[] = {
	[0] = {
		.name		= "NAND Flash",
		.start		= 0x10000000,
		.end		= 0x1001ffff,
		.flags		= IORESOURCE_MEM,
	}
};

static struct platform_device kfr2r09_nand_flash_device = {
	.name		= "onenand-flash",
	.resource	= kfr2r09_nand_flash_resources,
	.num_resources	= ARRAY_SIZE(kfr2r09_nand_flash_resources),
};

static struct sh_keysc_info kfr2r09_sh_keysc_info = {
	.mode = SH_KEYSC_MODE_1, /* KEYOUT0->4, KEYIN0->4 */
	.scan_timing = 3,
	.delay = 10,
	.keycodes = {
		KEY_PHONE, KEY_CLEAR, KEY_MAIL, KEY_WWW, KEY_ENTER,
		KEY_1, KEY_2, KEY_3, 0, KEY_UP,
		KEY_4, KEY_5, KEY_6, 0, KEY_LEFT,
		KEY_7, KEY_8, KEY_9, KEY_PROG1, KEY_RIGHT,
		KEY_S, KEY_0, KEY_P, KEY_PROG2, KEY_DOWN,
		0, 0, 0, 0, 0
	},
};

static struct resource kfr2r09_sh_keysc_resources[] = {
	[0] = {
		.name	= "KEYSC",
		.start  = 0x044b0000,
		.end    = 0x044b000f,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = 79,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device kfr2r09_sh_keysc_device = {
	.name           = "sh_keysc",
	.id             = 0, /* "keysc0" clock */
	.num_resources  = ARRAY_SIZE(kfr2r09_sh_keysc_resources),
	.resource       = kfr2r09_sh_keysc_resources,
	.dev	= {
		.platform_data	= &kfr2r09_sh_keysc_info,
	},
};

static struct sh_mobile_lcdc_info kfr2r09_sh_lcdc_info = {
	.clock_source = LCDC_CLK_BUS,
	.ch[0] = {
		.chan = LCDC_CHAN_MAINLCD,
		.bpp = 16,
		.interface_type = SYS18,
		.clock_divider = 6,
		.flags = LCDC_FLAGS_DWPOL,
		.lcd_cfg = {
			.name = "TX07D34VM0AAA",
			.xres = 240,
			.yres = 400,
			.left_margin = 0,
			.right_margin = 16,
			.hsync_len = 8,
			.upper_margin = 0,
			.lower_margin = 1,
			.vsync_len = 1,
			.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		},
		.lcd_size_cfg = {
			.width = 35,
			.height = 58,
		},
		.board_cfg = {
			.setup_sys = kfr2r09_lcd_setup,
			.display_on = kfr2r09_lcd_on,
			.display_off = kfr2r09_lcd_off,
		},
		.sys_bus_cfg = {
			.ldmt2r = 0x07010904,
			.ldmt3r = 0x14012914,
			/* set 1s delay to encourage fsync() */
			.deferred_io_msec = 1000,
		},
	}
};

static struct resource kfr2r09_sh_lcdc_resources[] = {
	[0] = {
		.name	= "LCDC",
		.start	= 0xfe940000, /* P4-only space */
		.end	= 0xfe941fff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 106,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device kfr2r09_sh_lcdc_device = {
	.name		= "sh_mobile_lcdc_fb",
	.num_resources	= ARRAY_SIZE(kfr2r09_sh_lcdc_resources),
	.resource	= kfr2r09_sh_lcdc_resources,
	.dev	= {
		.platform_data	= &kfr2r09_sh_lcdc_info,
	},
};

static struct platform_device *kfr2r09_devices[] __initdata = {
	&kfr2r09_nor_flash_device,
	&kfr2r09_nand_flash_device,
	&kfr2r09_sh_keysc_device,
	&kfr2r09_sh_lcdc_device,
};

#define BSC_CS0BCR 0xfec10004
#define BSC_CS0WCR 0xfec10024
#define BSC_CS4BCR 0xfec10010
#define BSC_CS4WCR 0xfec10030

static int __init kfr2r09_devices_setup(void)
{
	/* enable SCIF1 serial port for YC401 console support */
	gpio_request(GPIO_FN_SCIF1_RXD, NULL);
	gpio_request(GPIO_FN_SCIF1_TXD, NULL);

	/* setup NOR flash at CS0 */
	ctrl_outl(0x36db0400, BSC_CS0BCR);
	ctrl_outl(0x00000500, BSC_CS0WCR);

	/* setup NAND flash at CS4 */
	ctrl_outl(0x36db0400, BSC_CS4BCR);
	ctrl_outl(0x00000500, BSC_CS4WCR);

	/* setup KEYSC pins */
	gpio_request(GPIO_FN_KEYOUT0, NULL);
	gpio_request(GPIO_FN_KEYOUT1, NULL);
	gpio_request(GPIO_FN_KEYOUT2, NULL);
	gpio_request(GPIO_FN_KEYOUT3, NULL);
	gpio_request(GPIO_FN_KEYOUT4_IN6, NULL);
	gpio_request(GPIO_FN_KEYIN0, NULL);
	gpio_request(GPIO_FN_KEYIN1, NULL);
	gpio_request(GPIO_FN_KEYIN2, NULL);
	gpio_request(GPIO_FN_KEYIN3, NULL);
	gpio_request(GPIO_FN_KEYIN4, NULL);
	gpio_request(GPIO_FN_KEYOUT5_IN5, NULL);

	/* setup LCDC pins for SYS panel */
	gpio_request(GPIO_FN_LCDD17, NULL);
	gpio_request(GPIO_FN_LCDD16, NULL);
	gpio_request(GPIO_FN_LCDD15, NULL);
	gpio_request(GPIO_FN_LCDD14, NULL);
	gpio_request(GPIO_FN_LCDD13, NULL);
	gpio_request(GPIO_FN_LCDD12, NULL);
	gpio_request(GPIO_FN_LCDD11, NULL);
	gpio_request(GPIO_FN_LCDD10, NULL);
	gpio_request(GPIO_FN_LCDD9, NULL);
	gpio_request(GPIO_FN_LCDD8, NULL);
	gpio_request(GPIO_FN_LCDD7, NULL);
	gpio_request(GPIO_FN_LCDD6, NULL);
	gpio_request(GPIO_FN_LCDD5, NULL);
	gpio_request(GPIO_FN_LCDD4, NULL);
	gpio_request(GPIO_FN_LCDD3, NULL);
	gpio_request(GPIO_FN_LCDD2, NULL);
	gpio_request(GPIO_FN_LCDD1, NULL);
	gpio_request(GPIO_FN_LCDD0, NULL);
	gpio_request(GPIO_FN_LCDRS, NULL); /* LCD_RS */
	gpio_request(GPIO_FN_LCDCS, NULL); /* LCD_CS/ */
	gpio_request(GPIO_FN_LCDRD, NULL); /* LCD_RD/ */
	gpio_request(GPIO_FN_LCDWR, NULL); /* LCD_WR/ */
	gpio_request(GPIO_FN_LCDVSYN, NULL); /* LCD_VSYNC */
	gpio_request(GPIO_PTE4, NULL); /* LCD_RST/ */
	gpio_direction_output(GPIO_PTE4, 1);
	gpio_request(GPIO_PTF4, NULL); /* PROTECT/ */
	gpio_direction_output(GPIO_PTF4, 1);
	gpio_request(GPIO_PTU0, NULL); /* LEDSTDBY/ */
	gpio_direction_output(GPIO_PTU0, 1);

	return platform_add_devices(kfr2r09_devices,
				    ARRAY_SIZE(kfr2r09_devices));
}
device_initcall(kfr2r09_devices_setup);

/* Return the board specific boot mode pin configuration */
static int kfr2r09_mode_pins(void)
{
	/* MD0=1, MD1=1, MD2=0: Clock Mode 3
	 * MD3=0: 16-bit Area0 Bus Width
	 * MD5=1: Little Endian
	 * MD8=1: Test Mode Disabled
	 */
	return MODE_PIN0 | MODE_PIN1 | MODE_PIN5 | MODE_PIN8;
}

/*
 * The Machine Vector
 */
static struct sh_machine_vector mv_kfr2r09 __initmv = {
	.mv_name		= "kfr2r09",
	.mv_mode_pins		= kfr2r09_mode_pins,
};
