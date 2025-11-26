# Will pick modules from your kernel if you don't
# override KDIR in tasks.json (done automatically)
KDIR ?= /lib/modules/$(uname -r)/build
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