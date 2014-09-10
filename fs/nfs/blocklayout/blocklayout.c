/*
 *  linux/fs/nfs/blocklayout/blocklayout.c
 *
 *  Module for the NFSv4.1 pNFS block layout driver.
 *
 *  Copyright (c) 2006 The Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  Andy Adamson <andros@citi.umich.edu>
 *  Fred Isaman <iisaman@umich.edu>
 *
 * permission is granted to use, copy, create derivative works and
 * redistribute this software and such derivative works for any purpose,
 * so long as the name of the university of michigan is not used in
 * any advertising or publicity pertaining to the use or distribution
 * of this software without specific, written prior authorization.  if
 * the above copyright notice or any other identification of the
 * university of michigan is included in any copy of any portion of
 * this software, then the disclaimer below must also be included.
 *
 * this software is provided as is, without representation from the
 * university of michigan as to its fitness for any purpose, and without
 * warranty by the university of michigan of any kind, either express
 * or implied, including without limitation the implied warranties of
 * merchantability and fitness for a particular purpose.  the regents
 * of the university of michigan shall not be liable for any damages,
 * including special, indirect, incidental, or consequential damages,
 * with respect to any claim arising out or in connection with the use
 * of the software, even if it has been or is hereafter advised of the
 * possibility of such damages.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/bio.h>		/* struct bio */
#include <linux/prefetch.h>
#include <linux/pagevec.h>

#include "../pnfs.h"
#include "../nfs4session.h"
#include "../internal.h"
#include "blocklayout.h"

#define NFSDBG_FACILITY	NFSDBG_PNFS_LD

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andy Adamson <andros@citi.umich.edu>");
MODULE_DESCRIPTION("The NFSv4.1 pNFS Block layout driver");

static bool is_hole(struct pnfs_block_extent *be)
{
	switch (be->be_state) {
	case PNFS_BLOCK_NONE_DATA:
		return true;
	case PNFS_BLOCK_INVALID_DATA:
		return be->be_tag ? false : true;
	default:
		return false;
	}
}

/* The data we are handed might be spread across several bios.  We need
 * to track when the last one is finished.
 */
struct parallel_io {
	struct kref refcnt;
	void (*pnfs_callback) (void *data);
	void *data;
};

static inline struct parallel_io *alloc_parallel(void *data)
{
	struct parallel_io *rv;

	rv  = kmalloc(sizeof(*rv), GFP_NOFS);
	if (rv) {
		rv->data = data;
		kref_init(&rv->refcnt);
	}
	return rv;
}

static inline void get_parallel(struct parallel_io *p)
{
	kref_get(&p->refcnt);
}

static void destroy_parallel(struct kref *kref)
{
	struct parallel_io *p = container_of(kref, struct parallel_io, refcnt);

	dprintk("%s enter\n", __func__);
	p->pnfs_callback(p->data);
	kfree(p);
}

static inline void put_parallel(struct parallel_io *p)
{
	kref_put(&p->refcnt, destroy_parallel);
}

static struct bio *
bl_submit_bio(int rw, struct bio *bio)
{
	if (bio) {
		get_parallel(bio->bi_private);
		dprintk("%s submitting %s bio %u@%llu\n", __func__,
			rw == READ ? "read" : "write", bio->bi_iter.bi_size,
			(unsigned long long)bio->bi_iter.bi_sector);
		submit_bio(rw, bio);
	}
	return NULL;
}

static struct bio *bl_alloc_init_bio(int npg, sector_t isect,
				     struct pnfs_block_extent *be,
				     void (*end_io)(struct bio *, int err),
				     struct parallel_io *par)
{
	struct bio *bio;

	npg = min(npg, BIO_MAX_PAGES);
	bio = bio_alloc(GFP_NOIO, npg);
	if (!bio && (current->flags & PF_MEMALLOC)) {
		while (!bio && (npg /= 2))
			bio = bio_alloc(GFP_NOIO, npg);
	}

	if (bio) {
		bio->bi_iter.bi_sector = isect - be->be_f_offset +
			be->be_v_offset;
		bio->bi_bdev = be->be_mdev;
		bio->bi_end_io = end_io;
		bio->bi_private = par;
	}
	return bio;
}

