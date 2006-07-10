/*
 * Common pmac/prep/chrp pci routines. -- Cort
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/capability.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/bootmem.h>
#include <linux/irq.h>

#include <asm/processor.h>
#include <asm/io.h>
#include <asm/prom.h>
#include <asm/sections.h>
#include <asm/pci-bridge.h>
#include <asm/byteorder.h>
#include <asm/uaccess.h>
#include <asm/machdep.h>

#undef DEBUG

#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...)
#endif

unsigned long isa_io_base     = 0;
unsigned long isa_mem_base    = 0;
unsigned long pci_dram_offset = 0;
int pcibios_assign_bus_offset = 1;

void pcibios_make_OF_bus_map(void);

static int pci_relocate_bridge_resource(struct pci_bus *bus, int i);
static int probe_resource(struct pci_bus *parent, struct resource *pr,
			  struct resource *res, struct resource **conflict);
static void update_bridge_base(struct pci_bus *bus, int i);
static void pcibios_fixup_resources(struct pci_dev* dev);
static void fixup_broken_pcnet32(struct pci_dev* dev);
static int reparent_resources(struct resource *parent, struct resource *res);
static void fixup_cpc710_pci64(struct pci_dev* dev);
#ifdef CONFIG_PPC_OF
static u8* pci_to_OF_bus_map;
#endif

/* By default, we don't re-assign bus numbers. We do this only on
 * some pmacs
 */
int pci_assign_all_buses;

struct pci_controller* hose_head;
struct pci_controller** hose_tail = &hose_head;

static int pci_bus_count;

static void
fixup_broken_pcnet32(struct pci_dev* dev)
{
	if ((dev->class>>8 == PCI_CLASS_NETWORK_ETHERNET)) {
		dev->vendor = PCI_VENDOR_ID_AMD;
		pci_write_config_word(dev, PCI_VENDOR_ID, PCI_VENDOR_ID_AMD);
	}
}
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_TRIDENT,	PCI_ANY_ID,			fixup_broken_pcnet32);

static void
fixup_cpc710_pci64(struct pci_dev* dev)
{
	/* Hide the PCI64 BARs from the kernel as their content doesn't
	 * fit well in the resource management
	 */
	dev->resource[0].start = dev->resource[0].end = 0;
	dev->resource[0].flags = 0;
	dev->resource[1].start = dev->resource[1].end = 0;
	dev->resource[1].flags = 0;
}
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_IBM,	PCI_DEVICE_ID_IBM_CPC710_PCI64,	fixup_cpc710_pci64);

static void
pcibios_fixup_resources(struct pci_dev *dev)
{
	struct pci_controller* hose = (struct pci_controller *)dev->sysdata;
	int i;
	unsigned long offset;

	if (!hose) {
		printk(KERN_ERR "No hose for PCI dev %s!\n", pci_name(dev));
		return;
	}
	for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
		struct resource *res = dev->resource + i;
		if (!res->flags)
			continue;
		if (res->end == 0xffffffff) {
			DBG("PCI:%s Resource %d [%016llx-%016llx] is unassigned\n",
			    pci_name(dev), i, res->start, res->end);
			res->end -= res->start;
			res->start = 0;
			res->flags |= IORESOURCE_UNSET;
			continue;
		}
		offset = 0;
		if (res->flags & IORESOURCE_MEM) {
			offset = hose->pci_mem_offset;
		} else if (res->flags & IORESOURCE_IO) {
			offset = (unsigned long) hose->io_base_virt
				- isa_io_base;
		}
		if (offset != 0) {
			res->start += offset;
			res->end += offset;
#ifdef DEBUG
			printk("Fixup res %d (%lx) of dev %s: %llx -> %llx\n",
			       i, res->flags, pci_name(dev),
			       res->start - offset, res->start);
#endif
		}
	}

	/* Call machine specific resource fixup */
	if (ppc_md.pcibios_fixup_resources)
		ppc_md.pcibios_fixup_resources(dev);
}
DECLARE_PCI_FIXUP_HEADER(PCI_ANY_ID,		PCI_ANY_ID,			pcibios_fixup_resources);

void pcibios_resource_to_bus(struct pci_dev *dev, struct pci_bus_region *region,
			struct resource *res)
{
	unsigned long offset = 0;
	struct pci_controller *hose = dev->sysdata;

	if (hose && res->flags & IORESOURCE_IO)
		offset = (unsigned long)hose->io_base_virt - isa_io_base;
	else if (hose && res->flags & IORESOURCE_MEM)
		offset = hose->pci_mem_offset;
	region->start = res->start - offset;
	region->end = res->end - offset;
}
EXPORT_SYMBOL(pcibios_resource_to_bus);

void pcibios_bus_to_resource(struct pci_dev *dev, struct resource *res,
			     struct pci_bus_region *region)
{
	unsigned long offset = 0;
	struct pci_controller *hose = dev->sysdata;

	if (hose && res->flags & IORESOURCE_IO)
		offset = (unsigned long)hose->io_base_virt - isa_io_base;
	else if (hose && res->flags & IORESOURCE_MEM)
		offset = hose->pci_mem_offset;
	res->start = region->start + offset;
	res->end = region->end + offset;
}
EXPORT_SYMBOL(pcibios_bus_to_resource);

/*
 * We need to avoid collisions with `mirrored' VGA ports
 * and other strange ISA hardware, so we always want the
 * addresses to be allocated in the 0x000-0x0ff region
 * modulo 0x400.
 *
 * Why? Because some silly external IO cards only decode
 * the low 10 bits of the IO address. The 0x00-0xff region
 * is reserved for motherboard devices that decode all 16
 * bits, so it's ok to allocate at, say, 0x2800-0x28ff,
 * but we want to try to avoid allocating at 0x2900-0x2bff
 * which might have be mirrored at 0x0100-0x03ff..
 */
void pcibios_align_resource(void *data, struct resource *res,
				resource_size_t size, resource_size_t align)
{
	struct pci_dev *dev = data;

	if (res->flags & IORESOURCE_IO) {
		resource_size_t start = res->start;

		if (size > 0x100) {
			printk(KERN_ERR "PCI: I/O Region %s/%d too large"
			       " (%lld bytes)\n", pci_name(dev),
			       dev->resource - res, (unsigned long long)size);
		}

		if (start & 0x300) {
			start = (start + 0x3ff) & ~0x3ff;
			res->start = start;
		}
	}
}
EXPORT_SYMBOL(pcibios_align_resource);

/*
 *  Handle resources of PCI devices.  If the world were perfect, we could
 *  just allocate all the resource regions and do nothing more.  It isn't.
 *  On the other hand, we cannot just re-allocate all devices, as it would
 *  require us to know lots of host bridge internals.  So we attempt to
 *  keep as much of the original configuration as possible, but tweak it
 *  when it's found to be wrong.
 *
 *  Known BIOS problems we have to work around:
 *	- I/O or memory regions not configured
 *	- regions configured, but not enabled in the command register
 *	- bogus I/O addresses above 64K used
 *	- expansion ROMs left enabled (this may sound harmless, but given
 *	  the fact the PCI specs explicitly allow address decoders to be
 *	  shared between expansion ROMs and other resource regions, it's
 *	  at least dangerous)
 *
 *  Our solution:
 *	(1) Allocate resources for all buses behind PCI-to-PCI bridges.
 *	    This gives us fixed barriers on where we can allocate.
 *	(2) Allocate resources for all enabled devices.  If there is
 *	    a collision, just mark the resource as unallocated. Also
 *	    disable expansion ROMs during this step.
 *	(3) Try to allocate resources for disabled devices.  If the
 *	    resources were assigned correctly, everything goes well,
 *	    if they weren't, they won't disturb allocation of other
 *	    resources.
 *	(4) Assign new addresses to resources which were either
 *	    not configured at all or misconfigured.  If explicitly
 *	    requested by the user, configure expansion ROM address
 *	    as well.
 */

