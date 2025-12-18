# Synology Wrapper

Implements a bunch of things that are implemented in the custom Synology kernel so that you can run a different OS on a Synology box

## Requirements

- `kconfig-frontends` for making a `.config` as this is out-of-tree

## Features

Note: Things have only been tested on the DS918+

- Creates a file (`/dev/syno_wrapper`) that you can write UART commands to from userspace
- Has a fan controller that can be compiled in/out
- Can define fan curves in the code and use them. Can't set from userspace yet