/*
 *  HID driver for some sony "special" devices
 *
 *  Copyright (c) 1999 Andreas Gal
 *  Copyright (c) 2000-2005 Vojtech Pavlik <vojtech@suse.cz>
 *  Copyright (c) 2005 Michael Haboustak <mike-@cinci.rr.com> for Concept2, Inc
 *  Copyright (c) 2007 Paul Walmsley
 *  Copyright (c) 2008 Jiri Slaby
 *  Copyright (c) 2006-2008 Jiri Kosina
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>

static int atari_raw_event(struct hid_device *hdev, struct hid_report *report,
		__u8 *rd, int size)
{
	__u8 ud, lr;
	
	/*
		rd[0] specifies the player: 1 or 2
		rd[1] is a bit mask of 1 = right, 2 = left, 4 = down, 8 = up
		rd[2] is fire button pressed
	*/
/*	printk(KERN_ALERT "atari:  before %d: %x %x %x\n", size, rd[0], rd[1], rd[2]);*/
	lr = rd[1] & 0x03;
	if (lr == 2) rd[1] = (rd[1] & ~0x03) | 0x03;
	ud = rd[1] & 0x0c;
	if (ud == 8) rd[1] = (rd[1] & ~0x0c) | 0x0c;
/*	rd[1] = (rd[1] & 0xfe) | (~rd[1] & 0x01);*/
/*	printk(KERN_ALERT "after %d: %x %x %x\n", size, rd[0], rd[1], rd[2]);*/
	return 0;
}

static int atari_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret;

/*	printk(KERN_ALERT "atari_probe\n");*/

	hdev->quirks |= id->driver_data;

	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "parse failed\n");
		goto err_free;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		goto err_free;
	}

	if (ret < 0)
		goto err_stop;

	return 0;
err_stop:
	hid_hw_stop(hdev);
err_free:
	return ret;
}

static void atari_remove(struct hid_device *hdev)
{
	hid_hw_stop(hdev);
	kfree(hid_get_drvdata(hdev));
}

#define USB_VENDOR_ID_INNEX			0x1292
#define USB_DEVICE_ID_INNEX_ATARI_CONTROLLER	0x4154

static const struct hid_device_id atari_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_INNEX, USB_DEVICE_ID_INNEX_ATARI_CONTROLLER),
		.driver_data = HID_QUIRK_MULTI_INPUT },
	{ }
};
MODULE_DEVICE_TABLE(hid, atari_devices);

static struct hid_driver atari_driver = {
	.name = "atari",
	.id_table = atari_devices,
	.probe = atari_probe,
	.remove = atari_remove,
	.raw_event = atari_raw_event
};

static int __init atari_init(void)
{
	return hid_register_driver(&atari_driver);
}

static void __exit atari_exit(void)
{
	hid_unregister_driver(&atari_driver);
}

module_init(atari_init);
module_exit(atari_exit);
MODULE_LICENSE("GPL");