static struct bio *do_add_page_to_bio(struct bio *bio, int npg, int rw,
				      sector_t isect, struct page *page,
				      struct pnfs_block_extent *be,
				      void (*end_io)(struct bio *, int err),
				      struct parallel_io *par,
				      unsigned int offset, int len)
{
	isect = isect + (offset >> SECTOR_SHIFT);
	dprintk("%s: npg %d rw %d isect %llu offset %u len %d\n", __func__,
		npg, rw, (unsigned long long)isect, offset, len);
retry:
	if (!bio) {
		bio = bl_alloc_init_bio(npg, isect, be, end_io, par);
		if (!bio)
			return ERR_PTR(-ENOMEM);
	}
	if (bio_add_page(bio, page, len, offset) < len) {
		bio = bl_submit_bio(rw, bio);
		goto retry;
	}
	return bio;
}

static void bl_end_io_read(struct bio *bio, int err)
{
	struct parallel_io *par = bio->bi_private;

	if (err) {
		struct nfs_pgio_header *header = par->data;

		if (!header->pnfs_error)
			header->pnfs_error = -EIO;
		pnfs_set_lo_fail(header->lseg);
	}

	bio_put(bio);
	put_parallel(par);
}

static void bl_read_cleanup(struct work_struct *work)
{
	struct rpc_task *task;
	struct nfs_pgio_header *hdr;
	dprintk("%s enter\n", __func__);
	task = container_of(work, struct rpc_task, u.tk_work);
	hdr = container_of(task, struct nfs_pgio_header, task);
	pnfs_ld_read_done(hdr);
}

static void
bl_end_par_io_read(void *data)
{
	struct nfs_pgio_header *hdr = data;

	hdr->task.tk_status = hdr->pnfs_error;
	INIT_WORK(&hdr->task.u.tk_work, bl_read_cleanup);
	schedule_work(&hdr->task.u.tk_work);
}

static enum pnfs_try_status
bl_read_pagelist(struct nfs_pgio_header *header)
{
	struct pnfs_block_layout *bl = BLK_LSEG2EXT(header->lseg);
	struct bio *bio = NULL;
	struct pnfs_block_extent be;
	sector_t isect, extent_length = 0;
	struct parallel_io *par;
	loff_t f_offset = header->args.offset;
	size_t bytes_left = header->args.count;
	unsigned int pg_offset, pg_len;
	struct page **pages = header->args.pages;
	int pg_index = header->args.pgbase >> PAGE_CACHE_SHIFT;
	const bool is_dio = (header->dreq != NULL);
	struct blk_plug plug;
	int i;

	dprintk("%s enter nr_pages %u offset %lld count %u\n", __func__,
		header->page_array.npages, f_offset,
		(unsigned int)header->args.count);

	par = alloc_parallel(header);
	if (!par)
		return PNFS_NOT_ATTEMPTED;
	par->pnfs_callback = bl_end_par_io_read;

	blk_start_plug(&plug);

	isect = (sector_t) (f_offset >> SECTOR_SHIFT);
	/* Code assumes extents are page-aligned */
	for (i = pg_index; i < header->page_array.npages; i++) {
		if (extent_length <= 0) {
			/* We've used up the previous extent */
			bio = bl_submit_bio(READ, bio);

			/* Get the next one */
			if (!ext_tree_lookup(bl, isect, &be, false)) {
				header->pnfs_error = -EIO;
				goto out;
			}
			extent_length = be.be_length - (isect - be.be_f_offset);
		}

		pg_offset = f_offset & ~PAGE_CACHE_MASK;
		if (is_dio) {
			if (pg_offset + bytes_left > PAGE_CACHE_SIZE)
				pg_len = PAGE_CACHE_SIZE - pg_offset;
			else
				pg_len = bytes_left;

			f_offset += pg_len;
			bytes_left -= pg_len;
			isect += (pg_offset >> SECTOR_SHIFT);
			extent_length -= (pg_offset >> SECTOR_SHIFT);
		} else {
			BUG_ON(pg_offset != 0);
			pg_len = PAGE_CACHE_SIZE;
		}

		if (is_hole(&be)) {
			bio = bl_submit_bio(READ, bio);
			/* Fill hole w/ zeroes w/o accessing device */
			dprintk("%s Zeroing page for hole\n", __func__);
			zero_user_segment(pages[i], pg_offset, pg_len);
		} else {
			bio = do_add_page_to_bio(bio,
						 header->page_array.npages - i,
						 READ,
						 isect, pages[i], &be,
						 bl_end_io_read, par,
						 pg_offset, pg_len);
			if (IS_ERR(bio)) {
				header->pnfs_error = PTR_ERR(bio);
				bio = NULL;
				goto out;
			}
		}
		isect += (pg_len >> SECTOR_SHIFT);
		extent_length -= (pg_len >> SECTOR_SHIFT);
	}
	if ((isect << SECTOR_SHIFT) >= header->inode->i_size) {
		header->res.eof = 1;
		header->res.count = header->inode->i_size - header->args.offset;
	} else {
		header->res.count = (isect << SECTOR_SHIFT) - header->args.offset;
	}
out:
	bl_submit_bio(READ, bio);
	blk_finish_plug(&plug);
	put_parallel(par);
	return PNFS_ATTEMPTED;
}

