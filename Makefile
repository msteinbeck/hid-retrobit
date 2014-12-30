# optimize module with O2
ccflags-y += -O2

# seperate the two parts of the Makefile (kbuild and normal)
# see https://www.kernel.org/doc/Documentation/kbuild/modules.txt
ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m := hid-retrobit.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

all:
	$(MAKE) -C $(KDIR) M=$$PWD modules

install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

help:
	@echo ""
	@echo "  all     - default target, build the module"
	@echo "  install - install the module (requires root privileges)"
	@echo "  clean   - remove generated files"
	@echo "  help    - display this text"
	@echo ""
endif

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
