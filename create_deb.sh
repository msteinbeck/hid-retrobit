#!/bin/bash

# to create a valid package we need to be root
# see https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=291320
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

# maybe an earlier run of this script failed
rm -rf hid-retrobit

# create working directory
mkdir -p hid-retrobit/etc/udev/rules.d
mkdir -p hid-retrobit/etc/modules-load.d
mkdir -p hid-retrobit/usr/src/hid-retrobit-1.0.0

# copy necessary files
cp -R DEBIAN hid-retrobit/
cp 99-hid-retrobit.rules hid-retrobit/etc/udev/rules.d/
cp hid-retrobit.conf hid-retrobit/etc/modules-load.d/
cp COPYING hid-retrobit/usr/src/hid-retrobit-1.0.0/
cp DETAILS.md hid-retrobit/usr/src/hid-retrobit-1.0.0/
cp dkms.conf hid-retrobit/usr/src/hid-retrobit-1.0.0/
cp hid-retrobit.c hid-retrobit/usr/src/hid-retrobit-1.0.0/
cp Makefile hid-retrobit/usr/src/hid-retrobit-1.0.0/
cp README.md hid-retrobit/usr/src/hid-retrobit-1.0.0/

# create package
dpkg-deb --build hid-retrobit

# cleanup
rm -rf hid-retrobit
