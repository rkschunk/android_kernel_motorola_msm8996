/*
 * resource.c - Contains functions for registering and analyzing resource information
 *
 * based on isapnp.c resource management (c) Jaroslav Kysela <perex@perex.cz>
 * Copyright 2003 Adam Belay <ambx1@neo.rr.com>
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <asm/irq.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/init.h>

#include <linux/pnp.h>
#include "base.h"

static int pnp_reserve_irq[16] = {[0 ... 15] = -1 };	/* reserve (don't use) some IRQ */
static int pnp_reserve_dma[8] = {[0 ... 7] = -1 };	/* reserve (don't use) some DMA */
static int pnp_reserve_io[16] = {[0 ... 15] = -1 };	/* reserve (don't use) some I/O region */
static int pnp_reserve_mem[16] = {[0 ... 15] = -1 };	/* reserve (don't use) some memory region */

/*
 * option registration
 */

struct pnp_option *pnp_build_option(int priority)
{
	struct pnp_option *option = pnp_alloc(sizeof(struct pnp_option));

	if (!option)
		return NULL;

	option->priority = priority & 0xff;
	/* make sure the priority is valid */
	if (option->priority > PNP_RES_PRIORITY_FUNCTIONAL)
		option->priority = PNP_RES_PRIORITY_INVALID;

	return option;
}

struct pnp_option *pnp_register_independent_option(struct pnp_dev *dev)
{
	struct pnp_option *option;

	option = pnp_build_option(PNP_RES_PRIORITY_PREFERRED);

	/* this should never happen but if it does we'll try to continue */
	if (dev->independent)
		dev_err(&dev->dev, "independent resource already registered\n");
	dev->independent = option;

	dev_dbg(&dev->dev, "new independent option\n");
	return option;
}

struct pnp_option *pnp_register_dependent_option(struct pnp_dev *dev,
						 int priority)
{
	struct pnp_option *option;

	option = pnp_build_option(priority);

	if (dev->dependent) {
		struct pnp_option *parent = dev->dependent;
		while (parent->next)
			parent = parent->next;
		parent->next = option;
	} else
		dev->dependent = option;

	dev_dbg(&dev->dev, "new dependent option (priority %#x)\n", priority);
	return option;
}

int pnp_register_irq_resource(struct pnp_dev *dev, struct pnp_option *option,
			      pnp_irq_mask_t *map, unsigned char flags)
{
	struct pnp_irq *irq, *ptr;
#ifdef DEBUG
	char buf[PNP_IRQ_NR];   /* hex-encoded, so this is overkill but safe */
#endif

	irq = kzalloc(sizeof(struct pnp_irq), GFP_KERNEL);
	if (!irq)
		return -ENOMEM;

	irq->map = *map;
	irq->flags = flags;

	ptr = option->irq;
	while (ptr && ptr->next)
		ptr = ptr->next;
	if (ptr)
		ptr->next = irq;
	else
		option->irq = irq;

#ifdef CONFIG_PCI
	{
		int i;

		for (i = 0; i < 16; i++)
			if (test_bit(i, irq->map.bits))
				pcibios_penalize_isa_irq(i, 0);
	}
#endif

#ifdef DEBUG
	bitmap_scnprintf(buf, sizeof(buf), irq->map.bits, PNP_IRQ_NR);
	dev_dbg(&dev->dev, "  irq bitmask %s flags %#x\n", buf,
		irq->flags);
#endif
	return 0;
}

int pnp_register_dma_resource(struct pnp_dev *dev, struct pnp_option *option,
			      unsigned char map, unsigned char flags)
{
	struct pnp_dma *dma, *ptr;

	dma = kzalloc(sizeof(struct pnp_dma), GFP_KERNEL);
	if (!dma)
		return -ENOMEM;

	dma->map = map;
	dma->flags = flags;

	ptr = option->dma;
	while (ptr && ptr->next)
		ptr = ptr->next;
	if (ptr)
		ptr->next = dma;
	else
		option->dma = dma;

	dev_dbg(&dev->dev, "  dma bitmask %#x flags %#x\n", dma->map,
		dma->flags);
	return 0;
}