static void __init
pcibios_allocate_bus_resources(struct list_head *bus_list)
{
	struct pci_bus *bus;
	int i;
	struct resource *res, *pr;

	/* Depth-First Search on bus tree */
	list_for_each_entry(bus, bus_list, node) {
		for (i = 0; i < 4; ++i) {
			if ((res = bus->resource[i]) == NULL || !res->flags
			    || res->start > res->end)
				continue;
			if (bus->parent == NULL)
				pr = (res->flags & IORESOURCE_IO)?
					&ioport_resource: &iomem_resource;
			else {
				pr = pci_find_parent_resource(bus->self, res);
				if (pr == res) {
					/* this happens when the generic PCI
					 * code (wrongly) decides that this
					 * bridge is transparent  -- paulus
					 */
					continue;
				}
			}

			DBG("PCI: bridge rsrc %llx..%llx (%lx), parent %p\n",
				res->start, res->end, res->flags, pr);
			if (pr) {
				if (request_resource(pr, res) == 0)
					continue;
				/*
				 * Must be a conflict with an existing entry.
				 * Move that entry (or entries) under the
				 * bridge resource and try again.
				 */
				if (reparent_resources(pr, res) == 0)
					continue;
			}
			printk(KERN_ERR "PCI: Cannot allocate resource region "
			       "%d of PCI bridge %d\n", i, bus->number);
			if (pci_relocate_bridge_resource(bus, i))
				bus->resource[i] = NULL;
		}
		pcibios_allocate_bus_resources(&bus->children);
	}
}

/*
 * Reparent resource children of pr that conflict with res
 * under res, and make res replace those children.
 */
static int __init
reparent_resources(struct resource *parent, struct resource *res)
{
	struct resource *p, **pp;
	struct resource **firstpp = NULL;

	for (pp = &parent->child; (p = *pp) != NULL; pp = &p->sibling) {
		if (p->end < res->start)
			continue;
		if (res->end < p->start)
			break;
		if (p->start < res->start || p->end > res->end)
			return -1;	/* not completely contained */
		if (firstpp == NULL)
			firstpp = pp;
	}
	if (firstpp == NULL)
		return -1;	/* didn't find any conflicting entries? */
	res->parent = parent;
	res->child = *firstpp;
	res->sibling = *pp;
	*firstpp = res;
	*pp = NULL;
	for (p = res->child; p != NULL; p = p->sibling) {
		p->parent = res;
		DBG(KERN_INFO "PCI: reparented %s [%llx..%llx] under %s\n",
		    p->name, p->start, p->end, res->name);
	}
	return 0;
}

/*
 * A bridge has been allocated a range which is outside the range
 * of its parent bridge, so it needs to be moved.
 */
static int __init
pci_relocate_bridge_resource(struct pci_bus *bus, int i)
{
	struct resource *res, *pr, *conflict;
	unsigned long try, size;
	int j;
	struct pci_bus *parent = bus->parent;

	if (parent == NULL) {
		/* shouldn't ever happen */
		printk(KERN_ERR "PCI: can't move host bridge resource\n");
		return -1;
	}
	res = bus->resource[i];
	if (res == NULL)
		return -1;
	pr = NULL;
	for (j = 0; j < 4; j++) {
		struct resource *r = parent->resource[j];
		if (!r)
			continue;
		if ((res->flags ^ r->flags) & (IORESOURCE_IO | IORESOURCE_MEM))
			continue;
		if (!((res->flags ^ r->flags) & IORESOURCE_PREFETCH)) {
			pr = r;
			break;
		}
		if (res->flags & IORESOURCE_PREFETCH)
			pr = r;
	}
	if (pr == NULL)
		return -1;
	size = res->end - res->start;
	if (pr->start > pr->end || size > pr->end - pr->start)
		return -1;
	try = pr->end;
	for (;;) {
		res->start = try - size;
		res->end = try;
		if (probe_resource(bus->parent, pr, res, &conflict) == 0)
			break;
		if (conflict->start <= pr->start + size)
			return -1;
		try = conflict->start - 1;
	}
	if (request_resource(pr, res)) {
		DBG(KERN_ERR "PCI: huh? couldn't move to %llx..%llx\n",
		    res->start, res->end);
		return -1;		/* "can't happen" */
	}
	update_bridge_base(bus, i);
	printk(KERN_INFO "PCI: bridge %d resource %d moved to %llx..%llx\n",
	       bus->number, i, (unsigned long long)res->start,
	       (unsigned long long)res->end);
	return 0;
}

static int __init
probe_resource(struct pci_bus *parent, struct resource *pr,
	       struct resource *res, struct resource **conflict)
{
	struct pci_bus *bus;
	struct pci_dev *dev;
	struct resource *r;
	int i;

	for (r = pr->child; r != NULL; r = r->sibling) {
		if (r->end >= res->start && res->end >= r->start) {
			*conflict = r;
			return 1;
		}
	}
	list_for_each_entry(bus, &parent->children, node) {
		for (i = 0; i < 4; ++i) {
			if ((r = bus->resource[i]) == NULL)
				continue;
			if (!r->flags || r->start > r->end || r == res)
				continue;
			if (pci_find_parent_resource(bus->self, r) != pr)
				continue;
			if (r->end >= res->start && res->end >= r->start) {
				*conflict = r;
				return 1;
			}
		}
	}
	list_for_each_entry(dev, &parent->devices, bus_list) {
		for (i = 0; i < 6; ++i) {
			r = &dev->resource[i];
			if (!r->flags || (r->flags & IORESOURCE_UNSET))
				continue;
			if (pci_find_parent_resource(dev, r) != pr)
				continue;
			if (r->end >= res->start && res->end >= r->start) {
				*conflict = r;
				return 1;
			}
		}
	}
	return 0;
}

static void __init
update_bridge_base(struct pci_bus *bus, int i)
{
	struct resource *res = bus->resource[i];
	u8 io_base_lo, io_limit_lo;
	u16 mem_base, mem_limit;
	u16 cmd;
	unsigned long start, end, off;
	struct pci_dev *dev = bus->self;
	struct pci_controller *hose = dev->sysdata;

	if (!hose) {
		printk("update_bridge_base: no hose?\n");
		return;
	}
	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	pci_write_config_word(dev, PCI_COMMAND,
			      cmd & ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY));
	if (res->flags & IORESOURCE_IO) {
		off = (unsigned long) hose->io_base_virt - isa_io_base;
		start = res->start - off;
		end = res->end - off;
		io_base_lo = (start >> 8) & PCI_IO_RANGE_MASK;
		io_limit_lo = (end >> 8) & PCI_IO_RANGE_MASK;
		if (end > 0xffff) {
			pci_write_config_word(dev, PCI_IO_BASE_UPPER16,
					      start >> 16);
			pci_write_config_word(dev, PCI_IO_LIMIT_UPPER16,
					      end >> 16);
			io_base_lo |= PCI_IO_RANGE_TYPE_32;
		} else
			io_base_lo |= PCI_IO_RANGE_TYPE_16;
		pci_write_config_byte(dev, PCI_IO_BASE, io_base_lo);
		pci_write_config_byte(dev, PCI_IO_LIMIT, io_limit_lo);

	} else if ((res->flags & (IORESOURCE_MEM | IORESOURCE_PREFETCH))
		   == IORESOURCE_MEM) {
		off = hose->pci_mem_offset;
		mem_base = ((res->start - off) >> 16) & PCI_MEMORY_RANGE_MASK;
		mem_limit = ((res->end - off) >> 16) & PCI_MEMORY_RANGE_MASK;
		pci_write_config_word(dev, PCI_MEMORY_BASE, mem_base);
		pci_write_config_word(dev, PCI_MEMORY_LIMIT, mem_limit);

	} else if ((res->flags & (IORESOURCE_MEM | IORESOURCE_PREFETCH))
		   == (IORESOURCE_MEM | IORESOURCE_PREFETCH)) {
		off = hose->pci_mem_offset;
		mem_base = ((res->start - off) >> 16) & PCI_PREF_RANGE_MASK;
		mem_limit = ((res->end - off) >> 16) & PCI_PREF_RANGE_MASK;
		pci_write_config_word(dev, PCI_PREF_MEMORY_BASE, mem_base);
		pci_write_config_word(dev, PCI_PREF_MEMORY_LIMIT, mem_limit);

	} else {
		DBG(KERN_ERR "PCI: ugh, bridge %s res %d has flags=%lx\n",
		    pci_name(dev), i, res->flags);
	}
	pci_write_config_word(dev, PCI_COMMAND, cmd);
}

