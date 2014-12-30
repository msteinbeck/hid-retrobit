hid-retrobit is a Linux driver for the retro-bit (sometime called Retro Link) Atari, 
Nintendo NES, SNES & Sega Genesis joystick and gamepad controller adapters. 
It allows the use of actual compatible devices in emulators.

http://www.amazon.com/gp/product/B007ZS35CU/  
http://www.amazon.com/gp/product/B0080RAT42/

This driver is based on https://github.com/robmcmullen/hid-atari-retrobit.
Unfortunately it looks that way that RobMcMulle doesn't maintain it anymore.
However hid-retrobit is an improved version as it is much easier to install.
If you are interested in the technical details have a look at [Details](/DETAILS.md).

Installation
=====

Install the driver with the following commands:
```bash
make              # build the driver
sudo make install # install the driver
sudo depmod -a    # update the module dependency graph
```

Now you should be able to load the driver with:  
(the dirver was successfully loaded if there is no terminal output)
```bash
sudo modprobe hid-retrobit
```

Unfortunately the driver *hid-generic* binds all HID devices with the result that 
*hid-retrobit* is unable to do its job. The file **99-hid-retrobit.rules** contains
some udev rules which unbinds all supported devices from *hid-generic* and rebinds
them to *hid-retrobit* on the fly. Copy the file to **/lib/udev/rules.d** or, if this
directory doesn't exist, to **/etc/udev/rules.d**. Finally, you need to load some 
modules during boot time automatically. If the drivers *usbhid* and *hid-generic* 
are build as module (most operating systems have this) you need to load 
*hid-retrobit*, *usbhid* and *hid-generic*. If the drivers are build as build-in you 
only need to load *hid-retrobit*. You can check your configuration for *usbhid* with:
```bash
cat /boot/config-`uname -r` | grep CONFIG_USB_HID
```

and for *hid-generic* with:
```bash
cat /boot/config-`uname -r` | grep CONFIG_HID_GENERIC
```

If the value is *m*, the dirver is build as module. If the value is *y*, the driver is
build as build-in. To load the module(s) automatically, open the file **/etc/modules**
(if exists) and append them. If, for example, you need to load *hid-retrobit*, *usbhid* 
and *hid-generic* append the following entries:
```bash
hid-retrobit
usbhid
hid-generic
```

If the file **/etc/modules** doesn't exist, create the file **hid-retrobit.conf** in
**/etc/modules-load.d**.

Now everything should be ready. Have fun playing games retro style :).

Alternate solution
==================

If the following driver does not work, the desperate solution is to
recompile your kernel. It could be a lengthy and tedious task and should
be repeated on every incoming security patch or on your Linux distro updates.

Here is the fix. The Retrolink adapter does not follow the USB HIB specification 
(section 5.10 and 6.2.25) and returns out-of-bound values. 
Contrary to the former, the HID driver strictly follows the specification and
ignores such values. The fix consists in clamping the values instead of discarding them.
        
In your linux kernel source directory, edit the file /drivers/hid/hid-input.c

Search for::

    if ((field->flags & HID_MAIN_ITEM_VARIABLE) &&
        (value < field->logical_minimum ||
        value > field->logical_maximum)) {
            dbg_hid("Ignoring out-of-range value %x\n", value);
            return;
    }

and replace by::

    if ((field->flags & HID_MAIN_ITEM_VARIABLE) &&
        (value < field->logical_minimum ||
        value > field->logical_maximum)) {
            if(value < field->logical_minimum)
                value = field->logical_minimum;
            else
                value = field->logical_maximum;
    }

This fix could be improved by a new USB quirk assigned to Retrolink devices.




