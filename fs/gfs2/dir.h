/*
 * Copyright (C) Sistina Software, Inc.  1997-2003 All rights reserved.
 * Copyright (C) 2004-2005 Red Hat, Inc.  All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 */

#ifndef __DIR_DOT_H__
#define __DIR_DOT_H__

/**
 * gfs2_filldir_t - Report a directory entry to the caller of gfs2_dir_read()
 * @opaque: opaque data used by the function
 * @name: the name of the directory entry
 * @length: the length of the name
 * @offset: the entry's offset in the directory
 * @inum: the inode number the entry points to
 * @type: the type of inode the entry points to
 *
 * Returns: 0 on success, 1 if buffer full
 */

typedef int (*gfs2_filldir_t) (void *opaque,
			      const char *name, unsigned int length,
			      uint64_t offset,
			      struct gfs2_inum *inum, unsigned int type);

int gfs2_dir_search(struct inode *dir, const struct qstr *filename,
		    struct gfs2_inum *inum, unsigned int *type);
int gfs2_dir_add(struct inode *inode, const struct qstr *filename,
		 const struct gfs2_inum *inum, unsigned int type);
int gfs2_dir_del(struct gfs2_inode *dip, const struct qstr *filename);
int gfs2_dir_read(struct gfs2_inode *dip, uint64_t * offset, void *opaque,
		  gfs2_filldir_t filldir);
int gfs2_dir_mvino(struct gfs2_inode *dip, const struct qstr *filename,
		   struct gfs2_inum *new_inum, unsigned int new_type);

int gfs2_dir_exhash_dealloc(struct gfs2_inode *dip);

int gfs2_diradd_alloc_required(struct inode *dir,
			       const struct qstr *filename);
int gfs2_dir_get_buffer(struct gfs2_inode *ip, uint64_t block, int new,
                        struct buffer_head **bhp);

/* N.B. This probably ought to take inum & type as args as well */
static inline void gfs2_qstr2dirent(const struct qstr *name, u16 reclen, struct gfs2_dirent *dent)
{
	dent->de_inum.no_addr = cpu_to_be64(0);
	dent->de_inum.no_formal_ino = cpu_to_be64(0);
	dent->de_hash = cpu_to_be32(name->hash);
	dent->de_rec_len = cpu_to_be16(reclen);
	dent->de_name_len = cpu_to_be16(name->len);
	dent->de_type = cpu_to_be16(0);
	memset(dent->__pad, 0, sizeof(dent->__pad));
	memcpy((char*)(dent+1), name->name, name->len);
}

#endif /* __DIR_DOT_H__ */
