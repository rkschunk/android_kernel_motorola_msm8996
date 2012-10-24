/*
 * comedi/drivers/cb_pcidda.c
 * Driver for the ComputerBoards / MeasurementComputing PCI-DDA series.
 *
 * Copyright (C) 2001 Ivan Martinez <ivanmr@altavista.com>
 * Copyright (C) 2001 Frank Mori Hess <fmhess@users.sourceforge.net>
 *
 * COMEDI - Linux Control and Measurement Device Interface
 * Copyright (C) 1997-8 David A. Schleef <ds@schleef.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Driver: cb_pcidda
 * Description: MeasurementComputing PCI-DDA series
 * Devices: (Measurement Computing) PCI-DDA08/12 [pci-dda08/12]
 *	    (Measurement Computing) PCI-DDA04/12 [pci-dda04/12]
 *	    (Measurement Computing) PCI-DDA02/12 [pci-dda02/12]
 *	    (Measurement Computing) PCI-DDA08/16 [pci-dda08/16]
 *	    (Measurement Computing) PCI-DDA04/16 [pci-dda04/16]
 *	    (Measurement Computing) PCI-DDA02/16 [pci-dda02/16]
 * Author: Ivan Martinez <ivanmr@altavista.com>
 *	   Frank Mori Hess <fmhess@users.sourceforge.net>
 * Status: works
 *
 * Configuration options: not applicable, uses PCI auto config
 *
 * Only simple analog output writing is supported.
 */

#include "../comedidev.h"

#include "comedi_fc.h"
#include "8255.h"

/*
 * ComputerBoards PCI Device ID's supported by this driver
 */
#define PCI_DEVICE_ID_DDA02_12		0x0020
#define PCI_DEVICE_ID_DDA04_12		0x0021
#define PCI_DEVICE_ID_DDA08_12		0x0022
#define PCI_DEVICE_ID_DDA02_16		0x0023
#define PCI_DEVICE_ID_DDA04_16		0x0024
#define PCI_DEVICE_ID_DDA08_16		0x0025

#define EEPROM_SIZE	128	/*  number of entries in eeprom */
/* maximum number of ao channels for supported boards */
#define MAX_AO_CHANNELS 8

/* Digital I/O registers */
#define PORT1A 0		/*  PORT 1A DATA */

#define PORT1B 1		/*  PORT 1B DATA */

#define PORT1C 2		/*  PORT 1C DATA */

#define CONTROL1 3		/*  CONTROL REGISTER 1 */

#define PORT2A 4		/*  PORT 2A DATA */

#define PORT2B 5		/*  PORT 2B DATA */

#define PORT2C 6		/*  PORT 2C DATA */

#define CONTROL2 7		/*  CONTROL REGISTER 2 */

/* DAC registers */
#define DACONTROL	0	/*  D/A CONTROL REGISTER */
#define	SU	0000001		/*  Simultaneous update enabled */
#define NOSU	0000000		/*  Simultaneous update disabled */
#define	ENABLEDAC	0000002	/*  Enable specified DAC */
#define	DISABLEDAC	0000000	/*  Disable specified DAC */
#define RANGE2V5	0000000	/*  2.5V */
#define RANGE5V	0000200		/*  5V */
#define RANGE10V	0000300	/*  10V */
#define UNIP	0000400		/*  Unipolar outputs */
#define BIP	0000000		/*  Bipolar outputs */

#define DACALIBRATION1	4	/*  D/A CALIBRATION REGISTER 1 */
/* write bits */
/* serial data input for eeprom, caldacs, reference dac */
#define SERIAL_IN_BIT   0x1
#define	CAL_CHANNEL_MASK	(0x7 << 1)
#define	CAL_CHANNEL_BITS(channel)	(((channel) << 1) & CAL_CHANNEL_MASK)
/* read bits */
#define	CAL_COUNTER_MASK	0x1f
/* calibration counter overflow status bit */
#define CAL_COUNTER_OVERFLOW_BIT        0x20
/* analog output is less than reference dac voltage */
#define AO_BELOW_REF_BIT        0x40
#define	SERIAL_OUT_BIT	0x80	/*  serial data out, for reading from eeprom */

