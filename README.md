hid-retrobit is a Linux driver for the Retro-Bit (also known as Retro-Link)
Atari, Nintendo NES, SNES, and Sega Genesis USB adapters. This driver enables
you to use supported joysticks and gamepads in emulators.

http://www.amazon.com/gp/product/B007ZS35CU/  
http://www.amazon.com/gp/product/B0080RAT42/

This driver is based on https://github.com/robmcmullen/hid-atari-retrobit.
Unfortunately, it looks like RobMcMulle doesn't maintain it anymore. However,
hid-retrobit is an improved version as it is much easier to install and use.
Have a look at [Details](/DETAILS.md) for more information.

Installation
=====

##### Raspberry Pi (RetroPie/Raspbian)

1. Download the latest deb package from:
   https://github.com/retuxx/hid-retrobit/releases.
2. Install linux headers and dkms:  
   ```
   sudo apt-get install raspberrypi-kernel-headers dkms
   ```
3. Install the driver:  
   ```
   sudo dpkg -i hid-retrobit.deb
   ```
4. Plugin your device.

##### Debian, Ubuntu, SteamOS, ...

1. Download the latest deb package from:
   https://github.com/retuxx/hid-retrobit/releases.
2. Install dkms:  
   ```
   sudo apt-get install dkms
   ```
3. Install the driver:  
   ```
   sudo dpkg -i hid-retrobit.deb
   ```
4. Plugin your device.

##### Fedora, CentOS, ...


1. Download the latest rpm package from:
   https://github.com/retuxx/hid-retrobit/releases.
2. Make sure you have installed the latest kernel. Otherwise, dkms is unable to
   build the module because the linux headers are missing:  
   ```
   sudo yum update
   ```
3. Reboot your system.
4. Install linux headers:  
   ```
   sudo yum install kernel-devel
   ```
5. Install the driver:  
   ```
   sudo yum localinstall hid-retrobit-1.0.0-1.noarch.rpm
   ```
6. Plugin your device.

##### Manual

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

Unfortunately, the driver *hid-generic* prevents *hid-retrobit* from binding
your device. The file **99-hid-retrobit.rules** contains some udev rules which
unbinds all supported devices from *hid-generic* and rebinds them to
*hid-retrobit* on the fly. Copy this file to **/etc/udev/rules.d**. To ensure
the udev rules are able to rebind your devices, you finally need to copy the
file **hid-retrobit.conf** to **/etc/modules-load.d**.

Now your devices are ready to use. Restart your system and have fun playing 
games retro style :).

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




