#  Makefile to compile NI-KAL against the kernel
#
#  (C) Copyright 2002-2015,
#  National Instruments Corporation.
#  All Rights reserved.

# KERNELRELEASE is set, so we're being invoked from the kernel build system
ifneq ($(KERNELRELEASE),)

EXTRA_CFLAGS += $(shell echo $(KERNEL_VARIANTS))
EXTRA_CFLAGS += -DnNIKAL100_kTargetName=\"nikal.ko\"

obj-m := nikal.o

# KERNELRELEASE is not set, so this is a command-line invocation
else

include Makefile.in
export KERNEL_VARIANTS

PWD := $(shell pwd)

all: nikal.ko

install: $(MODPATH)/nikal/nikal.ko ../Module.symvers /usr/local/sbin/nidevnode

$(MODPATH)/nikal/nikal.ko: nikal.ko
	@mkdir -p $(MODPATH)/nikal
	@$(MAKE) --no-print-directory -C $(KERNELDIRECTORY) M=$(PWD) INSTALL_MOD_DIR=kernel/natinst/nikal modules_install

nikal.ko: nikal.c nikal.h Makefile.in
	@$(MAKE) --no-print-directory -C $(KERNELDIRECTORY) M=$(PWD) modules

../Module.symvers: Module.symvers
	@cp -f $< $@

Module.symvers: nikal.ko

nidevnode: nidevnode.c
	@gcc $(CFLAGS) $(LDFLAGS) -o $@ $<

/usr/local/sbin/nidevnode: nidevnode
	@ln -sf $(PWD)/nidevnode /usr/local/sbin/nidevnode

uninstall:
	@rm -rf $(MODPATH)/nikal/nikal.ko
	@rm -f /usr/local/sbin/nidevnode

clean:
	@$(MAKE) --no-print-directory -s -C $(KERNELDIRECTORY) M=$(PWD) clean
	@rm -f /usr/local/sbin/nidevnode nidevnode
	@rm -rf Module.* ../Module.symvers

.PHONY: all install uninstall clean

endif