static inline void alloc_resource(struct pci_dev *dev, int idx)
{
	struct resource *pr, *r = &dev->resource[idx];

	DBG("PCI:%s: Resource %d: %016llx-%016llx (f=%lx)\n",
	    pci_name(dev), idx, r->start, r->end, r->flags);
	pr = pci_find_parent_resource(dev, r);
	if (!pr || request_resource(pr, r) < 0) {
		printk(KERN_ERR "PCI: Cannot allocate resource region %d"
		       " of device %s\n", idx, pci_name(dev));
		if (pr)
			DBG("PCI:  parent is %p: %016llx-%016llx (f=%lx)\n",
			    pr, pr->start, pr->end, pr->flags);
		/* We'll assign a new address later */
		r->flags |= IORESOURCE_UNSET;
		r->end -= r->start;
		r->start = 0;
	}
}

static void __init
pcibios_allocate_resources(int pass)
{
	struct pci_dev *dev = NULL;
	int idx, disabled;
	u16 command;
	struct resource *r;

	for_each_pci_dev(dev) {
		pci_read_config_word(dev, PCI_COMMAND, &command);
		for (idx = 0; idx < 6; idx++) {
			r = &dev->resource[idx];
			if (r->parent)		/* Already allocated */
				continue;
			if (!r->flags || (r->flags & IORESOURCE_UNSET))
				continue;	/* Not assigned at all */
			if (r->flags & IORESOURCE_IO)
				disabled = !(command & PCI_COMMAND_IO);
			else
				disabled = !(command & PCI_COMMAND_MEMORY);
			if (pass == disabled)
				alloc_resource(dev, idx);
		}
		if (pass)
			continue;
		r = &dev->resource[PCI_ROM_RESOURCE];
		if (r->flags & IORESOURCE_ROM_ENABLE) {
			/* Turn the ROM off, leave the resource region, but keep it unregistered. */
			u32 reg;
			DBG("PCI: Switching off ROM of %s\n", pci_name(dev));
			r->flags &= ~IORESOURCE_ROM_ENABLE;
			pci_read_config_dword(dev, dev->rom_base_reg, &reg);
			pci_write_config_dword(dev, dev->rom_base_reg,
					       reg & ~PCI_ROM_ADDRESS_ENABLE);
		}
	}
}

static void __init
pcibios_assign_resources(void)
{
	struct pci_dev *dev = NULL;
	int idx;
	struct resource *r;

	for_each_pci_dev(dev) {
		int class = dev->class >> 8;

		/* Don't touch classless devices and host bridges */
		if (!class || class == PCI_CLASS_BRIDGE_HOST)
			continue;

		for (idx = 0; idx < 6; idx++) {
			r = &dev->resource[idx];

			/*
			 * We shall assign a new address to this resource,
			 * either because the BIOS (sic) forgot to do so
			 * or because we have decided the old address was
			 * unusable for some reason.
			 */
			if ((r->flags & IORESOURCE_UNSET) && r->end &&
			    (!ppc_md.pcibios_enable_device_hook ||
			     !ppc_md.pcibios_enable_device_hook(dev, 1))) {
				r->flags &= ~IORESOURCE_UNSET;
				pci_assign_resource(dev, idx);
			}
		}

#if 0 /* don't assign ROMs */
		r = &dev->resource[PCI_ROM_RESOURCE];
		r->end -= r->start;
		r->start = 0;
		if (r->end)
			pci_assign_resource(dev, PCI_ROM_RESOURCE);
#endif
	}
}


int
pcibios_enable_resources(struct pci_dev *dev, int mask)
{
	u16 cmd, old_cmd;
	int idx;
	struct resource *r;

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	old_cmd = cmd;
	for (idx=0; idx<6; idx++) {
		/* Only set up the requested stuff */
		if (!(mask & (1<<idx)))
			continue;
	
		r = &dev->resource[idx];
		if (r->flags & IORESOURCE_UNSET) {
			printk(KERN_ERR "PCI: Device %s not available because of resource collisions\n", pci_name(dev));
			return -EINVAL;
		}
		if (r->flags & IORESOURCE_IO)
			cmd |= PCI_COMMAND_IO;
		if (r->flags & IORESOURCE_MEM)
			cmd |= PCI_COMMAND_MEMORY;
	}
	if (dev->resource[PCI_ROM_RESOURCE].start)
		cmd |= PCI_COMMAND_MEMORY;
	if (cmd != old_cmd) {
		printk("PCI: Enabling device %s (%04x -> %04x)\n", pci_name(dev), old_cmd, cmd);
		pci_write_config_word(dev, PCI_COMMAND, cmd);
	}
	return 0;
}

static int next_controller_index;

struct pci_controller * __init
pcibios_alloc_controller(void)
{
	struct pci_controller *hose;

	hose = (struct pci_controller *)alloc_bootmem(sizeof(*hose));
	memset(hose, 0, sizeof(struct pci_controller));

	*hose_tail = hose;
	hose_tail = &hose->next;

	hose->index = next_controller_index++;

	return hose;
}

#ifdef CONFIG_PPC_OF
/*
 * Functions below are used on OpenFirmware machines.
 */
static void
make_one_node_map(struct device_node* node, u8 pci_bus)
{
	int *bus_range;
	int len;

	if (pci_bus >= pci_bus_count)
		return;
	bus_range = (int *) get_property(node, "bus-range", &len);
	if (bus_range == NULL || len < 2 * sizeof(int)) {
		printk(KERN_WARNING "Can't get bus-range for %s, "
		       "assuming it starts at 0\n", node->full_name);
		pci_to_OF_bus_map[pci_bus] = 0;
	} else
		pci_to_OF_bus_map[pci_bus] = bus_range[0];

	for (node=node->child; node != 0;node = node->sibling) {
		struct pci_dev* dev;
		unsigned int *class_code, *reg;
	
		class_code = (unsigned int *) get_property(node, "class-code", NULL);
		if (!class_code || ((*class_code >> 8) != PCI_CLASS_BRIDGE_PCI &&
			(*class_code >> 8) != PCI_CLASS_BRIDGE_CARDBUS))
			continue;
		reg = (unsigned int *)get_property(node, "reg", NULL);
		if (!reg)
			continue;
		dev = pci_find_slot(pci_bus, ((reg[0] >> 8) & 0xff));
		if (!dev || !dev->subordinate)
			continue;
		make_one_node_map(node, dev->subordinate->number);
	}
}
	
void
pcibios_make_OF_bus_map(void)
{
	int i;
	struct pci_controller* hose;
	u8* of_prop_map;

	pci_to_OF_bus_map = (u8*)kmalloc(pci_bus_count, GFP_KERNEL);
	if (!pci_to_OF_bus_map) {
		printk(KERN_ERR "Can't allocate OF bus map !\n");
		return;
	}

	/* We fill the bus map with invalid values, that helps
	 * debugging.
	 */
	for (i=0; i<pci_bus_count; i++)
		pci_to_OF_bus_map[i] = 0xff;

	/* For each hose, we begin searching bridges */
	for(hose=hose_head; hose; hose=hose->next) {
		struct device_node* node;	
		node = (struct device_node *)hose->arch_data;
		if (!node)
			continue;
		make_one_node_map(node, hose->first_busno);
	}
	of_prop_map = get_property(find_path_device("/"), "pci-OF-bus-map", NULL);
	if (of_prop_map)
		memcpy(of_prop_map, pci_to_OF_bus_map, pci_bus_count);
#ifdef DEBUG
	printk("PCI->OF bus map:\n");
	for (i=0; i<pci_bus_count; i++) {
		if (pci_to_OF_bus_map[i] == 0xff)
			continue;
		printk("%d -> %d\n", i, pci_to_OF_bus_map[i]);
	}
#endif
}