static void bl_end_io_write(struct bio *bio, int err)
{
	struct parallel_io *par = bio->bi_private;
	const int uptodate = test_bit(BIO_UPTODATE, &bio->bi_flags);
	struct nfs_pgio_header *header = par->data;

	if (!uptodate) {
		if (!header->pnfs_error)
			header->pnfs_error = -EIO;
		pnfs_set_lo_fail(header->lseg);
	}
	bio_put(bio);
	put_parallel(par);
}

/* Function scheduled for call during bl_end_par_io_write,
 * it marks sectors as written and extends the commitlist.
 */
static void bl_write_cleanup(struct work_struct *work)
{
	struct rpc_task *task = container_of(work, struct rpc_task, u.tk_work);
	struct nfs_pgio_header *hdr =
			container_of(task, struct nfs_pgio_header, task);

	dprintk("%s enter\n", __func__);

	if (likely(!hdr->pnfs_error)) {
		struct pnfs_block_layout *bl = BLK_LSEG2EXT(hdr->lseg);
		u64 start = hdr->args.offset & (loff_t)PAGE_CACHE_MASK;
		u64 end = (hdr->args.offset + hdr->args.count +
			PAGE_CACHE_SIZE - 1) & (loff_t)PAGE_CACHE_MASK;

		ext_tree_mark_written(bl, start >> SECTOR_SHIFT,
					(end - start) >> SECTOR_SHIFT);
	}

	pnfs_ld_write_done(hdr);
}

/* Called when last of bios associated with a bl_write_pagelist call finishes */
static void bl_end_par_io_write(void *data)
{
	struct nfs_pgio_header *hdr = data;

	hdr->task.tk_status = hdr->pnfs_error;
	hdr->verf.committed = NFS_FILE_SYNC;
	INIT_WORK(&hdr->task.u.tk_work, bl_write_cleanup);
	schedule_work(&hdr->task.u.tk_work);
}

