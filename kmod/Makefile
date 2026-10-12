# Out-of-tree debug module. Build against the K1 kernel, e.g.:
#   make KDIR=/path/to/kernel/spacemit/common ARCH=riscv \
#        CROSS_COMPILE=riscv64-linux-gnu-
KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
