ifeq ($(KERNELRELEASE),)

# We're outside Kbuild, so we define some targets that will run make from
# within the kernel source tree.  By default, we'll build against the same
# version as that of the running kernel.  Defining M tells kbuild we wish to
# build external modules in the given directory.

KERNELDIR ?= /lib/modules/$(shell uname -r)/build # ?= means set if not already
PWD := $(shell pwd) # := means don't defer substitution, evaluate it now

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

.PHONY: modules modules_install clean

else

# We're now being called from kbuild (i.e. make running within the kernel
# source tree), so all we need to do is define the module objects we want
# Kbuild to build.

obj-m := hello.o

endif
