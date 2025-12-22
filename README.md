# Synology Wrapper

Implements a bunch of things that are implemented in the custom Synology kernel so that you can run a different OS on a Synology box

## Requirements

- `kconfig-frontends` for making a `.config` as this is out-of-tree

## Features

Note: Things have only been tested on the DS918+

- Creates a file (`/dev/syno_wrapper`) that you can write UART commands to from userspace
- Has a fan controller that can be compiled in/out
- Can define fan curves in the code and use them. Can't set from userspace yet
- Has a backplane controller that can be compiled in/out
- Controls the backplane slot power, turning it on when a drive is in and off when no drive is in

## Installing on Synology Box

```
sudo install -D -m 0644 syno_wrapper.ko /lib/modules/$(uname -r)/custom/syno_wrapper.ko
sudo depmod -a
sudo update-initramfs -u -k $(uname -r)
```