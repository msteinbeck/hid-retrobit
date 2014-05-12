#!/bin/bash

#install retrobit driver

rmmod ./hid-atari-retrobit.ko; rmmod usbhid;
insmod ./hid-atari-retrobit.ko ; modprobe usbhid;

#unbind from hid-generic
usbid=""
for i in `ls /sys/bus/hid/drivers/hid-generic/`; do echo $i; 
if [[ $i == *1292* ]]
then
  echo "Unbinding Retrolink device from hid-generic: $i";
  usbid=$i
fi;
done;

echo -n $usbid > /sys/bus/hid/drivers/hid-generic/unbind; 
echo -n $usbid > /sys/bus/hid/drivers/atari/bind
jstest /dev/input/js0
