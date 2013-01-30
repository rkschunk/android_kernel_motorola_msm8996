#ifndef _COMEDI_INTERNAL_H
#define _COMEDI_INTERNAL_H

#include <linux/types.h>

/*
 * various internal comedi stuff
 */
int do_rangeinfo_ioctl(struct comedi_device *dev,
		       struct comedi_rangeinfo __user *arg);
int comedi_alloc_board_minor(struct device *hardware_device);
void comedi_free_board_minor(unsigned minor);
int comedi_find_board_minor(struct device *hardware_device);

int comedi_buf_alloc(struct comedi_device *dev, struct comedi_subdevice *s,
		     unsigned long new_size);
void comedi_buf_reset(struct comedi_async *async);
unsigned int comedi_buf_write_n_allocated(struct comedi_async *async);

extern unsigned int comedi_default_buf_size_kb;
extern unsigned int comedi_default_buf_maxsize_kb;
extern bool comedi_autoconfig;

/* drivers.c */

extern struct comedi_driver *comedi_drivers;

int insn_inval(struct comedi_device *, struct comedi_subdevice *,
	       struct comedi_insn *, unsigned int *);

void comedi_device_detach(struct comedi_device *);
int comedi_device_attach(struct comedi_device *, struct comedi_devconfig *);

#endif /* _COMEDI_INTERNAL_H */
