# Kernel-Level Programming

## Overview

This folder adds small Linux kernel programming exercises to the operating-system basics collection. Each subfolder is a standalone loadable kernel module with its own `Makefile`, source file, and README.

These examples are meant for a Linux development VM or lab machine where kernel headers are installed.

## Labs

- `Hello-Kernel-Module` - a minimal loadable module that logs load/unload events.
- `Procfs-Process-Snapshot` - a module that creates a `/proc` entry and reports basic scheduler/process state.

## Prerequisites

```sh
sudo apt install build-essential linux-headers-$(uname -r)
```

Package names may vary by distribution. The important requirement is that `/lib/modules/$(uname -r)/build` points to matching kernel headers.

## Safety Notes

Kernel modules run with kernel privileges. Build and load these inside a VM or disposable lab environment, review source before loading, and unload modules when finished.
