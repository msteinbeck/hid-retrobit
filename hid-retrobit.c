/*
 *
 * HID driver for retro-bit/Retro Link controller adapters.
 * URL: https://github.com/retuxx/hid-retrobit
 *
 * Copyright (c) 2013 RobMcMullen
 *
 * This file is part of hid-retrobit.
 *
 * hid-retrobit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hid-retrobit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hid-retrobit.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>


/********************************************************
*                                                       *
* Setup supported devices                               *
*                                                       *
********************************************************/
#define USB_VENDOR_ID_INNEX                  0x1292
#define USB_DEVICE_ID_INNEX_ATARI_CONTROLLER 0x4154
#define USB_DEVICE_ID_INNEX_NES_CONTROLLER   0x4643
#define USB_DEVICE_ID_INNEX_SNES_ADAPTER     0x5346
#define USB_DEVICE_ID_INNEX_GENESIS_ADAPTER  0x4745

static const struct hid_device_id retrobit_devices[] = {
    {
        HID_USB_DEVICE(USB_VENDOR_ID_INNEX, USB_DEVICE_ID_INNEX_ATARI_CONTROLLER),
        .driver_data = HID_QUIRK_MULTI_INPUT
    },
    {
        HID_USB_DEVICE(USB_VENDOR_ID_INNEX, USB_DEVICE_ID_INNEX_NES_CONTROLLER),
        .driver_data = 0x0 // single controller only
    },
    {
        HID_USB_DEVICE(USB_VENDOR_ID_INNEX, USB_DEVICE_ID_INNEX_SNES_ADAPTER),
        .driver_data = HID_QUIRK_MULTI_INPUT
    },
    {
        HID_USB_DEVICE(USB_VENDOR_ID_INNEX, USB_DEVICE_ID_INNEX_GENESIS_ADAPTER),
        .driver_data = HID_QUIRK_MULTI_INPUT
    },
    {}
};

MODULE_DEVICE_TABLE(hid, retrobit_devices);


/********************************************************
*                                                       *
* Callbacks                                             *
*                                                       *
********************************************************/
static int retrobit_raw_event(
    struct hid_device *hdev, struct hid_report *report, __u8 *rawdata, int size
)
{
    // rawdata[0] specifies controller 1 or 2 in dual controller adapters,
    //      is axes/Directional-Pad data in single controller adapters
    //
    // rawdata[1] axes/Directional-Pad data in dual controller adapters,
    //      is button presses in single controller adapters
    //      is axes/Directional-Pad data in single controller adapters
    //
    // rawdata[2] is button presses in dual controller adapters
    
    const __u8 dpad = (hdev->quirks & HID_QUIRK_MULTI_INPUT) ? 1 : 0;
    const __u8 lr = rawdata[dpad] & 0x03;
    const __u8 ud = rawdata[dpad] & 0x0c;
    
    // map left to left/right
    if (lr == 2) {
        rawdata[dpad] = (rawdata[dpad] & ~0x03) | 0x03;
    }
    
    // map up to up/down
    if (ud == 8) {
        rawdata[dpad] = (rawdata[dpad] & ~0x0c) | 0x0c;
    }

    return 0;
}

static int retrobit_probe(
    struct hid_device *hdev, const struct hid_device_id *id
)
{
    int ret;

    // add quirks, see struct retrobit_devices
    hdev->quirks |= id->driver_data;

    ret = hid_parse(hdev);
    if (ret) {
        hid_err(hdev, "parse failed\n");
        return ret;
    }

    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret) {
        hid_err(hdev, "hw start failed\n");
        return ret;
    }

    if (ret < 0) {
        hid_hw_stop(hdev);
        return ret;
    }

    return 0;
}

static void retrobit_remove(struct hid_device *hdev)
{
    hid_hw_stop(hdev);
    kfree(hid_get_drvdata(hdev));
}


/********************************************************
*                                                       *
* Setup module                                          *
*                                                       *
********************************************************/
static struct hid_driver retrobit_driver = {
    .name      = "hid-retrobit",
    .id_table  = retrobit_devices,
    .probe     = retrobit_probe,
    .remove    = retrobit_remove,
    .raw_event = retrobit_raw_event
};

static int __init retrobit_init(void)
{
    return hid_register_driver(&retrobit_driver);
}

static void __exit retrobit_exit(void)
{
    hid_unregister_driver(&retrobit_driver);
}

module_init(retrobit_init);
module_exit(retrobit_exit);
MODULE_LICENSE("GPL");
