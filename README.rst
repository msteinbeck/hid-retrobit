Driver for the Retro-Bit Atari, Nintendo NES, SNES & Sega Genesis joystick controller adapters

This is a USB to dual-DB9, NES/SNES or Genesis header controller that allows the use of actual 
compatible joysticks in emulators.

http://www.amazon.com/gp/product/B007ZS35CU/


Problem
=======

The normal joydev/hid-input driver doesn't handle this correctly, as only right
and down work.  It doesn't register left or up.  Tried 3 different machines,
both amd64 and x86.  jstest shows the controller, but it can't generate the
-32767 value expected for up or left.  Here the joystick is registering down
and right simultaneously::

    # jstest /dev/input/js0 
    Driver version is 2.1.0.
    Joystick (INNEX ATARI Controller USB) has 4 axes (X, Y, Z, Rx)
    and 2 buttons (Trigger, ThumbBtn).
    Testing ... (interrupt to exit)
    Axes:  0: 32767  1: 32767  2:     0  3:     0 Buttons:  0:off  1:off 

Using DEBUG_FS, the device shows up in /sys/kernel/debug/usb/devices as::

    T:  Bus=08 Lev=01 Prnt=01 Port=03 Cnt=01 Dev#=  2 Spd=1.5  MxCh= 0
    D:  Ver= 1.10 Cls=00(>ifc ) Sub=00 Prot=00 MxPS= 8 #Cfgs=  1
    P:  Vendor=1292 ProdID=4154 Rev= 1.00
    S:  Manufacturer=INNEX
    S:  Product=ATARI Controller USB
    C:* #Ifs= 1 Cfg#= 1 Atr=80 MxPwr=300mA
    I:* If#= 0 Alt= 0 #EPs= 1 Cls=03(HID  ) Sub=00 Prot=00 Driver=usbhid
    E:  Ad=81(I) Atr=03(Int.) MxPS=   8 Ivl=10ms

or using lsusb::

    Bus 008 Device 003: ID 1292:4154 Innomedia 

Not a hardware problem
----------------------

Plugging it in to a windows machine works correctly.  Using the usbmon
kernel module, I found out that the device is generating events for all the
directions, so it isn't a hardware problem.

Player 1::

    FIRE:  ffff8805966fda80 1370147216 C Ii:8:003:1 0:8 8 = 01000100 00000000
    RIGHT: ffff8805966fda80 1514147303 C Ii:8:003:1 0:8 8 = 01010000 00000000
    LEFT:  ffff8805966fda80 1586219357 C Ii:8:003:1 0:8 8 = 01020000 00000000
    DOWN:  ffff8805966fda80 1600331288 C Ii:8:003:1 0:8 8 = 01040000 00000000
    UP:    ffff8805966fda80 1625075305 C Ii:8:003:1 0:8 8 = 01080000 00000000

Player 2::

    FIRE:  ffff8805966fda80 1691635346 C Ii:8:003:1 0:8 8 = 02000100 00000000
    RIGHT: ffff8805966fda80 1718227439 C Ii:8:003:1 0:8 8 = 02010000 00000000
    LEFT:  ffff8805966fda80 1730515446 C Ii:8:003:1 0:8 8 = 02020000 00000000
    DOWN:  ffff8805966fda80 1745035454 C Ii:8:003:1 0:8 8 = 02040000 00000000
    UP:    ffff8805966fda80 1770787482 C Ii:8:003:1 0:8 8 = 02080000 00000000

From the 8 bytes of data (here called rd)::

    rd[0] specifies the player: 1 or 2
    rd[1] is a bit mask of 1 = right, 2 = left, 4 = down, 8 = up
    rd[2] is fire button pressed

USB Quirks
----------

Found one issue: there are something flags called HID Quirks that are based
on USB ID.  Turning on the HID_QUIRK_MULTI_INPUT (0x40) separates out the two
controllers so there are /dev/input/js0 and /dev/input/js1, and jstest shows 2
axes and one button for each joystick now.  But it doesn't correct the up/left
problem::

    # rmmod usbhid
    # modprobe usbhid quirks=0x1292:0x4154:0x40
    # jstest /dev/input/js0 
    Driver version is 2.1.0.
    Joystick (INNEX ATARI Controller USB) has 2 axes (X, Y)
    and 1 buttons (Trigger).
    Testing ... (interrupt to exit)
    Axes:  0:     0  1:     0 Buttons:  0:off
    # jstest /dev/input/js1 
    Driver version is 2.1.0.
    Joystick (INNEX ATARI Controller USB) has 2 axes (X, Y)
    and 1 buttons (Trigger).
    Testing ... (interrupt to exit)
    Axes:  0:     0  1:     0 Buttons:  0:off

HID_QUIRK_BADPAD
................

Thought I was getting somewhere when I discovered the HID_QUIRK_BADPAD (0x20),
which then allows jstest to recognize left and up, but unfortunately it marks
left as 2, center as 0 and right as 1.  I spent a long time trying to figure
out jscal with its limited documentation and it appears that jscal only has
linear coefficients and can't cope with that.  It appears to assume that left
will always be less than center and center always less than right.



Driver
======

This device driver relies on the accidental discovery that when both left and
right are pressed at the same time, joydev produces a correct result for left.
(Similarly for up and down simulaneously).  Haven't figured out why because
I haven't been able to trace through the HID input code.

Since it doesn't happen in a real joystick, we can replace the value for left
with (left & right) and not change any other aspects of the operation of the
stick.


Usage
=====

Compile with::

    # make

The usbhid module appears to handle all USB IDs if it's already loaded, so for
testing you must remove it first::

    # rmmod ./hid-atari-retrobit.ko; rmmod usbhid; insmod ./hid-atari-retrobit.ko ; modprobe usbhid

If you have no other USB input device, your favourite paddle just now works !

    # jstest /dev/input/js0
    
Pressing the up/left key should returns the value -32767.

However, if you have any USB input device plugged in, such as a simple USB
keyboard, the usbhid driver binds the paddle before the retrobit driver. 
You have to unbind the adapter from usbhid. Download and execute the following script.

    # sudo ./installdriver.sh


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