#define DACALIBRATION2	6	/*  D/A CALIBRATION REGISTER 2 */
#define	SELECT_EEPROM_BIT	0x1	/*  send serial data in to eeprom */
/* don't send serial data to MAX542 reference dac */
#define DESELECT_REF_DAC_BIT    0x2
/* don't send serial data to caldac n */
#define DESELECT_CALDAC_BIT(n)  (0x4 << (n))
/* manual says to set this bit with no explanation */
#define DUMMY_BIT       0x40

#define DADATA	8		/*  FIRST D/A DATA REGISTER (0) */

static const struct comedi_lrange cb_pcidda_ranges = {
	6, {
		BIP_RANGE(10),
		BIP_RANGE(5),
		BIP_RANGE(2.5),
		UNI_RANGE(10),
		UNI_RANGE(5),
		UNI_RANGE(2.5)
	}
};

/*
 * Board descriptions for two imaginary boards.  Describing the
 * boards in this way is optional, and completely driver-dependent.
 * Some drivers use arrays such as this, other do not.
 */
struct cb_pcidda_board {
	const char *name;
	char status;		/*  Driver status: */

	/*
	 * 0 - tested
	 * 1 - manual read, not tested
	 * 2 - manual not read
	 */

	unsigned short device_id;
	int ao_chans;
	int ao_bits;
};

static const struct cb_pcidda_board cb_pcidda_boards[] = {
	{
	 .name = "pci-dda02/12",
	 .status = 1,
	 .device_id = PCI_DEVICE_ID_DDA02_12,
	 .ao_chans = 2,
	 .ao_bits = 12,
	 },
	{
	 .name = "pci-dda04/12",
	 .status = 1,
	 .device_id = PCI_DEVICE_ID_DDA04_12,
	 .ao_chans = 4,
	 .ao_bits = 12,
	 },
	{
	 .name = "pci-dda08/12",
	 .status = 0,
	 .device_id = PCI_DEVICE_ID_DDA08_12,
	 .ao_chans = 8,
	 .ao_bits = 12,
	 },
	{
	 .name = "pci-dda02/16",
	 .status = 2,
	 .device_id = PCI_DEVICE_ID_DDA02_16,
	 .ao_chans = 2,
	 .ao_bits = 16,
	 },
	{
	 .name = "pci-dda04/16",
	 .status = 2,
	 .device_id = PCI_DEVICE_ID_DDA04_16,
	 .ao_chans = 4,
	 .ao_bits = 16,
	 },
	{
	 .name = "pci-dda08/16",
	 .status = 0,
	 .device_id = PCI_DEVICE_ID_DDA08_16,
	 .ao_chans = 8,
	 .ao_bits = 16,
	 },
};

struct cb_pcidda_private {
	/* bits last written to da calibration register 1 */
	unsigned int dac_cal1_bits;
	/* current range settings for output channels */
	unsigned int ao_range[MAX_AO_CHANNELS];
	u16 eeprom_data[EEPROM_SIZE];	/*  software copy of board's eeprom */
};

/* lowlevel read from eeprom */
static unsigned int cb_pcidda_serial_in(struct comedi_device *dev)
{
	unsigned int value = 0;
	int i;
	const int value_width = 16;	/*  number of bits wide values are */

	for (i = 1; i <= value_width; i++) {
		/*  read bits most significant bit first */
		if (inw_p(dev->iobase + DACALIBRATION1) & SERIAL_OUT_BIT)
			value |= 1 << (value_width - i);
	}

	return value;
}

/* lowlevel write to eeprom/dac */
static void cb_pcidda_serial_out(struct comedi_device *dev, unsigned int value,
				 unsigned int num_bits)
{
	struct cb_pcidda_private *devpriv = dev->private;
	int i;

	for (i = 1; i <= num_bits; i++) {
		/*  send bits most significant bit first */
		if (value & (1 << (num_bits - i)))
			devpriv->dac_cal1_bits |= SERIAL_IN_BIT;
		else
			devpriv->dac_cal1_bits &= ~SERIAL_IN_BIT;
		outw_p(devpriv->dac_cal1_bits, dev->iobase + DACALIBRATION1);
	}
}