typedef int (*pci_OF_scan_iterator)(struct device_node* node, void* data);

static struct device_node*
scan_OF_pci_childs(struct device_node* node, pci_OF_scan_iterator filter, void* data)
{
	struct device_node* sub_node;

	for (; node != 0;node = node->sibling) {
		unsigned int *class_code;
	
		if (filter(node, data))
			return node;

		/* For PCI<->PCI bridges or CardBus bridges, we go down
		 * Note: some OFs create a parent node "multifunc-device" as
		 * a fake root for all functions of a multi-function device,
		 * we go down them as well.
		 */
		class_code = (unsigned int *) get_property(node, "class-code", NULL);
		if ((!class_code || ((*class_code >> 8) != PCI_CLASS_BRIDGE_PCI &&
			(*class_code >> 8) != PCI_CLASS_BRIDGE_CARDBUS)) &&
			strcmp(node->name, "multifunc-device"))
			continue;
		sub_node = scan_OF_pci_childs(node->child, filter, data);
		if (sub_node)
			return sub_node;
	}
	return NULL;
}

static int
scan_OF_pci_childs_iterator(struct device_node* node, void* data)
{
	unsigned int *reg;
	u8* fdata = (u8*)data;
	
	reg = (unsigned int *) get_property(node, "reg", NULL);
	if (reg && ((reg[0] >> 8) & 0xff) == fdata[1]
		&& ((reg[0] >> 16) & 0xff) == fdata[0])
		return 1;
	return 0;
}

static struct device_node*
scan_OF_childs_for_device(struct device_node* node, u8 bus, u8 dev_fn)
{
	u8 filter_data[2] = {bus, dev_fn};

	return scan_OF_pci_childs(node, scan_OF_pci_childs_iterator, filter_data);
}

/*
 * Scans the OF tree for a device node matching a PCI device
 */
struct device_node *
pci_busdev_to_OF_node(struct pci_bus *bus, int devfn)
{
	struct pci_controller *hose;
	struct device_node *node;
	int busnr;

	if (!have_of)
		return NULL;
	
	/* Lookup the hose */
	busnr = bus->number;
	hose = pci_bus_to_hose(busnr);
	if (!hose)
		return NULL;

	/* Check it has an OF node associated */
	node = (struct device_node *) hose->arch_data;
	if (!node)
		return NULL;

	/* Fixup bus number according to what OF think it is. */
#ifdef CONFIG_PPC_PMAC
	/* The G5 need a special case here. Basically, we don't remap all
	 * busses on it so we don't create the pci-OF-map. However, we do
	 * remap the AGP bus and so have to deal with it. A future better
	 * fix has to be done by making the remapping per-host and always
	 * filling the pci_to_OF map. --BenH
	 */
	if (machine_is(powermac) && busnr >= 0xf0)
		busnr -= 0xf0;
	else
#endif
	if (pci_to_OF_bus_map)
		busnr = pci_to_OF_bus_map[busnr];
	if (busnr == 0xff)
		return NULL;
	
	/* Now, lookup childs of the hose */
	return scan_OF_childs_for_device(node->child, busnr, devfn);
}
EXPORT_SYMBOL(pci_busdev_to_OF_node);

struct device_node*
pci_device_to_OF_node(struct pci_dev *dev)
{
	return pci_busdev_to_OF_node(dev->bus, dev->devfn);
}
EXPORT_SYMBOL(pci_device_to_OF_node);

/* This routine is meant to be used early during boot, when the
 * PCI bus numbers have not yet been assigned, and you need to
 * issue PCI config cycles to an OF device.
 * It could also be used to "fix" RTAS config cycles if you want
 * to set pci_assign_all_buses to 1 and still use RTAS for PCI
 * config cycles.
 */
struct pci_controller* pci_find_hose_for_OF_device(struct device_node* node)
{
	if (!have_of)
		return NULL;
	while(node) {
		struct pci_controller* hose;
		for (hose=hose_head;hose;hose=hose->next)
			if (hose->arch_data == node)
				return hose;
		node=node->parent;
	}
	return NULL;
}

static int
find_OF_pci_device_filter(struct device_node* node, void* data)
{
	return ((void *)node == data);
}

/*
 * Returns the PCI device matching a given OF node
 */
int
pci_device_from_OF_node(struct device_node* node, u8* bus, u8* devfn)
{
	unsigned int *reg;
	struct pci_controller* hose;
	struct pci_dev* dev = NULL;
	
	if (!have_of)
		return -ENODEV;
	/* Make sure it's really a PCI device */
	hose = pci_find_hose_for_OF_device(node);
	if (!hose || !hose->arch_data)
		return -ENODEV;
	if (!scan_OF_pci_childs(((struct device_node*)hose->arch_data)->child,
			find_OF_pci_device_filter, (void *)node))
		return -ENODEV;
	reg = (unsigned int *) get_property(node, "reg", NULL);
	if (!reg)
		return -ENODEV;
	*bus = (reg[0] >> 16) & 0xff;
	*devfn = ((reg[0] >> 8) & 0xff);

	/* Ok, here we need some tweak. If we have already renumbered
	 * all busses, we can't rely on the OF bus number any more.
	 * the pci_to_OF_bus_map is not enough as several PCI busses
	 * may match the same OF bus number.
	 */
	if (!pci_to_OF_bus_map)
		return 0;

	for_each_pci_dev(dev)
		if (pci_to_OF_bus_map[dev->bus->number] == *bus &&
				dev->devfn == *devfn) {
			*bus = dev->bus->number;
			pci_dev_put(dev);
			return 0;
		}

	return -ENODEV;
}
EXPORT_SYMBOL(pci_device_from_OF_node);

void __init
pci_process_bridge_OF_ranges(struct pci_controller *hose,
			   struct device_node *dev, int primary)
{
	static unsigned int static_lc_ranges[256] __initdata;
	unsigned int *dt_ranges, *lc_ranges, *ranges, *prev;
	unsigned int size;
	int rlen = 0, orig_rlen;
	int memno = 0;
	struct resource *res;
	int np, na = prom_n_addr_cells(dev);
	np = na + 5;

	/* First we try to merge ranges to fix a problem with some pmacs
	 * that can have more than 3 ranges, fortunately using contiguous
	 * addresses -- BenH
	 */
	dt_ranges = (unsigned int *) get_property(dev, "ranges", &rlen);
	if (!dt_ranges)
		return;
	/* Sanity check, though hopefully that never happens */
	if (rlen > sizeof(static_lc_ranges)) {
		printk(KERN_WARNING "OF ranges property too large !\n");
		rlen = sizeof(static_lc_ranges);
	}
	lc_ranges = static_lc_ranges;
	memcpy(lc_ranges, dt_ranges, rlen);
	orig_rlen = rlen;

	/* Let's work on a copy of the "ranges" property instead of damaging
	 * the device-tree image in memory
	 */
	ranges = lc_ranges;
	prev = NULL;
	while ((rlen -= np * sizeof(unsigned int)) >= 0) {
		if (prev) {
			if (prev[0] == ranges[0] && prev[1] == ranges[1] &&
				(prev[2] + prev[na+4]) == ranges[2] &&
				(prev[na+2] + prev[na+4]) == ranges[na+2]) {
				prev[na+4] += ranges[na+4];
				ranges[0] = 0;
				ranges += np;
				continue;
			}
		}
		prev = ranges;
		ranges += np;
	}