int pnp_register_port_resource(struct pnp_dev *dev, struct pnp_option *option,
			       resource_size_t min, resource_size_t max,
			       resource_size_t align, resource_size_t size,
			       unsigned char flags)
{
	struct pnp_port *port, *ptr;

	port = kzalloc(sizeof(struct pnp_port), GFP_KERNEL);
	if (!port)
		return -ENOMEM;

	port->min = min;
	port->max = max;
	port->align = align;
	port->size = size;
	port->flags = flags;

	ptr = option->port;
	while (ptr && ptr->next)
		ptr = ptr->next;
	if (ptr)
		ptr->next = port;
	else
		option->port = port;

	dev_dbg(&dev->dev, "  io  "
		"min %#llx max %#llx align %lld size %lld flags %#x\n",
		(unsigned long long) port->min,
		(unsigned long long) port->max,
		(unsigned long long) port->align,
		(unsigned long long) port->size, port->flags);
	return 0;
}

int pnp_register_mem_resource(struct pnp_dev *dev, struct pnp_option *option,
			      resource_size_t min, resource_size_t max,
			      resource_size_t align, resource_size_t size,
			      unsigned char flags)
{
	struct pnp_mem *mem, *ptr;

	mem = kzalloc(sizeof(struct pnp_mem), GFP_KERNEL);
	if (!mem)
		return -ENOMEM;

	mem->min = min;
	mem->max = max;
	mem->align = align;
	mem->size = size;
	mem->flags = flags;

	ptr = option->mem;
	while (ptr && ptr->next)
		ptr = ptr->next;
	if (ptr)
		ptr->next = mem;
	else
		option->mem = mem;

	dev_dbg(&dev->dev, "  mem "
		"min %#llx max %#llx align %lld size %lld flags %#x\n",
		(unsigned long long) mem->min,
		(unsigned long long) mem->max,
		(unsigned long long) mem->align,
		(unsigned long long) mem->size, mem->flags);
	return 0;
}

static void pnp_free_port(struct pnp_port *port)
{
	struct pnp_port *next;

	while (port) {
		next = port->next;
		kfree(port);
		port = next;
	}
}

static void pnp_free_irq(struct pnp_irq *irq)
{
	struct pnp_irq *next;

	while (irq) {
		next = irq->next;
		kfree(irq);
		irq = next;
	}
}

static void pnp_free_dma(struct pnp_dma *dma)
{
	struct pnp_dma *next;

	while (dma) {
		next = dma->next;
		kfree(dma);
		dma = next;
	}
}

static void pnp_free_mem(struct pnp_mem *mem)
{
	struct pnp_mem *next;

	while (mem) {
		next = mem->next;
		kfree(mem);
		mem = next;
	}
}

void pnp_free_option(struct pnp_option *option)
{
	struct pnp_option *next;

	while (option) {
		next = option->next;
		pnp_free_port(option->port);
		pnp_free_irq(option->irq);
		pnp_free_dma(option->dma);
		pnp_free_mem(option->mem);
		kfree(option);
		option = next;
	}
}

/*
 * resource validity checking
 */

#define length(start, end) (*(end) - *(start) + 1)

/* Two ranges conflict if one doesn't end before the other starts */
#define ranged_conflict(starta, enda, startb, endb) \
	!((*(enda) < *(startb)) || (*(endb) < *(starta)))

#define cannot_compare(flags) \
((flags) & IORESOURCE_DISABLED)

int pnp_check_port(struct pnp_dev *dev, struct resource *res)
{
	int i;
	struct pnp_dev *tdev;
	struct resource *tres;
	resource_size_t *port, *end, *tport, *tend;

	port = &res->start;
	end = &res->end;

	/* if the resource doesn't exist, don't complain about it */
	if (cannot_compare(res->flags))
		return 1;

	/* check if the resource is already in use, skip if the
	 * device is active because it itself may be in use */
	if (!dev->active) {
		if (__check_region(&ioport_resource, *port, length(port, end)))
			return 0;
	}

	/* check if the resource is reserved */
	for (i = 0; i < 8; i++) {
		int rport = pnp_reserve_io[i << 1];
		int rend = pnp_reserve_io[(i << 1) + 1] + rport - 1;
		if (ranged_conflict(port, end, &rport, &rend))
			return 0;
	}

	/* check for internal conflicts */
	for (i = 0; (tres = pnp_get_resource(dev, IORESOURCE_IO, i)); i++) {
		if (tres != res && tres->flags & IORESOURCE_IO) {
			tport = &tres->start;
			tend = &tres->end;
			if (ranged_conflict(port, end, tport, tend))
				return 0;
		}
	}

	/* check for conflicts with other pnp devices */
	pnp_for_each_dev(tdev) {
		if (tdev == dev)
			continue;
		for (i = 0;
		     (tres = pnp_get_resource(tdev, IORESOURCE_IO, i));
		     i++) {
			if (tres->flags & IORESOURCE_IO) {
				if (cannot_compare(tres->flags))
					continue;
				tport = &tres->start;
				tend = &tres->end;
				if (ranged_conflict(port, end, tport, tend))
					return 0;
			}
		}
	}

	return 1;
}

