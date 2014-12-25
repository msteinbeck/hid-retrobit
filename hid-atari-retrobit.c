/*
 *  HID driver for Retrolink adapters & paddles devices
 *
 *  Copyright (c) 2013 RobMcMullen
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

#define DEBUG_ATARI

static int atari_raw_event(struct hid_device *hdev, struct hid_report *report,
    __u8 *rawdata, int size)
{    
  __u8 updown, leftright, dpad;
 
/*
        rawdata[0] specifies controller 1 or 2 in dual controller adapters,
                is axes/Directional-Pad data in single controller adapters
 
        rawdata[1] axes/Directional-Pad data in dual controller adapters,
                is button presses in single controller adapters
                is axes/Directional-Pad data in single controller adapters
 
         rawdata[2] is button presses in dual controller adapters
*/
  dpad = (hdev->quirks & HID_QUIRK_MULTI_INPUT) ? 1 : 0;

        leftright = rawdata[dpad] & 0x03;
        if (leftright == 2) rawdata[dpad] = (rawdata[dpad] & ~0x03) | 0x03;
        updown = rawdata[dpad] & 0x0c;
        if (updown == 8) rawdata[dpad] = (rawdata[dpad] & ~0x0c) | 0x0c;

 
        return 0;
}

static int atari_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
  int ret;

#ifdef DEBUG_ATARI
  printk(KERN_ALERT "atari_probe\n");
#endif

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

#define USB_VENDOR_ID_INNEX      0x1292
#define USB_DEVICE_ID_INNEX_ATARI_CONTROLLER  0x4154
#define USB_DEVICE_ID_INNEX_NES_CONTROLLER  0x4643
#define USB_DEVICE_ID_INNEX_SNES_ADAPTER  0x5346
#define USB_DEVICE_ID_INNEX_GENESIS_ADAPTER     0x4745

static const struct hid_device_id atari_devices[] = {
  { HID_USB_DEVICE(USB_VENDOR_ID_INNEX, USB_DEVICE_ID_INNEX_ATARI_CONTROLLER),
    .driver_data = HID_QUIRK_MULTI_INPUT },
  { HID_USB_DEVICE(USB_VENDOR_ID_INNEX, USB_DEVICE_ID_INNEX_NES_CONTROLLER),
     .driver_data = 0x0}, //single controller only
  { HID_USB_DEVICE(USB_VENDOR_ID_INNEX, USB_DEVICE_ID_INNEX_SNES_ADAPTER),
    .driver_data = HID_QUIRK_MULTI_INPUT },
  { HID_USB_DEVICE(USB_VENDOR_ID_INNEX, USB_DEVICE_ID_INNEX_GENESIS_ADAPTER),
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
#ifdef DEBUG_ATARI
  printk(KERN_ALERT "atari_init\n");
#endif
  return hid_register_driver(&atari_driver);
}

static void __exit atari_exit(void)
{
#ifdef DEBUG_ATARI
  printk(KERN_ALERT "atari_exit\n");
#endif
  hid_unregister_driver(&atari_driver);
}

module_init(atari_init);
module_exit(atari_exit);
MODULE_LICENSE("GPL");
