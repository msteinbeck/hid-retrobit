#!/bin/bash

# maybe an earlier run of this script failed
rm -rf RPM/hid-retrobit

# create working directory
pushd RPM
./togo -c hid-retrobit
mkdir -p hid-retrobit/root/etc/udev/rules.d
mkdir -p hid-retrobit/root/etc/modules-load.d
mkdir -p hid-retrobit/root/usr/src/hid-retrobit-1.0.0
cp header hid-retrobit/spec/
cp post hid-retrobit/spec/
cp postun hid-retrobit/spec/
popd

# copy necessary files
cp 99-hid-retrobit.rules RPM/hid-retrobit/root/etc/udev/rules.d/
cp hid-retrobit.conf RPM/hid-retrobit/root/etc/modules-load.d/
cp COPYING RPM/hid-retrobit/root/usr/src/hid-retrobit-1.0.0/
cp DETAILS.md RPM/hid-retrobit/root/usr/src/hid-retrobit-1.0.0/
cp dkms.conf RPM/hid-retrobit/root/usr/src/hid-retrobit-1.0.0/
cp hid-retrobit.c RPM/hid-retrobit/root/usr/src/hid-retrobit-1.0.0/
cp Makefile RPM/hid-retrobit/root/usr/src/hid-retrobit-1.0.0/
cp README.md RPM/hid-retrobit/root/usr/src/hid-retrobit-1.0.0/

pushd RPM/hid-retrobit # otherwise togo -f fails
# exclude ownership
../togo -f root/etc/udev/rules.d
../togo -f root/etc/modules-load.d

# create package
../togo -f root/usr/src/hid-retrobit-1.0.0
../togo -bp
popd

# move rpm to root dir
mv RPM/hid-retrobit/rpms/hid-retrobit-1.0.0-1.noarch.rpm ./

# cleanup
rm -rf RPM/hid-retrobit