	/*
	 * The ranges property is laid out as an array of elements,
	 * each of which comprises:
	 *   cells 0 - 2:	a PCI address
	 *   cells 3 or 3+4:	a CPU physical address
	 *			(size depending on dev->n_addr_cells)
	 *   cells 4+5 or 5+6:	the size of the range
	 */
	ranges = lc_ranges;
	rlen = orig_rlen;
	while (ranges && (rlen -= np * sizeof(unsigned int)) >= 0) {
		res = NULL;
		size = ranges[na+4];
		switch ((ranges[0] >> 24) & 0x3) {
		case 1:		/* I/O space */
			if (ranges[2] != 0)
				break;
			hose->io_base_phys = ranges[na+2];
			/* limit I/O space to 16MB */
			if (size > 0x01000000)
				size = 0x01000000;
			hose->io_base_virt = ioremap(ranges[na+2], size);
			if (primary)
				isa_io_base = (unsigned long) hose->io_base_virt;
			res = &hose->io_resource;
			res->flags = IORESOURCE_IO;
			res->start = ranges[2];
			DBG("PCI: IO 0x%llx -> 0x%llx\n",
				    res->start, res->start + size - 1);
			break;
		case 2:		/* memory space */
			memno = 0;
			if (ranges[1] == 0 && ranges[2] == 0
			    && ranges[na+4] <= (16 << 20)) {
				/* 1st 16MB, i.e. ISA memory area */
				if (primary)
					isa_mem_base = ranges[na+2];
				memno = 1;
			}
			while (memno < 3 && hose->mem_resources[memno].flags)
				++memno;
			if (memno == 0)
				hose->pci_mem_offset = ranges[na+2] - ranges[2];
			if (memno < 3) {
				res = &hose->mem_resources[memno];
				res->flags = IORESOURCE_MEM;
				if(ranges[0] & 0x40000000)
					res->flags |= IORESOURCE_PREFETCH;
				res->start = ranges[na+2];
				DBG("PCI: MEM[%d] 0x%llx -> 0x%llx\n", memno,
					    res->start, res->start + size - 1);
			}
			break;
		}
		if (res != NULL) {
			res->name = dev->full_name;
			res->end = res->start + size - 1;
			res->parent = NULL;
			res->sibling = NULL;
			res->child = NULL;
		}
		ranges += np;
	}
}

/* We create the "pci-OF-bus-map" property now so it appears in the
 * /proc device tree
 */
void __init
pci_create_OF_bus_map(void)
{
	struct property* of_prop;
	
	of_prop = (struct property*) alloc_bootmem(sizeof(struct property) + 256);
	if (of_prop && find_path_device("/")) {
		memset(of_prop, -1, sizeof(struct property) + 256);
		of_prop->name = "pci-OF-bus-map";
		of_prop->length = 256;
		of_prop->value = (unsigned char *)&of_prop[1];
		prom_add_property(find_path_device("/"), of_prop);
	}
}

static ssize_t pci_show_devspec(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct pci_dev *pdev;
	struct device_node *np;

	pdev = to_pci_dev (dev);
	np = pci_device_to_OF_node(pdev);
	if (np == NULL || np->full_name == NULL)
		return 0;
	return sprintf(buf, "%s", np->full_name);
}
static DEVICE_ATTR(devspec, S_IRUGO, pci_show_devspec, NULL);

#else /* CONFIG_PPC_OF */
void pcibios_make_OF_bus_map(void)
{
}
#endif /* CONFIG_PPC_OF */

/* Add sysfs properties */
void pcibios_add_platform_entries(struct pci_dev *pdev)
{
#ifdef CONFIG_PPC_OF
	device_create_file(&pdev->dev, &dev_attr_devspec);
#endif /* CONFIG_PPC_OF */
}


#ifdef CONFIG_PPC_PMAC
/*
 * This set of routines checks for PCI<->PCI bridges that have closed
 * IO resources and have child devices. It tries to re-open an IO
 * window on them.
 *
 * This is a _temporary_ fix to workaround a problem with Apple's OF
 * closing IO windows on P2P bridges when the OF drivers of cards
 * below this bridge don't claim any IO range (typically ATI or
 * Adaptec).
 *
 * A more complete fix would be to use drivers/pci/setup-bus.c, which
 * involves a working pcibios_fixup_pbus_ranges(), some more care about
 * ordering when creating the host bus resources, and maybe a few more
 * minor tweaks
 */

/* Initialize bridges with base/limit values we have collected */
static void __init
do_update_p2p_io_resource(struct pci_bus *bus, int enable_vga)
{
	struct pci_dev *bridge = bus->self;
	struct pci_controller* hose = (struct pci_controller *)bridge->sysdata;
	u32 l;
	u16 w;
	struct resource res;

	if (bus->resource[0] == NULL)
		return;
 	res = *(bus->resource[0]);

	DBG("Remapping Bus %d, bridge: %s\n", bus->number, pci_name(bridge));
	res.start -= ((unsigned long) hose->io_base_virt - isa_io_base);
	res.end -= ((unsigned long) hose->io_base_virt - isa_io_base);
	DBG("  IO window: %016llx-%016llx\n", res.start, res.end);

	/* Set up the top and bottom of the PCI I/O segment for this bus. */
	pci_read_config_dword(bridge, PCI_IO_BASE, &l);
	l &= 0xffff000f;
	l |= (res.start >> 8) & 0x00f0;
	l |= res.end & 0xf000;
	pci_write_config_dword(bridge, PCI_IO_BASE, l);

	if ((l & PCI_IO_RANGE_TYPE_MASK) == PCI_IO_RANGE_TYPE_32) {
		l = (res.start >> 16) | (res.end & 0xffff0000);
		pci_write_config_dword(bridge, PCI_IO_BASE_UPPER16, l);
	}

	pci_read_config_word(bridge, PCI_COMMAND, &w);
	w |= PCI_COMMAND_IO;
	pci_write_config_word(bridge, PCI_COMMAND, w);

#if 0 /* Enabling this causes XFree 4.2.0 to hang during PCI probe */
	if (enable_vga) {
		pci_read_config_word(bridge, PCI_BRIDGE_CONTROL, &w);
		w |= PCI_BRIDGE_CTL_VGA;
		pci_write_config_word(bridge, PCI_BRIDGE_CONTROL, w);
	}
#endif
}

/* This function is pretty basic and actually quite broken for the
 * general case, it's enough for us right now though. It's supposed
 * to tell us if we need to open an IO range at all or not and what
 * size.
 */
static int __init
check_for_io_childs(struct pci_bus *bus, struct resource* res, int *found_vga)
{
	struct pci_dev *dev;
	int	i;
	int	rc = 0;

#define push_end(res, mask) do {		\
	BUG_ON((mask+1) & mask);		\
	res->end = (res->end + mask) | mask;	\
} while (0)

	list_for_each_entry(dev, &bus->devices, bus_list) {
		u16 class = dev->class >> 8;

		if (class == PCI_CLASS_DISPLAY_VGA ||
		    class == PCI_CLASS_NOT_DEFINED_VGA)
			*found_vga = 1;
		if (class >> 8 == PCI_BASE_CLASS_BRIDGE && dev->subordinate)
			rc |= check_for_io_childs(dev->subordinate, res, found_vga);
		if (class == PCI_CLASS_BRIDGE_CARDBUS)
			push_end(res, 0xfff);

		for (i=0; i<PCI_NUM_RESOURCES; i++) {
			struct resource *r;
			unsigned long r_size;

			if (dev->class >> 8 == PCI_CLASS_BRIDGE_PCI
			    && i >= PCI_BRIDGE_RESOURCES)
				continue;
			r = &dev->resource[i];
			r_size = r->end - r->start;
			if (r_size < 0xfff)
				r_size = 0xfff;
			if (r->flags & IORESOURCE_IO && (r_size) != 0) {
				rc = 1;
				push_end(res, r_size);
			}
		}
	}

	return rc;
}

/* Here we scan all P2P bridges of a given level that have a closed
 * IO window. Note that the test for the presence of a VGA card should
 * be improved to take into account already configured P2P bridges,
 * currently, we don't see them and might end up configuring 2 bridges
 * with VGA pass through enabled
 */
