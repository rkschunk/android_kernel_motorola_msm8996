/*
 *	watchdog_dev.c
 *
 *	(c) Copyright 2008-2011 Alan Cox <alan@lxorguk.ukuu.org.uk>,
 *						All Rights Reserved.
 *
 *	(c) Copyright 2008-2011 Wim Van Sebroeck <wim@iguana.be>.
 *
 *
 *	This source code is part of the generic code that can be used
 *	by all the watchdog timer drivers.
 *
 *	This part of the generic code takes care of the following
 *	misc device: /dev/watchdog.
 *
 *	Based on source code of the following authors:
 *	  Matt Domsch <Matt_Domsch@dell.com>,
 *	  Rob Radez <rob@osinvestor.com>,
 *	  Rusty Lynch <rusty@linux.co.intel.com>
 *	  Satyam Sharma <satyam@infradead.org>
 *	  Randy Dunlap <randy.dunlap@oracle.com>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *	Neither Alan Cox, CymruNet Ltd., Wim Van Sebroeck nor Iguana vzw.
 *	admit liability nor provide warranty for any of this software.
 *	This material is provided "AS-IS" and at no charge.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>	/* For module stuff/... */
#include <linux/types.h>	/* For standard types (like size_t) */
#include <linux/errno.h>	/* For the -ENODEV/... values */
#include <linux/kernel.h>	/* For printk/panic/... */
#include <linux/fs.h>		/* For file operations */
#include <linux/watchdog.h>	/* For watchdog specific items */
#include <linux/miscdevice.h>	/* For handling misc devices */
#include <linux/init.h>		/* For __init/__exit/... */
#include <linux/uaccess.h>	/* For copy_to_user/put_user/... */

/* make sure we only register one /dev/watchdog device */
static unsigned long watchdog_dev_busy;
/* the watchdog device behind /dev/watchdog */
static struct watchdog_device *wdd;

/*
 *	watchdog_ping: ping the watchdog.
 *	@wddev: the watchdog device to ping
 *
 *	If the watchdog has no own ping operation then it needs to be
 *	restarted via the start operation. This wrapper function does
 *	exactly that.
 */

static int watchdog_ping(struct watchdog_device *wddev)
{
	if (wddev->ops->ping)
		return wddev->ops->ping(wddev);  /* ping the watchdog */
	else
		return wddev->ops->start(wddev); /* restart the watchdog */
}

/*
 *	watchdog_write: writes to the watchdog.
 *	@file: file from VFS
 *	@data: user address of data
 *	@len: length of data
 *	@ppos: pointer to the file offset
 *
 *	A write to a watchdog device is defined as a keepalive ping.
 */

static ssize_t watchdog_write(struct file *file, const char __user *data,
						size_t len, loff_t *ppos)
{
	size_t i;
	char c;

	if (len == 0)
		return 0;

	for (i = 0; i != len; i++) {
		if (get_user(c, data + i))
			return -EFAULT;
	}

	/* someone wrote to us, so we send the watchdog a keepalive ping */
	watchdog_ping(wdd);

	return len;
}

/*
 *	watchdog_open: open the /dev/watchdog device.
 *	@inode: inode of device
 *	@file: file handle to device
 *
 *	When the /dev/watchdog device gets opened, we start the watchdog.
 *	Watch out: the /dev/watchdog device is single open, so we make sure
 *	it can only be opened once.
 */

static int watchdog_open(struct inode *inode, struct file *file)
{
	int err = -EBUSY;

	/* the watchdog is single open! */
	if (test_and_set_bit(WDOG_DEV_OPEN, &wdd->status))
		return -EBUSY;

	/*
	 * If the /dev/watchdog device is open, we don't want the module
	 * to be unloaded.
	 */
	if (!try_module_get(wdd->ops->owner))
		goto out;

	err = wdd->ops->start(wdd);
	if (err < 0)
		goto out_mod;

	/* dev/watchdog is a virtual (and thus non-seekable) filesystem */
	return nonseekable_open(inode, file);

out_mod:
	module_put(wdd->ops->owner);
out:
	clear_bit(WDOG_DEV_OPEN, &wdd->status);
	return err;
}

/*
 *      watchdog_release: release the /dev/watchdog device.
 *      @inode: inode of device
 *      @file: file handle to device
 *
 *	This is the code for when /dev/watchdog gets closed.
 */

static int watchdog_release(struct inode *inode, struct file *file)
{
	int err;

	err = wdd->ops->stop(wdd);
	if (err != 0) {
		pr_crit("%s: watchdog did not stop!\n", wdd->info->identity);
		watchdog_ping(wdd);
	}

	/* Allow the owner module to be unloaded again */
	module_put(wdd->ops->owner);

	/* make sure that /dev/watchdog can be re-opened */
	clear_bit(WDOG_DEV_OPEN, &wdd->status);

	return 0;
}

static const struct file_operations watchdog_fops = {
	.owner		= THIS_MODULE,
	.write		= watchdog_write,
	.open		= watchdog_open,
	.release	= watchdog_release,
};

static struct miscdevice watchdog_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &watchdog_fops,
};

/*
 *	watchdog_dev_register:
 *	@watchdog: watchdog device
 *
 *	Register a watchdog device as /dev/watchdog. /dev/watchdog
 *	is actually a miscdevice and thus we set it up like that.
 */

int watchdog_dev_register(struct watchdog_device *watchdog)
{
	int err;

	/* Only one device can register for /dev/watchdog */
	if (test_and_set_bit(0, &watchdog_dev_busy)) {
		pr_err("only one watchdog can use /dev/watchdog.\n");
		return -EBUSY;
	}

	wdd = watchdog;

	err = misc_register(&watchdog_miscdev);
	if (err != 0) {
		pr_err("%s: cannot register miscdev on minor=%d (err=%d).\n",
			watchdog->info->identity, WATCHDOG_MINOR, err);
		goto out;
	}

	return 0;

out:
	wdd = NULL;
	clear_bit(0, &watchdog_dev_busy);
	return err;
}

/*
 *	watchdog_dev_unregister:
 *	@watchdog: watchdog device
 *
 *	Deregister the /dev/watchdog device.
 */

int watchdog_dev_unregister(struct watchdog_device *watchdog)
{
	/* Check that a watchdog device was registered in the past */
	if (!test_bit(0, &watchdog_dev_busy) || !wdd)
		return -ENODEV;

	/* We can only unregister the watchdog device that was registered */
	if (watchdog != wdd) {
		pr_err("%s: watchdog was not registered as /dev/watchdog.\n",
			watchdog->info->identity);
		return -ENODEV;
	}

	misc_deregister(&watchdog_miscdev);
	wdd = NULL;
	clear_bit(0, &watchdog_dev_busy);
	return 0;
}