/* reads a 16 bit value from board's eeprom */
static unsigned int cb_pcidda_read_eeprom(struct comedi_device *dev,
					  unsigned int address)
{
	unsigned int i;
	unsigned int cal2_bits;
	unsigned int value;
	/* one caldac for every two dac channels */
	const int max_num_caldacs = 4;
	/* bits to send to tell eeprom we want to read */
	const int read_instruction = 0x6;
	const int instruction_length = 3;
	const int address_length = 8;

	/*  send serial output stream to eeprom */
	cal2_bits = SELECT_EEPROM_BIT | DESELECT_REF_DAC_BIT | DUMMY_BIT;
	/*  deactivate caldacs (one caldac for every two channels) */
	for (i = 0; i < max_num_caldacs; i++)
		cal2_bits |= DESELECT_CALDAC_BIT(i);
	outw_p(cal2_bits, dev->iobase + DACALIBRATION2);

	/*  tell eeprom we want to read */
	cb_pcidda_serial_out(dev, read_instruction, instruction_length);
	/*  send address we want to read from */
	cb_pcidda_serial_out(dev, address, address_length);

	value = cb_pcidda_serial_in(dev);

	/*  deactivate eeprom */
	cal2_bits &= ~SELECT_EEPROM_BIT;
	outw_p(cal2_bits, dev->iobase + DACALIBRATION2);

	return value;
}

/* writes to 8 bit calibration dacs */
static void cb_pcidda_write_caldac(struct comedi_device *dev,
				   unsigned int caldac, unsigned int channel,
				   unsigned int value)
{
	unsigned int cal2_bits;
	unsigned int i;
	/* caldacs use 3 bit channel specification */
	const int num_channel_bits = 3;
	const int num_caldac_bits = 8;	/*  8 bit calibration dacs */
	/* one caldac for every two dac channels */
	const int max_num_caldacs = 4;

	/* write 3 bit channel */
	cb_pcidda_serial_out(dev, channel, num_channel_bits);
	/*  write 8 bit caldac value */
	cb_pcidda_serial_out(dev, value, num_caldac_bits);

/*
* latch stream into appropriate caldac deselect reference dac
*/
	cal2_bits = DESELECT_REF_DAC_BIT | DUMMY_BIT;
	/*  deactivate caldacs (one caldac for every two channels) */
	for (i = 0; i < max_num_caldacs; i++)
		cal2_bits |= DESELECT_CALDAC_BIT(i);
	/*  activate the caldac we want */
	cal2_bits &= ~DESELECT_CALDAC_BIT(caldac);
	outw_p(cal2_bits, dev->iobase + DACALIBRATION2);
	/*  deactivate caldac */
	cal2_bits |= DESELECT_CALDAC_BIT(caldac);
	outw_p(cal2_bits, dev->iobase + DACALIBRATION2);
}

/* returns caldac that calibrates given analog out channel */
static unsigned int caldac_number(unsigned int channel)
{
	return channel / 2;
}

/* returns caldac channel that provides fine gain for given ao channel */
static unsigned int fine_gain_channel(unsigned int ao_channel)
{
	return 4 * (ao_channel % 2);
}

/* returns caldac channel that provides coarse gain for given ao channel */
static unsigned int coarse_gain_channel(unsigned int ao_channel)
{
	return 1 + 4 * (ao_channel % 2);
}

/* returns caldac channel that provides coarse offset for given ao channel */
static unsigned int coarse_offset_channel(unsigned int ao_channel)
{
	return 2 + 4 * (ao_channel % 2);
}

/* returns caldac channel that provides fine offset for given ao channel */
static unsigned int fine_offset_channel(unsigned int ao_channel)
{
	return 3 + 4 * (ao_channel % 2);
}

/* returns eeprom address that provides offset for given ao channel and range */
static unsigned int offset_eeprom_address(unsigned int ao_channel,
					  unsigned int range)
{
	return 0x7 + 2 * range + 12 * ao_channel;
}

/*
 * returns eeprom address that provides gain calibration for given ao
 * channel and range
 */
static unsigned int gain_eeprom_address(unsigned int ao_channel,
					unsigned int range)
{
	return 0x8 + 2 * range + 12 * ao_channel;
}

/*
 * returns upper byte of eeprom entry, which gives the coarse adjustment
 * values
 */
