EXTERNAL_BUILD=y
CONFIG_HODOR=m

list-m :=
list-$(CONFIG_HODOR) += hodor
obj-$(CONFIG_HODOR) += hodor.o
hodor-objs := core.o breakpoint.o

KVER  := $(shell uname -r)
EXTRACT := $(M)/extract_symbol.sh
KSRC := /lib/modules/$(KVER)/build
KMISC := /lib/modules/$(KVER)/kernel/drivers/misc
KMISC_INC := /lib/modules/$(KVER)/include

PWD=$(shell pwd)

all: modules

clean:
	rm -f *.mod.c *.mod *.o *.ko .*.cmd .*.flags .lst *.lst *.order *.symvers
	rm -rf $(PWD)/tmp

TMP=$(PWD)/tmp
MODVERDIR=$(TMP)/.tmp_versions

modules:
	mkdir -p $(MODVERDIR)
	$(MAKE) -C $(KSRC) M=$(PWD) MODVERDIR=$(MODVERDIR) modules