int pnp_check_mem(struct pnp_dev *dev, struct resource *res)
{
	int i;
	struct pnp_dev *tdev;
	struct resource *tres;
	resource_size_t *addr, *end, *taddr, *tend;

	addr = &res->start;
	end = &res->end;

	/* if the resource doesn't exist, don't complain about it */
	if (cannot_compare(res->flags))
		return 1;

	/* check if the resource is already in use, skip if the
	 * device is active because it itself may be in use */
	if (!dev->active) {
		if (check_mem_region(*addr, length(addr, end)))
			return 0;
	}

	/* check if the resource is reserved */
	for (i = 0; i < 8; i++) {
		int raddr = pnp_reserve_mem[i << 1];
		int rend = pnp_reserve_mem[(i << 1) + 1] + raddr - 1;
		if (ranged_conflict(addr, end, &raddr, &rend))
			return 0;
	}

	/* check for internal conflicts */
	for (i = 0; (tres = pnp_get_resource(dev, IORESOURCE_MEM, i)); i++) {
		if (tres != res && tres->flags & IORESOURCE_MEM) {
			taddr = &tres->start;
			tend = &tres->end;
			if (ranged_conflict(addr, end, taddr, tend))
				return 0;
		}
	}

	/* check for conflicts with other pnp devices */
	pnp_for_each_dev(tdev) {
		if (tdev == dev)
			continue;
		for (i = 0;
		     (tres = pnp_get_resource(tdev, IORESOURCE_MEM, i));
		     i++) {
			if (tres->flags & IORESOURCE_MEM) {
				if (cannot_compare(tres->flags))
					continue;
				taddr = &tres->start;
				tend = &tres->end;
				if (ranged_conflict(addr, end, taddr, tend))
					return 0;
			}
		}
	}

	return 1;
}

static irqreturn_t pnp_test_handler(int irq, void *dev_id)
{
	return IRQ_HANDLED;
}

int pnp_check_irq(struct pnp_dev *dev, struct resource *res)
{
	int i;
	struct pnp_dev *tdev;
	struct resource *tres;
	resource_size_t *irq;

	irq = &res->start;

	/* if the resource doesn't exist, don't complain about it */
	if (cannot_compare(res->flags))
		return 1;

	/* check if the resource is valid */
	if (*irq < 0 || *irq > 15)
		return 0;

	/* check if the resource is reserved */
	for (i = 0; i < 16; i++) {
		if (pnp_reserve_irq[i] == *irq)
			return 0;
	}

	/* check for internal conflicts */
	for (i = 0; (tres = pnp_get_resource(dev, IORESOURCE_IRQ, i)); i++) {
		if (tres != res && tres->flags & IORESOURCE_IRQ) {
			if (tres->start == *irq)
				return 0;
		}
	}

#ifdef CONFIG_PCI
	/* check if the resource is being used by a pci device */
	{
		struct pci_dev *pci = NULL;
		for_each_pci_dev(pci) {
			if (pci->irq == *irq) {
				pci_dev_put(pci);
				return 0;
			}
		}
	}
#endif

	/* check if the resource is already in use, skip if the
	 * device is active because it itself may be in use */
	if (!dev->active) {
		if (request_irq(*irq, pnp_test_handler,
				IRQF_DISABLED | IRQF_PROBE_SHARED, "pnp", NULL))
			return 0;
		free_irq(*irq, NULL);
	}

	/* check for conflicts with other pnp devices */
	pnp_for_each_dev(tdev) {
		if (tdev == dev)
			continue;
		for (i = 0;
		     (tres = pnp_get_resource(tdev, IORESOURCE_IRQ, i));
		     i++) {
			if (tres->flags & IORESOURCE_IRQ) {
				if (cannot_compare(tres->flags))
					continue;
				if (tres->start == *irq)
					return 0;
			}
		}
	}

	return 1;
}