static unsigned int eeprom_coarse_byte(unsigned int word)
{
	return (word >> 8) & 0xff;
}

/* returns lower byte of eeprom entry, which gives the fine adjustment values */
static unsigned int eeprom_fine_byte(unsigned int word)
{
	return word & 0xff;
}

/* set caldacs to eeprom values for given channel and range */
static void cb_pcidda_calibrate(struct comedi_device *dev, unsigned int channel,
				unsigned int range)
{
	struct cb_pcidda_private *devpriv = dev->private;
	unsigned int coarse_offset, fine_offset, coarse_gain, fine_gain;

	/* remember range so we can tell when we need to readjust calibration */
	devpriv->ao_range[channel] = range;

	/*  get values from eeprom data */
	coarse_offset =
	    eeprom_coarse_byte(devpriv->eeprom_data
			       [offset_eeprom_address(channel, range)]);
	fine_offset =
	    eeprom_fine_byte(devpriv->eeprom_data
			     [offset_eeprom_address(channel, range)]);
	coarse_gain =
	    eeprom_coarse_byte(devpriv->eeprom_data
			       [gain_eeprom_address(channel, range)]);
	fine_gain =
	    eeprom_fine_byte(devpriv->eeprom_data
			     [gain_eeprom_address(channel, range)]);

	/*  set caldacs */
	cb_pcidda_write_caldac(dev, caldac_number(channel),
			       coarse_offset_channel(channel), coarse_offset);
	cb_pcidda_write_caldac(dev, caldac_number(channel),
			       fine_offset_channel(channel), fine_offset);
	cb_pcidda_write_caldac(dev, caldac_number(channel),
			       coarse_gain_channel(channel), coarse_gain);
	cb_pcidda_write_caldac(dev, caldac_number(channel),
			       fine_gain_channel(channel), fine_gain);
}

static int cb_pcidda_ao_winsn(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      struct comedi_insn *insn, unsigned int *data)
{
	struct cb_pcidda_private *devpriv = dev->private;
	unsigned int command;
	unsigned int channel, range;

	channel = CR_CHAN(insn->chanspec);
	range = CR_RANGE(insn->chanspec);

	/*  adjust calibration dacs if range has changed */
	if (range != devpriv->ao_range[channel])
		cb_pcidda_calibrate(dev, channel, range);

	/* output channel configuration */
	command = NOSU | ENABLEDAC;

	/* output channel range */
	switch (range) {
	case 0:
		command |= BIP | RANGE10V;
		break;
	case 1:
		command |= BIP | RANGE5V;
		break;
	case 2:
		command |= BIP | RANGE2V5;
		break;
	case 3:
		command |= UNIP | RANGE10V;
		break;
	case 4:
		command |= UNIP | RANGE5V;
		break;
	case 5:
		command |= UNIP | RANGE2V5;
		break;
	}

	/* output channel specification */
	command |= channel << 2;
	outw(command, dev->iobase + DACONTROL);

	/* write data */
	outw(data[0], dev->iobase + DADATA + channel * 2);

	/* return the number of samples read/written */
	return 1;
}

static const void *cb_pcidda_find_boardinfo(struct comedi_device *dev,
					    struct pci_dev *pcidev)
{
	const struct cb_pcidda_board *thisboard;
	int i;

	for (i = 0; i < ARRAY_SIZE(cb_pcidda_boards); i++) {
		thisboard = &cb_pcidda_boards[i];
		if (thisboard->device_id != pcidev->device)
			return thisboard;
	}
	return NULL;
}

static int cb_pcidda_attach_pci(struct comedi_device *dev,
				struct pci_dev *pcidev)
{
	const struct cb_pcidda_board *thisboard;
	struct cb_pcidda_private *devpriv;
	struct comedi_subdevice *s;
	unsigned long iobase_8255;
	int i;
	int ret;

	thisboard = cb_pcidda_find_boardinfo(dev, pcidev);
	if (!thisboard)
		return -ENODEV;
	dev->board_ptr = thisboard;
	dev->board_name = thisboard->name;

	devpriv = kzalloc(sizeof(*devpriv), GFP_KERNEL);
	if (!devpriv)
		return -ENOMEM;
	dev->private = devpriv;

