Problem
----------------------

The default joydev/hid-input driver doesn't handle the axes correctly, as only right
and down work properly. It doesn't register left or up. jstest shows the device, but 
it can't generate the -32767 value expected for left or up. The following output
shows an Atari joystick which registered right and down simultaneously::

    # jstest /dev/input/js0 
    Driver version is 2.1.0.
    Joystick (INNEX ATARI Controller USB) has 4 axes (X, Y, Z, Rx)
    and 2 buttons (Trigger, ThumbBtn).
    Testing ... (interrupt to exit)
    Axes:  0: 32767  1: 32767  2:     0  3:     0 Buttons:  0:off  1:off 

By using DEBUG_FS, the joystick shows up in /sys/kernel/debug/usb/devices as::

    T:  Bus=08 Lev=01 Prnt=01 Port=03 Cnt=01 Dev#=  2 Spd=1.5  MxCh= 0
    D:  Ver= 1.10 Cls=00(>ifc ) Sub=00 Prot=00 MxPS= 8 #Cfgs=  1
    P:  Vendor=1292 ProdID=4154 Rev= 1.00
    S:  Manufacturer=INNEX
    S:  Product=ATARI Controller USB
    C:* #Ifs= 1 Cfg#= 1 Atr=80 MxPwr=300mA
    I:* If#= 0 Alt= 0 #EPs= 1 Cls=03(HID  ) Sub=00 Prot=00 Driver=usbhid
    E:  Ad=81(I) Atr=03(Int.) MxPS=   8 Ivl=10ms

lsusb outputs::

    # lsusb
    Bus 008 Device 003: ID 1292:4154 Innomedia 

Not a hardware problem
----------------------

Plugging the joystick in to a Windows machine works correctly out of the box. By 
using the usbmon kernel module, one can see that the device is generating events 
for all the directions, so it isn't a hardware problem.

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

There are some flags called HID Quirks that are based on USB ID.

##### HID_QUIRK_MULTI_INPUT

RobMcMullen found out, that turning on the HID_QUIRK_MULTI_INPUT (0x40) separates out 
the two devices so that /dev/input/js0 and /dev/input/js1 are available. Furthermore 
jstest shows 2 axes and one button for each joystick now. But it doesn't correct the 
left/up problem itself::

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

##### HID_QUIRK_BADPAD

The HID_QUIRK_BADPAD (0x20) allows jstest to recognize left and up by marking left
as 2, center as 0 and right as 1. Unfortunately it appears that jscal only has
linear coefficients and can't cope with that because it somehow assumes that left will 
always be less than center and center will always be less than right.

Thanks again to RobMcMullen who spent a long time trying to figure out the problem
(jscal has very limited documentation).


Driver
----------

This device driver relies on the accidental discovery that when both left and
right are pressed at the same time, joydev produces a correct result for left.
(Similarly for up and down).

Since this isn't applicable for real joysticks and gamepads, we can replace the 
value for left with (left & right) and up with (up & down) without changing any 
other aspects of the operation of the device.
