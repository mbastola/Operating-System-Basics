# Efficient Mallocs

## Overview

This folder contains multiple `malloc` implementation experiments and a driver for testing allocator correctness and throughput against trace files.

The project follows the classic malloc lab style: select one allocator implementation as `mm.c`, build the driver, then run traces through `mdriver`.

## Allocator Implementations

- `mm-original.c` - naive baseline allocator implementation.
- `mm_freeList.c` - allocator using an explicit free list.
- `mm_seggregatedLists.c` - allocator using segregated free lists.

## Files

- `mdriver.c` - allocator test driver.
- `memlib.c`, `memlib.h` - simulated heap interface.
- `fsecs.c`, `fcyc.c`, `clock.c`, `ftimer.c` - timing helpers.
- `short1-bal.rep`, `short2-bal.rep` - tiny trace files for quick tests.
- `config.h`, `mm.h` - shared driver and allocator configuration.
- `Makefile` - builds `mdriver`.

## Build

Choose an allocator implementation and copy it to `mm.c` before building:

```sh
cp mm_freeList.c mm.c
make
```

## Run

```sh
./mdriver -V -f short1-bal.rep
./mdriver -V -f short2-bal.rep
```

Use `-h` to list all driver flags:

```sh
./mdriver -h
```

## Notes

The free-list and segregated-list versions are intended to improve allocator throughput and utilization compared with the naive baseline.