int pnp_check_dma(struct pnp_dev *dev, struct resource *res)
{
#ifndef CONFIG_IA64
	int i;
	struct pnp_dev *tdev;
	struct resource *tres;
	resource_size_t *dma;

	dma = &res->start;

	/* if the resource doesn't exist, don't complain about it */
	if (cannot_compare(res->flags))
		return 1;

	/* check if the resource is valid */
	if (*dma < 0 || *dma == 4 || *dma > 7)
		return 0;

	/* check if the resource is reserved */
	for (i = 0; i < 8; i++) {
		if (pnp_reserve_dma[i] == *dma)
			return 0;
	}

	/* check for internal conflicts */
	for (i = 0; (tres = pnp_get_resource(dev, IORESOURCE_DMA, i)); i++) {
		if (tres != res && tres->flags & IORESOURCE_DMA) {
			if (tres->start == *dma)
				return 0;
		}
	}

	/* check if the resource is already in use, skip if the
	 * device is active because it itself may be in use */
	if (!dev->active) {
		if (request_dma(*dma, "pnp"))
			return 0;
		free_dma(*dma);
	}

	/* check for conflicts with other pnp devices */
	pnp_for_each_dev(tdev) {
		if (tdev == dev)
			continue;
		for (i = 0;
		     (tres = pnp_get_resource(tdev, IORESOURCE_DMA, i));
		     i++) {
			if (tres->flags & IORESOURCE_DMA) {
				if (cannot_compare(tres->flags))
					continue;
				if (tres->start == *dma)
					return 0;
			}
		}
	}

	return 1;
#else
	/* IA64 does not have legacy DMA */
	return 0;
#endif
}

int pnp_resource_type(struct resource *res)
{
	return res->flags & (IORESOURCE_IO  | IORESOURCE_MEM |
			     IORESOURCE_IRQ | IORESOURCE_DMA);
}

struct resource *pnp_get_resource(struct pnp_dev *dev,
				  unsigned int type, unsigned int num)
{
	struct pnp_resource *pnp_res;
	struct resource *res;

	list_for_each_entry(pnp_res, &dev->resources, list) {
		res = &pnp_res->res;
		if (pnp_resource_type(res) == type && num-- == 0)
			return res;
	}
	return NULL;
}
EXPORT_SYMBOL(pnp_get_resource);

static struct pnp_resource *pnp_new_resource(struct pnp_dev *dev)
{
	struct pnp_resource *pnp_res;

	pnp_res = kzalloc(sizeof(struct pnp_resource), GFP_KERNEL);
	if (!pnp_res)
		return NULL;

	list_add_tail(&pnp_res->list, &dev->resources);
	return pnp_res;
}

struct pnp_resource *pnp_add_irq_resource(struct pnp_dev *dev, int irq,
					  int flags)
{
	struct pnp_resource *pnp_res;
	struct resource *res;

	pnp_res = pnp_new_resource(dev);
	if (!pnp_res) {
		dev_err(&dev->dev, "can't add resource for IRQ %d\n", irq);
		return NULL;
	}

	res = &pnp_res->res;
	res->flags = IORESOURCE_IRQ | flags;
	res->start = irq;
	res->end = irq;

	dev_dbg(&dev->dev, "  add irq %d flags %#x\n", irq, flags);
	return pnp_res;
}

struct pnp_resource *pnp_add_dma_resource(struct pnp_dev *dev, int dma,
					  int flags)
{
	struct pnp_resource *pnp_res;
	struct resource *res;

	pnp_res = pnp_new_resource(dev);
	if (!pnp_res) {
		dev_err(&dev->dev, "can't add resource for DMA %d\n", dma);
		return NULL;
	}

	res = &pnp_res->res;
	res->flags = IORESOURCE_DMA | flags;
	res->start = dma;
	res->end = dma;

	dev_dbg(&dev->dev, "  add dma %d flags %#x\n", dma, flags);
	return pnp_res;
}