static enum pnfs_try_status
bl_write_pagelist(struct nfs_pgio_header *header, int sync)
{
	struct pnfs_block_layout *bl = BLK_LSEG2EXT(header->lseg);
	struct bio *bio = NULL;
	struct pnfs_block_extent be;
	sector_t isect, extent_length = 0;
	struct parallel_io *par = NULL;
	loff_t offset = header->args.offset;
	size_t count = header->args.count;
	struct page **pages = header->args.pages;
	int pg_index = pg_index = header->args.pgbase >> PAGE_CACHE_SHIFT;
	struct blk_plug plug;
	int i;

	dprintk("%s enter, %Zu@%lld\n", __func__, count, offset);

	/* At this point, header->page_aray is a (sequential) list of nfs_pages.
	 * We want to write each, and if there is an error set pnfs_error
	 * to have it redone using nfs.
	 */
	par = alloc_parallel(header);
	if (!par)
		return PNFS_NOT_ATTEMPTED;
	par->pnfs_callback = bl_end_par_io_write;

	blk_start_plug(&plug);

	/* we always write out the whole page */
	offset = offset & (loff_t)PAGE_CACHE_MASK;
	isect = offset >> SECTOR_SHIFT;

	for (i = pg_index; i < header->page_array.npages; i++) {
		if (extent_length <= 0) {
			/* We've used up the previous extent */
			bio = bl_submit_bio(WRITE, bio);
			/* Get the next one */
			if (!ext_tree_lookup(bl, isect, &be, true)) {
				header->pnfs_error = -EINVAL;
				goto out;
			}

			extent_length = be.be_length - (isect - be.be_f_offset);
		}

		bio = do_add_page_to_bio(bio, header->page_array.npages - i,
					 WRITE, isect, pages[i], &be,
					 bl_end_io_write, par,
					 0, PAGE_CACHE_SIZE);
		if (IS_ERR(bio)) {
			header->pnfs_error = PTR_ERR(bio);
			bio = NULL;
			goto out;
		}
		offset += PAGE_CACHE_SIZE;
		count -= PAGE_CACHE_SIZE;
		isect += PAGE_CACHE_SECTORS;
		extent_length -= PAGE_CACHE_SECTORS;
	}

	header->res.count = header->args.count;
out:
	bl_submit_bio(WRITE, bio);
	blk_finish_plug(&plug);
	put_parallel(par);
	return PNFS_ATTEMPTED;
}

static void bl_free_layout_hdr(struct pnfs_layout_hdr *lo)
{
	struct pnfs_block_layout *bl = BLK_LO2EXT(lo);
	int err;

	dprintk("%s enter\n", __func__);

	err = ext_tree_remove(bl, true, 0, LLONG_MAX);
	WARN_ON(err);

	kfree(bl);
}

static struct pnfs_layout_hdr *bl_alloc_layout_hdr(struct inode *inode,
						   gfp_t gfp_flags)
{
	struct pnfs_block_layout *bl;

	dprintk("%s enter\n", __func__);
	bl = kzalloc(sizeof(*bl), gfp_flags);
	if (!bl)
		return NULL;

	bl->bl_ext_rw = RB_ROOT;
	bl->bl_ext_ro = RB_ROOT;
	spin_lock_init(&bl->bl_ext_lock);

	return &bl->bl_layout;
}

static void bl_free_lseg(struct pnfs_layout_segment *lseg)
{
	dprintk("%s enter\n", __func__);
	kfree(lseg);
}

/* We pretty much ignore lseg, and store all data layout wide, so we
 * can correctly merge.
 */
static struct pnfs_layout_segment *bl_alloc_lseg(struct pnfs_layout_hdr *lo,
						 struct nfs4_layoutget_res *lgr,
						 gfp_t gfp_flags)
{
	struct pnfs_layout_segment *lseg;
	int status;

	dprintk("%s enter\n", __func__);
	lseg = kzalloc(sizeof(*lseg), gfp_flags);
	if (!lseg)
		return ERR_PTR(-ENOMEM);
	status = nfs4_blk_process_layoutget(lo, lgr, gfp_flags);
	if (status) {
		/* We don't want to call the full-blown bl_free_lseg,
		 * since on error extents were not touched.
		 */
		kfree(lseg);
		return ERR_PTR(status);
	}
	return lseg;
}

static void
bl_return_range(struct pnfs_layout_hdr *lo,
		struct pnfs_layout_range *range)
{
	struct pnfs_block_layout *bl = BLK_LO2EXT(lo);
	sector_t offset = range->offset >> SECTOR_SHIFT, end;
	int err;

	if (range->offset % 8) {
		dprintk("%s: offset %lld not block size aligned\n",
			__func__, range->offset);
		return;
	}

	if (range->length != NFS4_MAX_UINT64) {
		if (range->length % 8) {
			dprintk("%s: length %lld not block size aligned\n",
				__func__, range->length);
			return;
		}

		end = offset + (range->length >> SECTOR_SHIFT);
	} else {
		end = round_down(NFS4_MAX_UINT64, PAGE_SIZE);
	}

	err = ext_tree_remove(bl, range->iomode & IOMODE_RW, offset, end);
}

