# Hello Kernel Module

## Overview

This is the smallest useful kernel-level lab in the project: a loadable Linux kernel module that announces when it is inserted and removed.

It demonstrates the kernel module lifecycle through `module_init`, `module_exit`, `pr_info`, and module metadata.

## Files

- `hello_kernel.c` - module init/exit implementation.
- `Makefile` - delegates module builds to the running kernel's build tree.

## Build

```sh
make
```

## Run

```sh
sudo insmod hello_kernel.ko
dmesg | tail
sudo rmmod hello_kernel
dmesg | tail
```

## Notes

Build this against headers for the same kernel you plan to load it into. A mismatch between `uname -r` and installed headers will usually fail during `make`.
