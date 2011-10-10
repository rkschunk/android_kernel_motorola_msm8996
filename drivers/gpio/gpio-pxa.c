/*
 *  linux/arch/arm/plat-pxa/gpio.c
 *
 *  Generic PXA GPIO handling
 *
 *  Author:	Nicolas Pitre
 *  Created:	Jun 15, 2001
 *  Copyright:	MontaVista Software Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/syscore_ops.h>
#include <linux/slab.h>

#include <mach/gpio-pxa.h>

int pxa_last_gpio;

struct pxa_gpio_chip {
	struct gpio_chip chip;
	void __iomem	*regbase;
	char label[10];

	unsigned long	irq_mask;
	unsigned long	irq_edge_rise;
	unsigned long	irq_edge_fall;

#ifdef CONFIG_PM
	unsigned long	saved_gplr;
	unsigned long	saved_gpdr;
	unsigned long	saved_grer;
	unsigned long	saved_gfer;
#endif
};

static DEFINE_SPINLOCK(gpio_lock);
static struct pxa_gpio_chip *pxa_gpio_chips;

#define for_each_gpio_chip(i, c)			\
	for (i = 0, c = &pxa_gpio_chips[0]; i <= pxa_last_gpio; i += 32, c++)

static inline void __iomem *gpio_chip_base(struct gpio_chip *c)
{
	return container_of(c, struct pxa_gpio_chip, chip)->regbase;
}

static inline struct pxa_gpio_chip *gpio_to_pxachip(unsigned gpio)
{
	return &pxa_gpio_chips[gpio_to_bank(gpio)];
}

static int pxa_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	void __iomem *base = gpio_chip_base(chip);
	uint32_t value, mask = 1 << offset;
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);

	value = __raw_readl(base + GPDR_OFFSET);
	if (__gpio_is_inverted(chip->base + offset))
		value |= mask;
	else
		value &= ~mask;
	__raw_writel(value, base + GPDR_OFFSET);

	spin_unlock_irqrestore(&gpio_lock, flags);
	return 0;
}

static int pxa_gpio_direction_output(struct gpio_chip *chip,
				     unsigned offset, int value)
{
	void __iomem *base = gpio_chip_base(chip);
	uint32_t tmp, mask = 1 << offset;
	unsigned long flags;

	__raw_writel(mask, base + (value ? GPSR_OFFSET : GPCR_OFFSET));

	spin_lock_irqsave(&gpio_lock, flags);

	tmp = __raw_readl(base + GPDR_OFFSET);
	if (__gpio_is_inverted(chip->base + offset))
		tmp &= ~mask;
	else
		tmp |= mask;
	__raw_writel(tmp, base + GPDR_OFFSET);

	spin_unlock_irqrestore(&gpio_lock, flags);
	return 0;
}

static int pxa_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	return __raw_readl(gpio_chip_base(chip) + GPLR_OFFSET) & (1 << offset);
}

static void pxa_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	__raw_writel(1 << offset, gpio_chip_base(chip) +
				(value ? GPSR_OFFSET : GPCR_OFFSET));
}

static int __init pxa_init_gpio_chip(int gpio_end)
{
	int i, gpio, nbanks = gpio_to_bank(gpio_end) + 1;
	struct pxa_gpio_chip *chips;

	chips = kzalloc(nbanks * sizeof(struct pxa_gpio_chip), GFP_KERNEL);
	if (chips == NULL) {
		pr_err("%s: failed to allocate GPIO chips\n", __func__);
		return -ENOMEM;
	}

	for (i = 0, gpio = 0; i < nbanks; i++, gpio += 32) {
		struct gpio_chip *c = &chips[i].chip;

		sprintf(chips[i].label, "gpio-%d", i);
		chips[i].regbase = GPIO_BANK(i);

		c->base  = gpio;
		c->label = chips[i].label;

		c->direction_input  = pxa_gpio_direction_input;
		c->direction_output = pxa_gpio_direction_output;
		c->get = pxa_gpio_get;
		c->set = pxa_gpio_set;

		/* number of GPIOs on last bank may be less than 32 */
		c->ngpio = (gpio + 31 > gpio_end) ? (gpio_end - gpio + 1) : 32;
		gpiochip_add(c);
	}
	pxa_gpio_chips = chips;
	return 0;
}