static void
bl_encode_layoutcommit(struct pnfs_layout_hdr *lo, struct xdr_stream *xdr,
		       const struct nfs4_layoutcommit_args *arg)
{
	dprintk("%s enter\n", __func__);
	ext_tree_encode_commit(BLK_LO2EXT(lo), xdr);
}

static void
bl_cleanup_layoutcommit(struct nfs4_layoutcommit_data *lcdata)
{
	struct pnfs_layout_hdr *lo = NFS_I(lcdata->args.inode)->layout;

	dprintk("%s enter\n", __func__);
	ext_tree_mark_committed(BLK_LO2EXT(lo), lcdata->res.status);
}

static void free_blk_mountid(struct block_mount_id *mid)
{
	if (mid) {
		struct pnfs_block_dev *dev, *tmp;

		/* No need to take bm_lock as we are last user freeing bm_devlist */
		list_for_each_entry_safe(dev, tmp, &mid->bm_devlist, bm_node) {
			list_del(&dev->bm_node);
			bl_free_block_dev(dev);
		}
		kfree(mid);
	}
}

/* This is mostly copied from the filelayout_get_device_info function.
 * It seems much of this should be at the generic pnfs level.
 */
static struct pnfs_block_dev *
nfs4_blk_get_deviceinfo(struct nfs_server *server, const struct nfs_fh *fh,
			struct nfs4_deviceid *d_id)
{
	struct pnfs_device *dev;
	struct pnfs_block_dev *rv;
	u32 max_resp_sz;
	int max_pages;
	struct page **pages = NULL;
	int i, rc;

	/*
	 * Use the session max response size as the basis for setting
	 * GETDEVICEINFO's maxcount
	 */
	max_resp_sz = server->nfs_client->cl_session->fc_attrs.max_resp_sz;
	max_pages = nfs_page_array_len(0, max_resp_sz);
	dprintk("%s max_resp_sz %u max_pages %d\n",
		__func__, max_resp_sz, max_pages);