static void __init
do_fixup_p2p_level(struct pci_bus *bus)
{
	struct pci_bus *b;
	int i, parent_io;
	int has_vga = 0;

	for (parent_io=0; parent_io<4; parent_io++)
		if (bus->resource[parent_io]
		    && bus->resource[parent_io]->flags & IORESOURCE_IO)
			break;
	if (parent_io >= 4)
		return;

	list_for_each_entry(b, &bus->children, node) {
		struct pci_dev *d = b->self;
		struct pci_controller* hose = (struct pci_controller *)d->sysdata;
		struct resource *res = b->resource[0];
		struct resource tmp_res;
		unsigned long max;
		int found_vga = 0;

		memset(&tmp_res, 0, sizeof(tmp_res));
		tmp_res.start = bus->resource[parent_io]->start;

		/* We don't let low addresses go through that closed P2P bridge, well,
		 * that may not be necessary but I feel safer that way
		 */
		if (tmp_res.start == 0)
			tmp_res.start = 0x1000;
	
		if (!list_empty(&b->devices) && res && res->flags == 0 &&
		    res != bus->resource[parent_io] &&
		    (d->class >> 8) == PCI_CLASS_BRIDGE_PCI &&
		    check_for_io_childs(b, &tmp_res, &found_vga)) {
			u8 io_base_lo;

			printk(KERN_INFO "Fixing up IO bus %s\n", b->name);

			if (found_vga) {
				if (has_vga) {
					printk(KERN_WARNING "Skipping VGA, already active"
					    " on bus segment\n");
					found_vga = 0;
				} else
					has_vga = 1;
			}
			pci_read_config_byte(d, PCI_IO_BASE, &io_base_lo);

			if ((io_base_lo & PCI_IO_RANGE_TYPE_MASK) == PCI_IO_RANGE_TYPE_32)
				max = ((unsigned long) hose->io_base_virt
					- isa_io_base) + 0xffffffff;
			else
				max = ((unsigned long) hose->io_base_virt
					- isa_io_base) + 0xffff;

			*res = tmp_res;
			res->flags = IORESOURCE_IO;
			res->name = b->name;
		
			/* Find a resource in the parent where we can allocate */
			for (i = 0 ; i < 4; i++) {
				struct resource *r = bus->resource[i];
				if (!r)
					continue;
				if ((r->flags & IORESOURCE_IO) == 0)
					continue;
				DBG("Trying to allocate from %016llx, size %016llx from parent"
				    " res %d: %016llx -> %016llx\n",
					res->start, res->end, i, r->start, r->end);
			
				if (allocate_resource(r, res, res->end + 1, res->start, max,
				    res->end + 1, NULL, NULL) < 0) {
					DBG("Failed !\n");
					continue;
				}
				do_update_p2p_io_resource(b, found_vga);
				break;
			}
		}
		do_fixup_p2p_level(b);
	}
}

static void
pcibios_fixup_p2p_bridges(void)
{
	struct pci_bus *b;

	list_for_each_entry(b, &pci_root_buses, node)
		do_fixup_p2p_level(b);
}

#endif /* CONFIG_PPC_PMAC */

static int __init
pcibios_init(void)
{
	struct pci_controller *hose;
	struct pci_bus *bus;
	int next_busno;

	printk(KERN_INFO "PCI: Probing PCI hardware\n");

	/* Scan all of the recorded PCI controllers.  */
	for (next_busno = 0, hose = hose_head; hose; hose = hose->next) {
		if (pci_assign_all_buses)
			hose->first_busno = next_busno;
		hose->last_busno = 0xff;
		bus = pci_scan_bus(hose->first_busno, hose->ops, hose);
		hose->last_busno = bus->subordinate;
		if (pci_assign_all_buses || next_busno <= hose->last_busno)
			next_busno = hose->last_busno + pcibios_assign_bus_offset;
	}
	pci_bus_count = next_busno;

	/* OpenFirmware based machines need a map of OF bus
	 * numbers vs. kernel bus numbers since we may have to
	 * remap them.
	 */
	if (pci_assign_all_buses && have_of)
		pcibios_make_OF_bus_map();

	/* Do machine dependent PCI interrupt routing */
	if (ppc_md.pci_swizzle && ppc_md.pci_map_irq)
		pci_fixup_irqs(ppc_md.pci_swizzle, ppc_md.pci_map_irq);

	/* Call machine dependent fixup */
	if (ppc_md.pcibios_fixup)
		ppc_md.pcibios_fixup();

	/* Allocate and assign resources */
	pcibios_allocate_bus_resources(&pci_root_buses);
	pcibios_allocate_resources(0);
	pcibios_allocate_resources(1);
#ifdef CONFIG_PPC_PMAC
	pcibios_fixup_p2p_bridges();
#endif /* CONFIG_PPC_PMAC */
	pcibios_assign_resources();

	/* Call machine dependent post-init code */
	if (ppc_md.pcibios_after_init)
		ppc_md.pcibios_after_init();

	return 0;
}

subsys_initcall(pcibios_init);

unsigned char __init
common_swizzle(struct pci_dev *dev, unsigned char *pinp)
{
	struct pci_controller *hose = dev->sysdata;

	if (dev->bus->number != hose->first_busno) {
		u8 pin = *pinp;
		do {
			pin = bridge_swizzle(pin, PCI_SLOT(dev->devfn));
			/* Move up the chain of bridges. */
			dev = dev->bus->self;
		} while (dev->bus->self);
		*pinp = pin;

		/* The slot is the idsel of the last bridge. */
	}
	return PCI_SLOT(dev->devfn);
}

unsigned long resource_fixup(struct pci_dev * dev, struct resource * res,
			     unsigned long start, unsigned long size)
{
	return start;
}

void __init pcibios_fixup_bus(struct pci_bus *bus)
{
	struct pci_controller *hose = (struct pci_controller *) bus->sysdata;
	unsigned long io_offset;
	struct resource *res;
	int i;

	io_offset = (unsigned long)hose->io_base_virt - isa_io_base;
	if (bus->parent == NULL) {
		/* This is a host bridge - fill in its resources */
		hose->bus = bus;

		bus->resource[0] = res = &hose->io_resource;
		if (!res->flags) {
			if (io_offset)
				printk(KERN_ERR "I/O resource not set for host"
				       " bridge %d\n", hose->index);
			res->start = 0;
			res->end = IO_SPACE_LIMIT;
			res->flags = IORESOURCE_IO;
		}
		res->start += io_offset;
		res->end += io_offset;

		for (i = 0; i < 3; ++i) {
			res = &hose->mem_resources[i];
			if (!res->flags) {
				if (i > 0)
					continue;
				printk(KERN_ERR "Memory resource not set for "
				       "host bridge %d\n", hose->index);
				res->start = hose->pci_mem_offset;
				res->end = ~0U;
				res->flags = IORESOURCE_MEM;
			}
			bus->resource[i+1] = res;
		}
	} else {
		/* This is a subordinate bridge */
		pci_read_bridge_bases(bus);

		for (i = 0; i < 4; ++i) {
			if ((res = bus->resource[i]) == NULL)
				continue;
			if (!res->flags)
				continue;
			if (io_offset && (res->flags & IORESOURCE_IO)) {
				res->start += io_offset;
				res->end += io_offset;
			} else if (hose->pci_mem_offset
				   && (res->flags & IORESOURCE_MEM)) {
				res->start += hose->pci_mem_offset;
				res->end += hose->pci_mem_offset;
			}
		}
	}

	if (ppc_md.pcibios_fixup_bus)
		ppc_md.pcibios_fixup_bus(bus);
}

char __init *pcibios_setup(char *str)
{
	return str;
}

/* the next one is stolen from the alpha port... */
void __init
pcibios_update_irq(struct pci_dev *dev, int irq)
{
	pci_write_config_byte(dev, PCI_INTERRUPT_LINE, irq);
	/* XXX FIXME - update OF device tree node interrupt property */
}

#ifdef CONFIG_PPC_MERGE
/* XXX This is a copy of the ppc64 version. This is temporary until we start
 * merging the 2 PCI layers
 */
/*
 * Reads the interrupt pin to determine if interrupt is use by card.
 * If the interrupt is used, then gets the interrupt line from the
 * openfirmware and sets it in the pci_dev and pci_config line.
 */