	ret = comedi_pci_enable(pcidev, dev->board_name);
	if (ret)
		return ret;
	dev->iobase = pci_resource_start(pcidev, 3);
	iobase_8255 = pci_resource_start(pcidev, 2);

	if (thisboard->status == 2)
		printk
		    ("WARNING: DRIVER FOR THIS BOARD NOT CHECKED WITH MANUAL. "
		     "WORKS ASSUMING FULL COMPATIBILITY WITH PCI-DDA08/12. "
		     "PLEASE REPORT USAGE TO <ivanmr@altavista.com>.\n");

	ret = comedi_alloc_subdevices(dev, 3);
	if (ret)
		return ret;

	s = &dev->subdevices[0];
	/* analog output subdevice */
	s->type = COMEDI_SUBD_AO;
	s->subdev_flags = SDF_WRITABLE;
	s->n_chan = thisboard->ao_chans;
	s->maxdata = (1 << thisboard->ao_bits) - 1;
	s->range_table = &cb_pcidda_ranges;
	s->insn_write = cb_pcidda_ao_winsn;

	/* two 8255 digital io subdevices */
	for (i = 0; i < 2; i++) {
		s = &dev->subdevices[1 + i];
		ret = subdev_8255_init(dev, s, NULL, iobase_8255 + (i * 4));
		if (ret)
			return ret;
	}

	/* Read the caldac eeprom data */
	for (i = 0; i < EEPROM_SIZE; i++)
		devpriv->eeprom_data[i] = cb_pcidda_read_eeprom(dev, i);

	/*  set calibrations dacs */
	for (i = 0; i < thisboard->ao_chans; i++)
		cb_pcidda_calibrate(dev, i, devpriv->ao_range[i]);

	dev_info(dev->class_dev, "%s attached\n", dev->board_name);

	return 0;
}

static void cb_pcidda_detach(struct comedi_device *dev)
{
	struct pci_dev *pcidev = comedi_to_pci_dev(dev);

	if (dev->subdevices) {
		subdev_8255_cleanup(dev, &dev->subdevices[1]);
		subdev_8255_cleanup(dev, &dev->subdevices[2]);
	}
	if (pcidev) {
		if (dev->iobase)
			comedi_pci_disable(pcidev);
	}
}

static struct comedi_driver cb_pcidda_driver = {
	.driver_name	= "cb_pcidda",
	.module		= THIS_MODULE,
	.attach_pci	= cb_pcidda_attach_pci,
	.detach		= cb_pcidda_detach,
};

static int __devinit cb_pcidda_pci_probe(struct pci_dev *dev,
					 const struct pci_device_id *ent)
{
	return comedi_pci_auto_config(dev, &cb_pcidda_driver);
}

static void __devexit cb_pcidda_pci_remove(struct pci_dev *dev)
{
	comedi_pci_auto_unconfig(dev);
}

static DEFINE_PCI_DEVICE_TABLE(cb_pcidda_pci_table) = {
	{ PCI_DEVICE(PCI_VENDOR_ID_CB, PCI_DEVICE_ID_DDA02_12) },
	{ PCI_DEVICE(PCI_VENDOR_ID_CB, PCI_DEVICE_ID_DDA04_12) },
	{ PCI_DEVICE(PCI_VENDOR_ID_CB, PCI_DEVICE_ID_DDA08_12) },
	{ PCI_DEVICE(PCI_VENDOR_ID_CB, PCI_DEVICE_ID_DDA02_16) },
	{ PCI_DEVICE(PCI_VENDOR_ID_CB, PCI_DEVICE_ID_DDA04_16) },
	{ PCI_DEVICE(PCI_VENDOR_ID_CB, PCI_DEVICE_ID_DDA08_16) },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, cb_pcidda_pci_table);

static struct pci_driver cb_pcidda_pci_driver = {
	.name		= "cb_pcidda",
	.id_table	= cb_pcidda_pci_table,
	.probe		= cb_pcidda_pci_probe,
	.remove		= __devexit_p(cb_pcidda_pci_remove),
};
module_comedi_pci_driver(cb_pcidda_driver, cb_pcidda_pci_driver);

MODULE_AUTHOR("Comedi http://www.comedi.org");
MODULE_DESCRIPTION("Comedi low-level driver");
MODULE_LICENSE("GPL");
