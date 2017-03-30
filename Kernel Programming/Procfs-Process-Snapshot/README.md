# Procfs Process Snapshot

## Overview

This kernel module creates a read-only `/proc/os_basics_proc_snapshot` entry. Reading it reports the current reader process, kernel `jiffies`, and a small snapshot of tasks known to the scheduler.

The lab demonstrates procfs integration, `seq_file`, module parameters, and safe process-list traversal.

## Files

- `proc_snapshot.c` - procfs module implementation.
- `Makefile` - delegates module builds to the running kernel's build tree.

## Build

```sh
make
```

## Run

```sh
sudo insmod proc_snapshot.ko
cat /proc/os_basics_proc_snapshot
sudo rmmod proc_snapshot
```

The number of tasks printed can be changed at load time:

```sh
sudo insmod proc_snapshot.ko max_tasks=20
```

## Notes

This module only reads scheduler-visible task metadata. It does not modify process state or register any writable procfs hooks.
