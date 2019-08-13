# Allow redirecting nidevnode installation dir
SBINDIR=/usr/sbin

# Inputs to Kbuild:
#   INSTALL_MOD_DIR:  the kernel module tree subdirectory to install
#                     a module to
#   INSTALL_MOD_PATH: alternate root location for the /lib/modules tree
#                     to install the module under
INSTALL_MOD_DIR=extra/natinst
INSTALL_MOD_PATH=


# Internal variables
#   MODULEDIR:        the directory in which the module to be built resides
#   KERNELVER:        kernel version number/name to build against
#   KERNELDIR:        location of the build Makefile being used to
#                     build the kernel module
#   SYMVERDIR:        the directory to cache per-module symbol version
#                     definitions
#   DEBUG:            enable additional debugging information, both during
#                     build as well as in the built kernel module
MODULEDIR ?= $(shell pwd)
KERNELVER ?= $(shell uname -r)

ifneq (,$(strip $(KERNELHEADERS)))
KERNELDIR ?= $(KERNELHEADERS)
else
KERNELDIR ?= /lib/modules/$(KERNELVER)/build
endif

SYMVERDIR  =/var/lib/nikal/$(KERNELVER)/symvers
DEBUG=

ifneq ($(DEBUG),)
Q=
KALDEBUG=debug
else
Q=@
endif

# Top-level targets
.PHONY: all build sysprep install uninstall clean sysclean distclean
all: install

build: nikal.ko
build: $(SYMVERDIR)/nikal.symvers

sysprep: nikal.ko $(SYMVERDIR)/nikal.symvers $(SBINDIR)/nidevnode
sysprep: $(SYMVERDIR)/nikal.symvers
sysprep: $(SBINDIR)/nidevnode

install: $(INSTALL_MOD_PATH)/lib/modules/$(KERNELVER)/$(INSTALL_MOD_DIR)/nikal.ko
install: $(SYMVERDIR)/nikal.symvers
install: $(SBINDIR)/nidevnode

# Build rules
nidevnode: nidevnode.c
	$(Q)gcc $(CFLAGS) $(LDFLAGS) -o $@ $<

$(SBINDIR)/nidevnode: nidevnode
	$(Q)mkdir -p $(@D)
	$(Q)cp -f $< $@

uninstall:
	$(Q)rm -f $(INSTALL_MOD_PATH)/lib/modules/$(KERNELVER)/$(INSTALL_MOD_DIR)/nikal.ko

clean:
	$(Q)$(MAKE) --no-print-directory -s -C $(KERNELDIR) M=$(MODULEDIR) $(if $(Q),,Q="") INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) clean
	$(Q)rm -f Kbuild 2> /dev/null

sysclean: clean
	$(Q)rm -f $(SBINDIR)/nidevnode nidevnode

distclean: clean sysclean uninstall
	$(Q)rm -f $(SBINDIR)/nidevnode nidevnode
	$(Q)rm -f $(SYMVERDIR)/nikal.symvers 2> /dev/null

$(INSTALL_MOD_PATH)/lib/modules/$(KERNELVER)/$(INSTALL_MOD_DIR)/nikal.ko: nikal.ko
	$(Q)echo Installing $<
	$(Q)$(MAKE) --no-print-directory -C $(KERNELDIR) M=$(MODULEDIR) $(if $(Q),,Q="") INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) modules_install

$(SYMVERDIR)/nikal.symvers: Module.symvers
	$(Q)echo Installing KAL symbol versions into:
	$(Q)echo $@
	$(Q)mkdir -p $(@D)
	$(Q)cp -f $< $@

Module.symvers: nikal.ko

nikal.ko: nikal.c nikal.h Kbuild
	$(Q)echo Making $@
	$(Q)$(MAKE) --no-print-directory -C $(KERNELDIR) M=$(MODULEDIR) $(if $(Q),,Q="") modules

Kbuild: configure
	$(Q)KERNELVER=$(KERNELVER) KERNELDIR=$(KERNELDIR) ./configure $(KALDEBUG)