int pci_read_irq_line(struct pci_dev *pci_dev)
{
	struct of_irq oirq;
	unsigned int virq;

	DBG("Try to map irq for %s...\n", pci_name(pci_dev));

	/* Try to get a mapping from the device-tree */
	if (of_irq_map_pci(pci_dev, &oirq)) {
		u8 line, pin;

		/* If that fails, lets fallback to what is in the config
		 * space and map that through the default controller. We
		 * also set the type to level low since that's what PCI
		 * interrupts are. If your platform does differently, then
		 * either provide a proper interrupt tree or don't use this
		 * function.
		 */
		if (pci_read_config_byte(pci_dev, PCI_INTERRUPT_PIN, &pin))
			return -1;
		if (pin == 0)
			return -1;
		if (pci_read_config_byte(pci_dev, PCI_INTERRUPT_LINE, &line) ||
		    line == 0xff) {
			return -1;
		}
		DBG(" -> no map ! Using irq line %d from PCI config\n", line);

		virq = irq_create_mapping(NULL, line);
		if (virq != NO_IRQ)
			set_irq_type(virq, IRQ_TYPE_LEVEL_LOW);
	} else {
		DBG(" -> got one, spec %d cells (0x%08x...) on %s\n",
		    oirq.size, oirq.specifier[0], oirq.controller->full_name);

		virq = irq_create_of_mapping(oirq.controller, oirq.specifier,
					     oirq.size);
	}
	if(virq == NO_IRQ) {
		DBG(" -> failed to map !\n");
		return -1;
	}
	pci_dev->irq = virq;
	pci_write_config_byte(pci_dev, PCI_INTERRUPT_LINE, virq);

	return 0;
}
EXPORT_SYMBOL(pci_read_irq_line);
#endif /* CONFIG_PPC_MERGE */

int pcibios_enable_device(struct pci_dev *dev, int mask)
{
	u16 cmd, old_cmd;
	int idx;
	struct resource *r;

	if (ppc_md.pcibios_enable_device_hook)
		if (ppc_md.pcibios_enable_device_hook(dev, 0))
			return -EINVAL;
		
	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	old_cmd = cmd;
	for (idx=0; idx<6; idx++) {
		r = &dev->resource[idx];
		if (r->flags & IORESOURCE_UNSET) {
			printk(KERN_ERR "PCI: Device %s not available because of resource collisions\n", pci_name(dev));
			return -EINVAL;
		}
		if (r->flags & IORESOURCE_IO)
			cmd |= PCI_COMMAND_IO;
		if (r->flags & IORESOURCE_MEM)
			cmd |= PCI_COMMAND_MEMORY;
	}
	if (cmd != old_cmd) {
		printk("PCI: Enabling device %s (%04x -> %04x)\n",
		       pci_name(dev), old_cmd, cmd);
		pci_write_config_word(dev, PCI_COMMAND, cmd);
	}
	return 0;
}

struct pci_controller*
pci_bus_to_hose(int bus)
{
	struct pci_controller* hose = hose_head;

	for (; hose; hose = hose->next)
		if (bus >= hose->first_busno && bus <= hose->last_busno)
			return hose;
	return NULL;
}

void __iomem *
pci_bus_io_base(unsigned int bus)
{
	struct pci_controller *hose;

	hose = pci_bus_to_hose(bus);
	if (!hose)
		return NULL;
	return hose->io_base_virt;
}

unsigned long
pci_bus_io_base_phys(unsigned int bus)
{
	struct pci_controller *hose;

	hose = pci_bus_to_hose(bus);
	if (!hose)
		return 0;
	return hose->io_base_phys;
}

unsigned long
pci_bus_mem_base_phys(unsigned int bus)
{
	struct pci_controller *hose;

	hose = pci_bus_to_hose(bus);
	if (!hose)
		return 0;
	return hose->pci_mem_offset;
}

unsigned long
pci_resource_to_bus(struct pci_dev *pdev, struct resource *res)
{
	/* Hack alert again ! See comments in chrp_pci.c
	 */
	struct pci_controller* hose =
		(struct pci_controller *)pdev->sysdata;
	if (hose && res->flags & IORESOURCE_MEM)
		return res->start - hose->pci_mem_offset;
	/* We may want to do something with IOs here... */
	return res->start;
}


static struct resource *__pci_mmap_make_offset(struct pci_dev *dev,
					       unsigned long *offset,
					       enum pci_mmap_state mmap_state)
{
	struct pci_controller *hose = pci_bus_to_hose(dev->bus->number);
	unsigned long io_offset = 0;
	int i, res_bit;

	if (hose == 0)
		return NULL;		/* should never happen */

	/* If memory, add on the PCI bridge address offset */
	if (mmap_state == pci_mmap_mem) {
		*offset += hose->pci_mem_offset;
		res_bit = IORESOURCE_MEM;
	} else {
		io_offset = hose->io_base_virt - ___IO_BASE;
		*offset += io_offset;
		res_bit = IORESOURCE_IO;
	}

	/*
	 * Check that the offset requested corresponds to one of the
	 * resources of the device.
	 */
	for (i = 0; i <= PCI_ROM_RESOURCE; i++) {
		struct resource *rp = &dev->resource[i];
		int flags = rp->flags;

		/* treat ROM as memory (should be already) */
		if (i == PCI_ROM_RESOURCE)
			flags |= IORESOURCE_MEM;

		/* Active and same type? */
		if ((flags & res_bit) == 0)
			continue;

		/* In the range of this resource? */
		if (*offset < (rp->start & PAGE_MASK) || *offset > rp->end)
			continue;

		/* found it! construct the final physical address */
		if (mmap_state == pci_mmap_io)
			*offset += hose->io_base_phys - io_offset;
		return rp;
	}

	return NULL;
}

/*
 * Set vm_page_prot of VMA, as appropriate for this architecture, for a pci
 * device mapping.
 */
static pgprot_t __pci_mmap_set_pgprot(struct pci_dev *dev, struct resource *rp,
				      pgprot_t protection,
				      enum pci_mmap_state mmap_state,
				      int write_combine)
{
	unsigned long prot = pgprot_val(protection);

	/* Write combine is always 0 on non-memory space mappings. On
	 * memory space, if the user didn't pass 1, we check for a
	 * "prefetchable" resource. This is a bit hackish, but we use
	 * this to workaround the inability of /sysfs to provide a write
	 * combine bit
	 */
	if (mmap_state != pci_mmap_mem)
		write_combine = 0;
	else if (write_combine == 0) {
		if (rp->flags & IORESOURCE_PREFETCH)
			write_combine = 1;
	}

	/* XXX would be nice to have a way to ask for write-through */
	prot |= _PAGE_NO_CACHE;
	if (write_combine)
		prot &= ~_PAGE_GUARDED;
	else
		prot |= _PAGE_GUARDED;

	printk("PCI map for %s:%llx, prot: %lx\n", pci_name(dev),
		(unsigned long long)rp->start, prot);

	return __pgprot(prot);
}

/*
 * This one is used by /dev/mem and fbdev who have no clue about the
 * PCI device, it tries to find the PCI device first and calls the
 * above routine
 */
pgprot_t pci_phys_mem_access_prot(struct file *file,
				  unsigned long pfn,
				  unsigned long size,
				  pgprot_t protection)
{
	struct pci_dev *pdev = NULL;
	struct resource *found = NULL;
	unsigned long prot = pgprot_val(protection);
	unsigned long offset = pfn << PAGE_SHIFT;
	int i;

	if (page_is_ram(pfn))
		return prot;

	prot |= _PAGE_NO_CACHE | _PAGE_GUARDED;

	for_each_pci_dev(pdev) {
		for (i = 0; i <= PCI_ROM_RESOURCE; i++) {
			struct resource *rp = &pdev->resource[i];
			int flags = rp->flags;

			/* Active and same type? */
			if ((flags & IORESOURCE_MEM) == 0)
				continue;
			/* In the range of this resource? */
			if (offset < (rp->start & PAGE_MASK) ||
			    offset > rp->end)
				continue;
			found = rp;
			break;
		}
		if (found)
			break;
	}
	if (found) {
		if (found->flags & IORESOURCE_PREFETCH)
			prot &= ~_PAGE_GUARDED;
		pci_dev_put(pdev);
	}

	DBG("non-PCI map for %lx, prot: %lx\n", offset, prot);

	return __pgprot(prot);
}