/* Update only those GRERx and GFERx edge detection register bits if those
 * bits are set in c->irq_mask
 */
static inline void update_edge_detect(struct pxa_gpio_chip *c)
{
	uint32_t grer, gfer;

	grer = __raw_readl(c->regbase + GRER_OFFSET) & ~c->irq_mask;
	gfer = __raw_readl(c->regbase + GFER_OFFSET) & ~c->irq_mask;
	grer |= c->irq_edge_rise & c->irq_mask;
	gfer |= c->irq_edge_fall & c->irq_mask;
	__raw_writel(grer, c->regbase + GRER_OFFSET);
	__raw_writel(gfer, c->regbase + GFER_OFFSET);
}

static int pxa_gpio_irq_type(struct irq_data *d, unsigned int type)
{
	struct pxa_gpio_chip *c;
	int gpio = irq_to_gpio(d->irq);
	unsigned long gpdr, mask = GPIO_bit(gpio);

	c = gpio_to_pxachip(gpio);

	if (type == IRQ_TYPE_PROBE) {
		/* Don't mess with enabled GPIOs using preconfigured edges or
		 * GPIOs set to alternate function or to output during probe
		 */
		if ((c->irq_edge_rise | c->irq_edge_fall) & GPIO_bit(gpio))
			return 0;

		if (__gpio_is_occupied(gpio))
			return 0;

		type = IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING;
	}

	gpdr = __raw_readl(c->regbase + GPDR_OFFSET);

	if (__gpio_is_inverted(gpio))
		__raw_writel(gpdr | mask,  c->regbase + GPDR_OFFSET);
	else
		__raw_writel(gpdr & ~mask, c->regbase + GPDR_OFFSET);

	if (type & IRQ_TYPE_EDGE_RISING)
		c->irq_edge_rise |= mask;
	else
		c->irq_edge_rise &= ~mask;

	if (type & IRQ_TYPE_EDGE_FALLING)
		c->irq_edge_fall |= mask;
	else
		c->irq_edge_fall &= ~mask;

	update_edge_detect(c);

	pr_debug("%s: IRQ%d (GPIO%d) - edge%s%s\n", __func__, d->irq, gpio,
		((type & IRQ_TYPE_EDGE_RISING)  ? " rising"  : ""),
		((type & IRQ_TYPE_EDGE_FALLING) ? " falling" : ""));
	return 0;
}

static void pxa_gpio_demux_handler(unsigned int irq, struct irq_desc *desc)
{
	struct pxa_gpio_chip *c;
	int loop, gpio, gpio_base, n;
	unsigned long gedr;

	do {
		loop = 0;
		for_each_gpio_chip(gpio, c) {
			gpio_base = c->chip.base;

			gedr = __raw_readl(c->regbase + GEDR_OFFSET);
			gedr = gedr & c->irq_mask;
			__raw_writel(gedr, c->regbase + GEDR_OFFSET);

			n = find_first_bit(&gedr, BITS_PER_LONG);
			while (n < BITS_PER_LONG) {
				loop = 1;

				generic_handle_irq(gpio_to_irq(gpio_base + n));
				n = find_next_bit(&gedr, BITS_PER_LONG, n + 1);
			}
		}
	} while (loop);
}

static void pxa_ack_muxed_gpio(struct irq_data *d)
{
	int gpio = irq_to_gpio(d->irq);
	struct pxa_gpio_chip *c = gpio_to_pxachip(gpio);

	__raw_writel(GPIO_bit(gpio), c->regbase + GEDR_OFFSET);
}

static void pxa_mask_muxed_gpio(struct irq_data *d)
{
	int gpio = irq_to_gpio(d->irq);
	struct pxa_gpio_chip *c = gpio_to_pxachip(gpio);
	uint32_t grer, gfer;

	c->irq_mask &= ~GPIO_bit(gpio);

	grer = __raw_readl(c->regbase + GRER_OFFSET) & ~GPIO_bit(gpio);
	gfer = __raw_readl(c->regbase + GFER_OFFSET) & ~GPIO_bit(gpio);
	__raw_writel(grer, c->regbase + GRER_OFFSET);
	__raw_writel(gfer, c->regbase + GFER_OFFSET);
}

