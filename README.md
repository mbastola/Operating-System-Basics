# Operating System Basics

A hands-on collection of C systems programming labs covering core operating-system concepts from user-space tools to introductory Linux kernel modules.

## Projects

- [CPU Scheduling](Cpu%20Scheduling/README.md) - FCFS, SJF, and STCF scheduling simulation.
- [Disk Scheduling](Disk%20Scheduling/README.md) - FCFS, SSTF, SCAN, CSCAN, LOOK, and CLOOK disk request simulation.
- [MCGrep](MCGrep/README.md) - threaded grep-style search using POSIX threads and `mmap`.
- [Simple XSSHell](Simple%20XSSHell/README.md) - a small shell with scripts, variables, built-ins, and process launching.
- [Efficient Mallocs](Efficient%20Mallocs/README.md) - allocator experiments using free lists and segregated free lists.
- [Kernel Programming](Kernel%20Programming/README.md) - basic loadable Linux kernel module examples.

## Focus Areas

- Scheduling algorithms
- Process control and shell behavior
- Threading and file I/O
- Dynamic memory allocation
- Linux kernel module basics
- procfs and scheduler-visible process state

## Build

Each folder is self-contained and includes its own `README.md` and `Makefile` where applicable.

```sh
cd "Cpu Scheduling"
make
```

Kernel module examples should be built in a Linux VM or lab environment with matching kernel headers installed.