/*
 * Perform the actual remap of the pages for a PCI device mapping, as
 * appropriate for this architecture.  The region in the process to map
 * is described by vm_start and vm_end members of VMA, the base physical
 * address is found in vm_pgoff.
 * The pci device structure is provided so that architectures may make mapping
 * decisions on a per-device or per-bus basis.
 *
 * Returns a negative error code on failure, zero on success.
 */
int pci_mmap_page_range(struct pci_dev *dev, struct vm_area_struct *vma,
			enum pci_mmap_state mmap_state,
			int write_combine)
{
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	struct resource *rp;
	int ret;

	rp = __pci_mmap_make_offset(dev, &offset, mmap_state);
	if (rp == NULL)
		return -EINVAL;

	vma->vm_pgoff = offset >> PAGE_SHIFT;
	vma->vm_page_prot = __pci_mmap_set_pgprot(dev, rp,
						  vma->vm_page_prot,
						  mmap_state, write_combine);

	ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			       vma->vm_end - vma->vm_start, vma->vm_page_prot);

	return ret;
}

/* Obsolete functions. Should be removed once the symbios driver
 * is fixed
 */
unsigned long
phys_to_bus(unsigned long pa)
{
	struct pci_controller *hose;
	int i;

	for (hose = hose_head; hose; hose = hose->next) {
		for (i = 0; i < 3; ++i) {
			if (pa >= hose->mem_resources[i].start
			    && pa <= hose->mem_resources[i].end) {
				/*
				 * XXX the hose->pci_mem_offset really
				 * only applies to mem_resources[0].
				 * We need a way to store an offset for
				 * the others.  -- paulus
				 */
				if (i == 0)
					pa -= hose->pci_mem_offset;
				return pa;
			}
		}
	}
	/* hmmm, didn't find it */
	return 0;
}

unsigned long
pci_phys_to_bus(unsigned long pa, int busnr)
{
	struct pci_controller* hose = pci_bus_to_hose(busnr);
	if (!hose)
		return pa;
	return pa - hose->pci_mem_offset;
}

unsigned long
pci_bus_to_phys(unsigned int ba, int busnr)
{
	struct pci_controller* hose = pci_bus_to_hose(busnr);
	if (!hose)
		return ba;
	return ba + hose->pci_mem_offset;
}

/* Provide information on locations of various I/O regions in physical
 * memory.  Do this on a per-card basis so that we choose the right
 * root bridge.
 * Note that the returned IO or memory base is a physical address
 */

long sys_pciconfig_iobase(long which, unsigned long bus, unsigned long devfn)
{
	struct pci_controller* hose;
	long result = -EOPNOTSUPP;

	/* Argh ! Please forgive me for that hack, but that's the
	 * simplest way to get existing XFree to not lockup on some
	 * G5 machines... So when something asks for bus 0 io base
	 * (bus 0 is HT root), we return the AGP one instead.
	 */
#ifdef CONFIG_PPC_PMAC
	if (machine_is(powermac) && machine_is_compatible("MacRISC4"))
		if (bus == 0)
			bus = 0xf0;
#endif /* CONFIG_PPC_PMAC */

	hose = pci_bus_to_hose(bus);
	if (!hose)
		return -ENODEV;

	switch (which) {
	case IOBASE_BRIDGE_NUMBER:
		return (long)hose->first_busno;
	case IOBASE_MEMORY:
		return (long)hose->pci_mem_offset;
	case IOBASE_IO:
		return (long)hose->io_base_phys;
	case IOBASE_ISA_IO:
		return (long)isa_io_base;
	case IOBASE_ISA_MEM:
		return (long)isa_mem_base;
	}

	return result;
}

void pci_resource_to_user(const struct pci_dev *dev, int bar,
			  const struct resource *rsrc,
			  resource_size_t *start, resource_size_t *end)
{
	struct pci_controller *hose = pci_bus_to_hose(dev->bus->number);
	unsigned long offset = 0;

	if (hose == NULL)
		return;

	if (rsrc->flags & IORESOURCE_IO)
		offset = ___IO_BASE - hose->io_base_virt + hose->io_base_phys;

	*start = rsrc->start + offset;
	*end = rsrc->end + offset;
}

void __init
pci_init_resource(struct resource *res, unsigned long start, unsigned long end,
		  int flags, char *name)
{
	res->start = start;
	res->end = end;
	res->flags = flags;
	res->name = name;
	res->parent = NULL;
	res->sibling = NULL;
	res->child = NULL;
}

void __iomem *pci_iomap(struct pci_dev *dev, int bar, unsigned long max)
{
	unsigned long start = pci_resource_start(dev, bar);
	unsigned long len = pci_resource_len(dev, bar);
	unsigned long flags = pci_resource_flags(dev, bar);

	if (!len)
		return NULL;
	if (max && len > max)
		len = max;
	if (flags & IORESOURCE_IO)
		return ioport_map(start, len);
	if (flags & IORESOURCE_MEM)
		/* Not checking IORESOURCE_CACHEABLE because PPC does
		 * not currently distinguish between ioremap and
		 * ioremap_nocache.
		 */
		return ioremap(start, len);
	/* What? */
	return NULL;
}

void pci_iounmap(struct pci_dev *dev, void __iomem *addr)
{
	/* Nothing to do */
}
EXPORT_SYMBOL(pci_iomap);
EXPORT_SYMBOL(pci_iounmap);

unsigned long pci_address_to_pio(phys_addr_t address)
{
	struct pci_controller* hose = hose_head;

	for (; hose; hose = hose->next) {
		unsigned int size = hose->io_resource.end -
			hose->io_resource.start + 1;
		if (address >= hose->io_base_phys &&
		    address < (hose->io_base_phys + size)) {
			unsigned long base =
				(unsigned long)hose->io_base_virt - _IO_BASE;
			return base + (address - hose->io_base_phys);
		}
	}
	return (unsigned int)-1;
}
EXPORT_SYMBOL(pci_address_to_pio);

/*
 * Null PCI config access functions, for the case when we can't
 * find a hose.
 */
#define NULL_PCI_OP(rw, size, type)					\
static int								\
null_##rw##_config_##size(struct pci_dev *dev, int offset, type val)	\
{									\
	return PCIBIOS_DEVICE_NOT_FOUND;    				\
}

static int
null_read_config(struct pci_bus *bus, unsigned int devfn, int offset,
		 int len, u32 *val)
{
	return PCIBIOS_DEVICE_NOT_FOUND;
}

static int
null_write_config(struct pci_bus *bus, unsigned int devfn, int offset,
		  int len, u32 val)
{
	return PCIBIOS_DEVICE_NOT_FOUND;
}

static struct pci_ops null_pci_ops =
{
	null_read_config,
	null_write_config
};

/*
 * These functions are used early on before PCI scanning is done
 * and all of the pci_dev and pci_bus structures have been created.
 */
static struct pci_bus *
fake_pci_bus(struct pci_controller *hose, int busnr)
{
	static struct pci_bus bus;

	if (hose == 0) {
		hose = pci_bus_to_hose(busnr);
		if (hose == 0)
			printk(KERN_ERR "Can't find hose for PCI bus %d!\n", busnr);
	}
	bus.number = busnr;
	bus.sysdata = hose;
	bus.ops = hose? hose->ops: &null_pci_ops;
	return &bus;
}

#define EARLY_PCI_OP(rw, size, type)					\
int early_##rw##_config_##size(struct pci_controller *hose, int bus,	\
			       int devfn, int offset, type value)	\
{									\
	return pci_bus_##rw##_config_##size(fake_pci_bus(hose, bus),	\
					    devfn, offset, value);	\
}

EARLY_PCI_OP(read, byte, u8 *)
EARLY_PCI_OP(read, word, u16 *)
EARLY_PCI_OP(read, dword, u32 *)
EARLY_PCI_OP(write, byte, u8)
EARLY_PCI_OP(write, word, u16)
EARLY_PCI_OP(write, dword, u32)
