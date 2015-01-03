hid-retrobit is a Linux driver for the retro-bit (sometime called Retro Link) Atari, 
Nintendo NES, SNES & Sega Genesis joystick and gamepad controller adapters. 
It allows the use of actual compatible devices in emulators.

http://www.amazon.com/gp/product/B007ZS35CU/  
http://www.amazon.com/gp/product/B0080RAT42/

This driver is based on https://github.com/robmcmullen/hid-atari-retrobit.
Unfortunately it looks that way that RobMcMulle doesn't maintain it anymore.
However hid-retrobit is an improved version as it is much easier to install/use.
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
them to *hid-retrobit* on the fly. Copy this file to **/lib/udev/rules.d**. If the
directory doesn't exist, copy the file to **/etc/udev/rules.d**. 

To ensure that the udev rules are able to rebind the supported devices, you finally 
need to copy the file **hid-retrobit.conf** to **/etc/modules-load.d**.

Now your devices are ready to use. Restart your system and have fun playing games 
retro style :).

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

Search for:

    if ((field->flags & HID_MAIN_ITEM_VARIABLE) &&
        (value < field->logical_minimum ||
        value > field->logical_maximum)) {
            dbg_hid("Ignoring out-of-range value %x\n", value);
            return;
    }

and replace by:

    if ((field->flags & HID_MAIN_ITEM_VARIABLE) &&
        (value < field->logical_minimum ||
        value > field->logical_maximum)) {
            if(value < field->logical_minimum)
                value = field->logical_minimum;
            else
                value = field->logical_maximum;
    }

This fix could be improved by a new USB quirk assigned to Retrolink devices.




