/*
 * Copyright 2010 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */

#include <core/device.h>
#include <core/gpuobj.h>

#include <subdev/timer.h>
#include <subdev/fb.h>
#include <subdev/vm.h>

struct nvc0_vmmgr_priv {
	struct nouveau_vmmgr base;
	spinlock_t lock;
};

void
nvc0_vm_map_pgt(struct nouveau_gpuobj *pgd, u32 index,
		struct nouveau_gpuobj *pgt[2])
{
	u32 pde[2] = { 0, 0 };

	if (pgt[0])
		pde[1] = 0x00000001 | (pgt[0]->addr >> 8);
	if (pgt[1])
		pde[0] = 0x00000001 | (pgt[1]->addr >> 8);

	nv_wo32(pgd, (index * 8) + 0, pde[0]);
	nv_wo32(pgd, (index * 8) + 4, pde[1]);
}

static inline u64
nvc0_vm_addr(struct nouveau_vma *vma, u64 phys, u32 memtype, u32 target)
{
	phys >>= 8;

	phys |= 0x00000001; /* present */
	if (vma->access & NV_MEM_ACCESS_SYS)
		phys |= 0x00000002;

	phys |= ((u64)target  << 32);
	phys |= ((u64)memtype << 36);

	return phys;
}

void
nvc0_vm_map(struct nouveau_vma *vma, struct nouveau_gpuobj *pgt,
	    struct nouveau_mem *mem, u32 pte, u32 cnt, u64 phys, u64 delta)
{
	u32 next = 1 << (vma->node->type - 8);

	phys  = nvc0_vm_addr(vma, phys, mem->memtype, 0);
	pte <<= 3;
	while (cnt--) {
		nv_wo32(pgt, pte + 0, lower_32_bits(phys));
		nv_wo32(pgt, pte + 4, upper_32_bits(phys));
		phys += next;
		pte  += 8;
	}
}

void
nvc0_vm_map_sg(struct nouveau_vma *vma, struct nouveau_gpuobj *pgt,
	       struct nouveau_mem *mem, u32 pte, u32 cnt, dma_addr_t *list)
{
	u32 target = (vma->access & NV_MEM_ACCESS_NOSNOOP) ? 7 : 5;

	pte <<= 3;
	while (cnt--) {
		u64 phys = nvc0_vm_addr(vma, *list++, mem->memtype, target);
		nv_wo32(pgt, pte + 0, lower_32_bits(phys));
		nv_wo32(pgt, pte + 4, upper_32_bits(phys));
		pte += 8;
	}
}

void
nvc0_vm_unmap(struct nouveau_gpuobj *pgt, u32 pte, u32 cnt)
{
	pte <<= 3;
	while (cnt--) {
		nv_wo32(pgt, pte + 0, 0x00000000);
		nv_wo32(pgt, pte + 4, 0x00000000);
		pte += 8;
	}
}

void
nvc0_vm_flush_engine(struct nouveau_subdev *subdev, u64 addr, int type)
{
	struct nvc0_vmmgr_priv *priv = (void *)nouveau_vmmgr(subdev);
	unsigned long flags;

	/* looks like maybe a "free flush slots" counter, the
	 * faster you write to 0x100cbc to more it decreases
	 */
	spin_lock_irqsave(&priv->lock, flags);
	if (!nv_wait_ne(subdev, 0x100c80, 0x00ff0000, 0x00000000)) {
		nv_error(subdev, "vm timeout 0: 0x%08x %d\n",
			 nv_rd32(subdev, 0x100c80), type);
	}

	nv_wr32(subdev, 0x100cb8, addr >> 8);
	nv_wr32(subdev, 0x100cbc, 0x80000000 | type);

	/* wait for flush to be queued? */
	if (!nv_wait(subdev, 0x100c80, 0x00008000, 0x00008000)) {
		nv_error(subdev, "vm timeout 1: 0x%08x %d\n",
			 nv_rd32(subdev, 0x100c80), type);
	}
	spin_unlock_irqrestore(&priv->lock, flags);
}

void
nvc0_vm_flush(struct nouveau_vm *vm)
{
	struct nouveau_vm_pgd *vpgd;

	list_for_each_entry(vpgd, &vm->pgd_list, head) {
		nvc0_vm_flush_engine(nv_subdev(vm->vmm), vpgd->obj->addr, 1);
	}
}

static int
nvc0_vm_create(struct nouveau_vmmgr *vmm, u64 offset, u64 length,
	       u64 mm_offset, struct nouveau_vm **pvm)
{
	return nouveau_vm_create(vmm, offset, length, mm_offset, 4096, pvm);
}

static int
nvc0_vmmgr_ctor(struct nouveau_object *parent, struct nouveau_object *engine,
		struct nouveau_oclass *oclass, void *data, u32 size,
		struct nouveau_object **pobject)
{
	struct nvc0_vmmgr_priv *priv;
	int ret;

	ret = nouveau_vmmgr_create(parent, engine, oclass, "VM", "vm", &priv);
	*pobject = nv_object(priv);
	if (ret)
		return ret;

	priv->base.pgt_bits  = 27 - 12;
	priv->base.spg_shift = 12;
	priv->base.lpg_shift = 17;
	priv->base.create = nvc0_vm_create;
	priv->base.map_pgt = nvc0_vm_map_pgt;
	priv->base.map = nvc0_vm_map;
	priv->base.map_sg = nvc0_vm_map_sg;
	priv->base.unmap = nvc0_vm_unmap;
	priv->base.flush = nvc0_vm_flush;
	spin_lock_init(&priv->lock);
	return 0;
}

struct nouveau_oclass
nvc0_vmmgr_oclass = {
	.handle = NV_SUBDEV(VM, 0xc0),
	.ofuncs = &(struct nouveau_ofuncs) {
		.ctor = nvc0_vmmgr_ctor,
		.dtor = _nouveau_vmmgr_dtor,
		.init = _nouveau_vmmgr_init,
		.fini = _nouveau_vmmgr_fini,
	},
};
