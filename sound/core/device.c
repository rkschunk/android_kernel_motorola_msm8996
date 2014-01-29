/*
 *  Device management routines
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/slab.h>
#include <linux/time.h>
#include <linux/export.h>
#include <linux/errno.h>
#include <sound/core.h>

/**
 * snd_device_new - create an ALSA device component
 * @card: the card instance
 * @type: the device type, SNDRV_DEV_XXX
 * @device_data: the data pointer of this device
 * @ops: the operator table
 *
 * Creates a new device component for the given data pointer.
 * The device will be assigned to the card and managed together
 * by the card.
 *
 * The data pointer plays a role as the identifier, too, so the
 * pointer address must be unique and unchanged.
 *
 * Return: Zero if successful, or a negative error code on failure.
 */
int snd_device_new(struct snd_card *card, enum snd_device_type type,
		   void *device_data, struct snd_device_ops *ops)
{
	struct snd_device *dev;
	struct list_head *p;

	if (snd_BUG_ON(!card || !device_data || !ops))
		return -ENXIO;
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (dev == NULL) {
		dev_err(card->dev, "Cannot allocate device, type=%d\n", type);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&dev->list);
	dev->card = card;
	dev->type = type;
	dev->state = SNDRV_DEV_BUILD;
	dev->device_data = device_data;
	dev->ops = ops;

	/* insert the entry in an incrementally sorted list */
	list_for_each_prev(p, &card->devices) {
		struct snd_device *pdev = list_entry(p, struct snd_device, list);
		if ((unsigned int)pdev->type <= (unsigned int)type)
			break;
	}

	list_add(&dev->list, p);
	return 0;
}
EXPORT_SYMBOL(snd_device_new);

static struct snd_device *look_for_dev(struct snd_card *card, void *device_data)
{
	struct snd_device *dev;

	list_for_each_entry(dev, &card->devices, list)
		if (dev->device_data == device_data)
			return dev;

	return NULL;
}

/**
 * snd_device_free - release the device from the card
 * @card: the card instance
 * @device_data: the data pointer to release
 *
 * Removes the device from the list on the card and invokes the
 * callbacks, dev_disconnect and dev_free, corresponding to the state.
 * Then release the device.
 *
 * Return: Zero if successful, or a negative error code on failure or if the
 * device not found.
 */
int snd_device_free(struct snd_card *card, void *device_data)
{
	struct snd_device *dev;
	
	if (snd_BUG_ON(!card || !device_data))
		return -ENXIO;
	dev = look_for_dev(card, device_data);
	if (dev) {
		/* unlink */
		list_del(&dev->list);
		if (dev->state == SNDRV_DEV_REGISTERED &&
		    dev->ops->dev_disconnect)
			if (dev->ops->dev_disconnect(dev))
				dev_err(card->dev,
					"device disconnect failure\n");
		if (dev->ops->dev_free) {
			if (dev->ops->dev_free(dev))
				dev_err(card->dev, "device free failure\n");
		}
		kfree(dev);
		return 0;
	}
	dev_dbg(card->dev, "device free %p (from %pF), not found\n",
		device_data, __builtin_return_address(0));
	return -ENXIO;
}
EXPORT_SYMBOL(snd_device_free);

/**
 * snd_device_disconnect - disconnect the device
 * @card: the card instance
 * @device_data: the data pointer to disconnect
 *
 * Turns the device into the disconnection state, invoking
 * dev_disconnect callback, if the device was already registered.
 *
 * Usually called from snd_card_disconnect().
 *
 * Return: Zero if successful, or a negative error code on failure or if the
 * device not found.
 */
int snd_device_disconnect(struct snd_card *card, void *device_data)
{
	struct snd_device *dev;

	if (snd_BUG_ON(!card || !device_data))
		return -ENXIO;
	dev = look_for_dev(card, device_data);
	if (dev) {
		if (dev->state == SNDRV_DEV_REGISTERED &&
		    dev->ops->dev_disconnect) {
			if (dev->ops->dev_disconnect(dev))
				dev_err(card->dev,
					"device disconnect failure\n");
			dev->state = SNDRV_DEV_DISCONNECTED;
		}
		return 0;
	}
	dev_dbg(card->dev, "device disconnect %p (from %pF), not found\n",
		device_data, __builtin_return_address(0));
	return -ENXIO;
}

/**
 * snd_device_register - register the device
 * @card: the card instance
 * @device_data: the data pointer to register
 *
 * Registers the device which was already created via
 * snd_device_new().  Usually this is called from snd_card_register(),
 * but it can be called later if any new devices are created after
 * invocation of snd_card_register().
 *
 * Return: Zero if successful, or a negative error code on failure or if the
 * device not found.
 */
int snd_device_register(struct snd_card *card, void *device_data)
{
	struct snd_device *dev;
	int err;

	if (snd_BUG_ON(!card || !device_data))
		return -ENXIO;
	dev = look_for_dev(card, device_data);
	if (dev) {
		if (dev->state == SNDRV_DEV_BUILD && dev->ops->dev_register) {
			if ((err = dev->ops->dev_register(dev)) < 0)
				return err;
			dev->state = SNDRV_DEV_REGISTERED;
			return 0;
		}
		dev_dbg(card->dev, "snd_device_register busy\n");
		return -EBUSY;
	}
	snd_BUG();
	return -ENXIO;
}
EXPORT_SYMBOL(snd_device_register);

/*
 * register all the devices on the card.
 * called from init.c
 */
int snd_device_register_all(struct snd_card *card)
{
	struct snd_device *dev;
	int err;
	
	if (snd_BUG_ON(!card))
		return -ENXIO;
	list_for_each_entry(dev, &card->devices, list) {
		if (dev->state == SNDRV_DEV_BUILD && dev->ops->dev_register) {
			if ((err = dev->ops->dev_register(dev)) < 0)
				return err;
			dev->state = SNDRV_DEV_REGISTERED;
		}
	}
	return 0;
}

/*
 * disconnect all the devices on the card.
 * called from init.c
 */
int snd_device_disconnect_all(struct snd_card *card)
{
	struct snd_device *dev;
	int err = 0;

	if (snd_BUG_ON(!card))
		return -ENXIO;
	list_for_each_entry_reverse(dev, &card->devices, list) {
		if (snd_device_disconnect(card, dev->device_data) < 0)
			err = -ENXIO;
	}
	return err;
}

/*
 * release all the devices on the card.
 * called from init.c
 */
int snd_device_free_all(struct snd_card *card)
{
	struct snd_device *dev, *next;
	int ret = 0;

	if (snd_BUG_ON(!card))
		return -ENXIO;
	list_for_each_entry_safe_reverse(dev, next, &card->devices, list) {
		int err = snd_device_free(card, dev->device_data);
		if (err < 0)
			ret = err;
	}
	return ret;
}