struct pnp_resource *pnp_add_io_resource(struct pnp_dev *dev,
					 resource_size_t start,
					 resource_size_t end, int flags)
{
	struct pnp_resource *pnp_res;
	struct resource *res;

	pnp_res = pnp_new_resource(dev);
	if (!pnp_res) {
		dev_err(&dev->dev, "can't add resource for IO %#llx-%#llx\n",
			(unsigned long long) start,
			(unsigned long long) end);
		return NULL;
	}

	res = &pnp_res->res;
	res->flags = IORESOURCE_IO | flags;
	res->start = start;
	res->end = end;

	dev_dbg(&dev->dev, "  add io  %#llx-%#llx flags %#x\n",
		(unsigned long long) start, (unsigned long long) end, flags);
	return pnp_res;
}

struct pnp_resource *pnp_add_mem_resource(struct pnp_dev *dev,
					  resource_size_t start,
					  resource_size_t end, int flags)
{
	struct pnp_resource *pnp_res;
	struct resource *res;

	pnp_res = pnp_new_resource(dev);
	if (!pnp_res) {
		dev_err(&dev->dev, "can't add resource for MEM %#llx-%#llx\n",
			(unsigned long long) start,
			(unsigned long long) end);
		return NULL;
	}

	res = &pnp_res->res;
	res->flags = IORESOURCE_MEM | flags;
	res->start = start;
	res->end = end;

	dev_dbg(&dev->dev, "  add mem %#llx-%#llx flags %#x\n",
		(unsigned long long) start, (unsigned long long) end, flags);
	return pnp_res;
}

static int pnp_possible_option(struct pnp_option *option, int type,
			       resource_size_t start, resource_size_t size)
{
	struct pnp_option *tmp;
	struct pnp_port *port;
	struct pnp_mem *mem;
	struct pnp_irq *irq;
	struct pnp_dma *dma;

	if (!option)
		return 0;

	for (tmp = option; tmp; tmp = tmp->next) {
		switch (type) {
		case IORESOURCE_IO:
			for (port = tmp->port; port; port = port->next) {
				if (port->min == start && port->size == size)
					return 1;
			}
			break;
		case IORESOURCE_MEM:
			for (mem = tmp->mem; mem; mem = mem->next) {
				if (mem->min == start && mem->size == size)
					return 1;
			}
			break;
		case IORESOURCE_IRQ:
			for (irq = tmp->irq; irq; irq = irq->next) {
				if (start < PNP_IRQ_NR &&
				    test_bit(start, irq->map.bits))
					return 1;
			}
			break;
		case IORESOURCE_DMA:
			for (dma = tmp->dma; dma; dma = dma->next) {
				if (dma->map & (1 << start))
					return 1;
			}
			break;
		}
	}

	return 0;
}

/*
 * Determine whether the specified resource is a possible configuration
 * for this device.
 */
int pnp_possible_config(struct pnp_dev *dev, int type, resource_size_t start,
			resource_size_t size)
{
	if (pnp_possible_option(dev->independent, type, start, size))
		return 1;

	if (pnp_possible_option(dev->dependent, type, start, size))
		return 1;

	return 0;
}
EXPORT_SYMBOL(pnp_possible_config);

/* format is: pnp_reserve_irq=irq1[,irq2] .... */
static int __init pnp_setup_reserve_irq(char *str)
{
	int i;

	for (i = 0; i < 16; i++)
		if (get_option(&str, &pnp_reserve_irq[i]) != 2)
			break;
	return 1;
}

__setup("pnp_reserve_irq=", pnp_setup_reserve_irq);

/* format is: pnp_reserve_dma=dma1[,dma2] .... */
static int __init pnp_setup_reserve_dma(char *str)
{
	int i;

	for (i = 0; i < 8; i++)
		if (get_option(&str, &pnp_reserve_dma[i]) != 2)
			break;
	return 1;
}

__setup("pnp_reserve_dma=", pnp_setup_reserve_dma);

/* format is: pnp_reserve_io=io1,size1[,io2,size2] .... */
static int __init pnp_setup_reserve_io(char *str)
{
	int i;

	for (i = 0; i < 16; i++)
		if (get_option(&str, &pnp_reserve_io[i]) != 2)
			break;
	return 1;
}

__setup("pnp_reserve_io=", pnp_setup_reserve_io);

/* format is: pnp_reserve_mem=mem1,size1[,mem2,size2] .... */
static int __init pnp_setup_reserve_mem(char *str)
{
	int i;

	for (i = 0; i < 16; i++)
		if (get_option(&str, &pnp_reserve_mem[i]) != 2)
			break;
	return 1;
}

__setup("pnp_reserve_mem=", pnp_setup_reserve_mem);
