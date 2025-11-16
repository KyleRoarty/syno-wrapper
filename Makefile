KDIR ?= ~/ds918p-pve/linux-headers-6.14.11-4-pve
CC := gcc-14

ifneq ($(KERNELRELEASE),)

power_led_uart-objs = power_led_common.o uart.o

obj-m += power_led_uart.o


else

all:
	$(MAKE) -f $(KDIR)/Makefile CC=$(CC) M=$(PWD) MO=$(PWD)/build modules

clean:
	$(MAKE) -f $(KDIR)/Makefile CC=$(CC) M=$(PWD) MO=$(PWD)/build clean

endif