KDIR ?= ~/ds918p-pve/linux-headers-6.14.11-4-pve
CC := gcc-14

ifneq ($(KERNELRELEASE),)

obj-m += hello-1.o

else

all:
	$(MAKE) -f $(KDIR)/Makefile CC=$(CC) M=$(PWD) MO=$(PWD)/build modules

clean:
	$(MAKE) -f $(KDIR)/Makefile CC=$(CC) M=$(PWD) MO=$(PWD)/build clean

endif