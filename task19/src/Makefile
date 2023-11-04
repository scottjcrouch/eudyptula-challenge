ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

sparse:
	$(MAKE) C=2 CF='-fdiagnostic-prefix=SPARSE' -C $(KERNELDIR) M=$(PWD) modules

checkpatch:
	$(KERNELDIR)/scripts/checkpatch.pl -f $(wildcard *.c) $(wildcard **/*.c) $(wildcard *.h) $(wildcard **/*.h)

clang-format:
	clang-format -i $(wildcard *.c) $(wildcard **/*.c) $(wildcard *.h) $(wildcard **/*.h)

clean:
	rm -rf *.o *.o.d *~ core .depend .*.cmd *.ko *.mod* .tmp_versions modules.order Module.symvers

.PHONY: modules modules_install sparse checkpatch clean

else

ifndef CONFIG_NETFILTER_XT_MATCH_STRING
  $(error This module requires CONFIG_NETFILTER_XT_MATCH_STRING)
endif

obj-m := eudyptula.o

endif