	dev = kmalloc(sizeof(*dev), GFP_NOFS);
	if (!dev) {
		dprintk("%s kmalloc failed\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	pages = kcalloc(max_pages, sizeof(struct page *), GFP_NOFS);
	if (pages == NULL) {
		kfree(dev);
		return ERR_PTR(-ENOMEM);
	}
	for (i = 0; i < max_pages; i++) {
		pages[i] = alloc_page(GFP_NOFS);
		if (!pages[i]) {
			rv = ERR_PTR(-ENOMEM);
			goto out_free;
		}
	}

	memcpy(&dev->dev_id, d_id, sizeof(*d_id));
	dev->layout_type = LAYOUT_BLOCK_VOLUME;
	dev->pages = pages;
	dev->pgbase = 0;
	dev->pglen = PAGE_SIZE * max_pages;
	dev->mincount = 0;
	dev->maxcount = max_resp_sz - nfs41_maxgetdevinfo_overhead;

	dprintk("%s: dev_id: %s\n", __func__, dev->dev_id.data);
	rc = nfs4_proc_getdeviceinfo(server, dev, NULL);
	dprintk("%s getdevice info returns %d\n", __func__, rc);
	if (rc) {
		rv = ERR_PTR(rc);
		goto out_free;
	}

	rv = nfs4_blk_decode_device(server, dev);
 out_free:
	for (i = 0; i < max_pages; i++)
		__free_page(pages[i]);
	kfree(pages);
	kfree(dev);
	return rv;
}

static int
bl_set_layoutdriver(struct nfs_server *server, const struct nfs_fh *fh)
{
	struct block_mount_id *b_mt_id = NULL;
	struct pnfs_devicelist *dlist = NULL;
	struct pnfs_block_dev *bdev;
	LIST_HEAD(block_disklist);
	int status, i;

	dprintk("%s enter\n", __func__);

	if (server->pnfs_blksize == 0) {
		dprintk("%s Server did not return blksize\n", __func__);
		return -EINVAL;
	}
	if (server->pnfs_blksize > PAGE_SIZE) {
		printk(KERN_ERR "%s: pNFS blksize %d not supported.\n",
			__func__, server->pnfs_blksize);
		return -EINVAL;
	}

	b_mt_id = kzalloc(sizeof(struct block_mount_id), GFP_NOFS);
	if (!b_mt_id) {
		status = -ENOMEM;
		goto out_error;
	}
	/* Initialize nfs4 block layout mount id */
	spin_lock_init(&b_mt_id->bm_lock);
	INIT_LIST_HEAD(&b_mt_id->bm_devlist);

	dlist = kmalloc(sizeof(struct pnfs_devicelist), GFP_NOFS);
	if (!dlist) {
		status = -ENOMEM;
		goto out_error;
	}
	dlist->eof = 0;
	while (!dlist->eof) {
		status = nfs4_proc_getdevicelist(server, fh, dlist);
		if (status)
			goto out_error;
		dprintk("%s GETDEVICELIST numdevs=%i, eof=%i\n",
			__func__, dlist->num_devs, dlist->eof);
		for (i = 0; i < dlist->num_devs; i++) {
			bdev = nfs4_blk_get_deviceinfo(server, fh,
						       &dlist->dev_id[i]);
			if (IS_ERR(bdev)) {
				status = PTR_ERR(bdev);
				goto out_error;
			}
			spin_lock(&b_mt_id->bm_lock);
			list_add(&bdev->bm_node, &b_mt_id->bm_devlist);
			spin_unlock(&b_mt_id->bm_lock);
		}
	}
	dprintk("%s SUCCESS\n", __func__);
	server->pnfs_ld_data = b_mt_id;

 out_return:
	kfree(dlist);
	return status;

 out_error:
	free_blk_mountid(b_mt_id);
	goto out_return;
}

static int
bl_clear_layoutdriver(struct nfs_server *server)
{
	struct block_mount_id *b_mt_id = server->pnfs_ld_data;

	dprintk("%s enter\n", __func__);
	free_blk_mountid(b_mt_id);
	dprintk("%s RETURNS\n", __func__);
	return 0;
}

static bool
is_aligned_req(struct nfs_pageio_descriptor *pgio,
		struct nfs_page *req, unsigned int alignment)
{
	/*
	 * Always accept buffered writes, higher layers take care of the
	 * right alignment.
	 */
	if (pgio->pg_dreq == NULL)
		return true;

	if (!IS_ALIGNED(req->wb_offset, alignment))
		return false;

	if (IS_ALIGNED(req->wb_bytes, alignment))
		return true;

	if (req_offset(req) + req->wb_bytes == i_size_read(pgio->pg_inode)) {
		/*
		 * If the write goes up to the inode size, just write
		 * the full page.  Data past the inode size is
		 * guaranteed to be zeroed by the higher level client
		 * code, and this behaviour is mandated by RFC 5663
		 * section 2.3.2.
		 */
		return true;
	}

	return false;
}

static void
bl_pg_init_read(struct nfs_pageio_descriptor *pgio, struct nfs_page *req)
{
	if (!is_aligned_req(pgio, req, SECTOR_SIZE)) {
		nfs_pageio_reset_read_mds(pgio);
		return;
	}

	pnfs_generic_pg_init_read(pgio, req);
}

/*
 * Return 0 if @req cannot be coalesced into @pgio, otherwise return the number
 * of bytes (maximum @req->wb_bytes) that can be coalesced.
 */
static size_t
bl_pg_test_read(struct nfs_pageio_descriptor *pgio, struct nfs_page *prev,
		struct nfs_page *req)
{
	if (!is_aligned_req(pgio, req, SECTOR_SIZE))
		return 0;
	return pnfs_generic_pg_test(pgio, prev, req);
}

/*
 * Return the number of contiguous bytes for a given inode
 * starting at page frame idx.
 */
static u64 pnfs_num_cont_bytes(struct inode *inode, pgoff_t idx)
{
	struct address_space *mapping = inode->i_mapping;
	pgoff_t end;

	/* Optimize common case that writes from 0 to end of file */
	end = DIV_ROUND_UP(i_size_read(inode), PAGE_CACHE_SIZE);
	if (end != NFS_I(inode)->npages) {
		rcu_read_lock();
		end = page_cache_next_hole(mapping, idx + 1, ULONG_MAX);
		rcu_read_unlock();
	}

	if (!end)
		return i_size_read(inode) - (idx << PAGE_CACHE_SHIFT);
	else
		return (end - idx) << PAGE_CACHE_SHIFT;
}

static void
bl_pg_init_write(struct nfs_pageio_descriptor *pgio, struct nfs_page *req)
{
	u64 wb_size;

	if (!is_aligned_req(pgio, req, PAGE_SIZE)) {
		nfs_pageio_reset_write_mds(pgio);
		return;
	}

	if (pgio->pg_dreq == NULL)
		wb_size = pnfs_num_cont_bytes(pgio->pg_inode,
					      req->wb_index);
	else
		wb_size = nfs_dreq_bytes_left(pgio->pg_dreq);

	pnfs_generic_pg_init_write(pgio, req, wb_size);
}

/*
 * Return 0 if @req cannot be coalesced into @pgio, otherwise return the number
 * of bytes (maximum @req->wb_bytes) that can be coalesced.
 */
static size_t
bl_pg_test_write(struct nfs_pageio_descriptor *pgio, struct nfs_page *prev,
		 struct nfs_page *req)
{
	if (!is_aligned_req(pgio, req, PAGE_SIZE))
		return 0;
	return pnfs_generic_pg_test(pgio, prev, req);
}

static const struct nfs_pageio_ops bl_pg_read_ops = {
	.pg_init = bl_pg_init_read,
	.pg_test = bl_pg_test_read,
	.pg_doio = pnfs_generic_pg_readpages,
};

static const struct nfs_pageio_ops bl_pg_write_ops = {
	.pg_init = bl_pg_init_write,
	.pg_test = bl_pg_test_write,
	.pg_doio = pnfs_generic_pg_writepages,
};

static struct pnfs_layoutdriver_type blocklayout_type = {
	.id				= LAYOUT_BLOCK_VOLUME,
	.name				= "LAYOUT_BLOCK_VOLUME",
	.owner				= THIS_MODULE,
	.flags				= PNFS_READ_WHOLE_PAGE,
	.read_pagelist			= bl_read_pagelist,
	.write_pagelist			= bl_write_pagelist,
	.alloc_layout_hdr		= bl_alloc_layout_hdr,
	.free_layout_hdr		= bl_free_layout_hdr,
	.alloc_lseg			= bl_alloc_lseg,
	.free_lseg			= bl_free_lseg,
	.return_range			= bl_return_range,
	.encode_layoutcommit		= bl_encode_layoutcommit,
	.cleanup_layoutcommit		= bl_cleanup_layoutcommit,
	.set_layoutdriver		= bl_set_layoutdriver,
	.clear_layoutdriver		= bl_clear_layoutdriver,
	.pg_read_ops			= &bl_pg_read_ops,
	.pg_write_ops			= &bl_pg_write_ops,
};

static const struct rpc_pipe_ops bl_upcall_ops = {
	.upcall		= rpc_pipe_generic_upcall,
	.downcall	= bl_pipe_downcall,
	.destroy_msg	= bl_pipe_destroy_msg,
};

static struct dentry *nfs4blocklayout_register_sb(struct super_block *sb,
					    struct rpc_pipe *pipe)
{
	struct dentry *dir, *dentry;

	dir = rpc_d_lookup_sb(sb, NFS_PIPE_DIRNAME);
	if (dir == NULL)
		return ERR_PTR(-ENOENT);
	dentry = rpc_mkpipe_dentry(dir, "blocklayout", NULL, pipe);
	dput(dir);
	return dentry;
}

static void nfs4blocklayout_unregister_sb(struct super_block *sb,
					  struct rpc_pipe *pipe)
{
	if (pipe->dentry)
		rpc_unlink(pipe->dentry);
}

static int rpc_pipefs_event(struct notifier_block *nb, unsigned long event,
			   void *ptr)
{
	struct super_block *sb = ptr;
	struct net *net = sb->s_fs_info;
	struct nfs_net *nn = net_generic(net, nfs_net_id);
	struct dentry *dentry;
	int ret = 0;

	if (!try_module_get(THIS_MODULE))
		return 0;

	if (nn->bl_device_pipe == NULL) {
		module_put(THIS_MODULE);
		return 0;
	}

	switch (event) {
	case RPC_PIPEFS_MOUNT:
		dentry = nfs4blocklayout_register_sb(sb, nn->bl_device_pipe);
		if (IS_ERR(dentry)) {
			ret = PTR_ERR(dentry);
			break;
		}
		nn->bl_device_pipe->dentry = dentry;
		break;
	case RPC_PIPEFS_UMOUNT:
		if (nn->bl_device_pipe->dentry)
			nfs4blocklayout_unregister_sb(sb, nn->bl_device_pipe);
		break;
	default:
		ret = -ENOTSUPP;
		break;
	}
	module_put(THIS_MODULE);
	return ret;
}

static struct notifier_block nfs4blocklayout_block = {
	.notifier_call = rpc_pipefs_event,
};

static struct dentry *nfs4blocklayout_register_net(struct net *net,
						   struct rpc_pipe *pipe)
{
	struct super_block *pipefs_sb;
	struct dentry *dentry;

	pipefs_sb = rpc_get_sb_net(net);
	if (!pipefs_sb)
		return NULL;
	dentry = nfs4blocklayout_register_sb(pipefs_sb, pipe);
	rpc_put_sb_net(net);
	return dentry;
}

static void nfs4blocklayout_unregister_net(struct net *net,
					   struct rpc_pipe *pipe)
{
	struct super_block *pipefs_sb;

	pipefs_sb = rpc_get_sb_net(net);
	if (pipefs_sb) {
		nfs4blocklayout_unregister_sb(pipefs_sb, pipe);
		rpc_put_sb_net(net);
	}
}

static int nfs4blocklayout_net_init(struct net *net)
{
	struct nfs_net *nn = net_generic(net, nfs_net_id);
	struct dentry *dentry;

	init_waitqueue_head(&nn->bl_wq);
	nn->bl_device_pipe = rpc_mkpipe_data(&bl_upcall_ops, 0);
	if (IS_ERR(nn->bl_device_pipe))
		return PTR_ERR(nn->bl_device_pipe);
	dentry = nfs4blocklayout_register_net(net, nn->bl_device_pipe);
	if (IS_ERR(dentry)) {
		rpc_destroy_pipe_data(nn->bl_device_pipe);
		return PTR_ERR(dentry);
	}
	nn->bl_device_pipe->dentry = dentry;
	return 0;
}

static void nfs4blocklayout_net_exit(struct net *net)
{
	struct nfs_net *nn = net_generic(net, nfs_net_id);

	nfs4blocklayout_unregister_net(net, nn->bl_device_pipe);
	rpc_destroy_pipe_data(nn->bl_device_pipe);
	nn->bl_device_pipe = NULL;
}

static struct pernet_operations nfs4blocklayout_net_ops = {
	.init = nfs4blocklayout_net_init,
	.exit = nfs4blocklayout_net_exit,
};

static int __init nfs4blocklayout_init(void)
{
	int ret;

	dprintk("%s: NFSv4 Block Layout Driver Registering...\n", __func__);

	ret = pnfs_register_layoutdriver(&blocklayout_type);
	if (ret)
		goto out;

	ret = rpc_pipefs_notifier_register(&nfs4blocklayout_block);
	if (ret)
		goto out_remove;
	ret = register_pernet_subsys(&nfs4blocklayout_net_ops);
	if (ret)
		goto out_notifier;
out:
	return ret;

out_notifier:
	rpc_pipefs_notifier_unregister(&nfs4blocklayout_block);
out_remove:
	pnfs_unregister_layoutdriver(&blocklayout_type);
	return ret;
}

static void __exit nfs4blocklayout_exit(void)
{
	dprintk("%s: NFSv4 Block Layout Driver Unregistering...\n",
	       __func__);

	rpc_pipefs_notifier_unregister(&nfs4blocklayout_block);
	unregister_pernet_subsys(&nfs4blocklayout_net_ops);
	pnfs_unregister_layoutdriver(&blocklayout_type);
}

MODULE_ALIAS("nfs-layouttype4-3");

module_init(nfs4blocklayout_init);
module_exit(nfs4blocklayout_exit);