static void pxa_unmask_muxed_gpio(struct irq_data *d)
{
	int gpio = irq_to_gpio(d->irq);
	struct pxa_gpio_chip *c = gpio_to_pxachip(gpio);

	c->irq_mask |= GPIO_bit(gpio);
	update_edge_detect(c);
}

static struct irq_chip pxa_muxed_gpio_chip = {
	.name		= "GPIO",
	.irq_ack	= pxa_ack_muxed_gpio,
	.irq_mask	= pxa_mask_muxed_gpio,
	.irq_unmask	= pxa_unmask_muxed_gpio,
	.irq_set_type	= pxa_gpio_irq_type,
};

void __init pxa_init_gpio(int mux_irq, int start, int end, set_wake_t fn)
{
	struct pxa_gpio_chip *c;
	int gpio, irq;

	pxa_last_gpio = end;

	/* Initialize GPIO chips */
	pxa_init_gpio_chip(end);

	/* clear all GPIO edge detects */
	for_each_gpio_chip(gpio, c) {
		__raw_writel(0, c->regbase + GFER_OFFSET);
		__raw_writel(0, c->regbase + GRER_OFFSET);
		__raw_writel(~0,c->regbase + GEDR_OFFSET);
	}

#ifdef CONFIG_ARCH_PXA
	irq = gpio_to_irq(0);
	irq_set_chip_and_handler(irq, &pxa_muxed_gpio_chip,
				 handle_edge_irq);
	set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
	irq_set_chained_handler(IRQ_GPIO0, pxa_gpio_demux_handler);

	irq = gpio_to_irq(1);
	irq_set_chip_and_handler(irq, &pxa_muxed_gpio_chip,
				 handle_edge_irq);
	set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
	irq_set_chained_handler(IRQ_GPIO1, pxa_gpio_demux_handler);
#endif

	for (irq  = gpio_to_irq(start); irq <= gpio_to_irq(end); irq++) {
		irq_set_chip_and_handler(irq, &pxa_muxed_gpio_chip,
					 handle_edge_irq);
		set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
	}

	/* Install handler for GPIO>=2 edge detect interrupts */
	irq_set_chained_handler(mux_irq, pxa_gpio_demux_handler);
	pxa_muxed_gpio_chip.irq_set_wake = fn;
}

#ifdef CONFIG_PM
static int pxa_gpio_suspend(void)
{
	struct pxa_gpio_chip *c;
	int gpio;

	for_each_gpio_chip(gpio, c) {
		c->saved_gplr = __raw_readl(c->regbase + GPLR_OFFSET);
		c->saved_gpdr = __raw_readl(c->regbase + GPDR_OFFSET);
		c->saved_grer = __raw_readl(c->regbase + GRER_OFFSET);
		c->saved_gfer = __raw_readl(c->regbase + GFER_OFFSET);

		/* Clear GPIO transition detect bits */
		__raw_writel(0xffffffff, c->regbase + GEDR_OFFSET);
	}
	return 0;
}

static void pxa_gpio_resume(void)
{
	struct pxa_gpio_chip *c;
	int gpio;

	for_each_gpio_chip(gpio, c) {
		/* restore level with set/clear */
		__raw_writel( c->saved_gplr, c->regbase + GPSR_OFFSET);
		__raw_writel(~c->saved_gplr, c->regbase + GPCR_OFFSET);

		__raw_writel(c->saved_grer, c->regbase + GRER_OFFSET);
		__raw_writel(c->saved_gfer, c->regbase + GFER_OFFSET);
		__raw_writel(c->saved_gpdr, c->regbase + GPDR_OFFSET);
	}
}
#else
#define pxa_gpio_suspend	NULL
#define pxa_gpio_resume		NULL
#endif

struct syscore_ops pxa_gpio_syscore_ops = {
	.suspend	= pxa_gpio_suspend,
	.resume		= pxa_gpio_resume,
};
