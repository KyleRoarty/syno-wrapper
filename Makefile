# Will pick modules from your kernel if you don't
# override KDIR in tasks.json (done automatically)
KDIR ?= /lib/modules/$(uname -r)/build
CC := gcc-14

ifneq ($(KERNELRELEASE),)

ifneq ($(wildcard $(KCONFIG_CONFIG)),)
	include $(KCONFIG_CONFIG)
endif

# Needed for #ifdef checks on CONFIG_XXX
ccflags-y += -include $(src)/include/generated/autoconf.h

power_led_uart-y := power_led_common.o
power_led_uart-$(CONFIG_SYNO_WRAPPER_UART) += uart.o
power_led_uart-$(CONFIG_SYNO_WRAPPER_FANCTL) += fanctl.o

obj-m += power_led_uart.o

else

BASEDIR := $(CURDIR)
all:
	@if [ ! -f "$(BASEDIR)/.config" ]; then (echo ".config doesn't exist!" && exit 1); fi
	$(MAKE) -f $(KDIR)/Makefile CC=$(CC) M=$(PWD) MO=$(PWD)/build KCONFIG_CONFIG=$(BASEDIR)/.config modules

clean:
	$(MAKE) -f $(KDIR)/Makefile CC=$(CC) M=$(PWD) MO=$(PWD)/build clean

endif