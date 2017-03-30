# Disk Scheduling

## Overview

This project compares disk arm scheduling algorithms over a simple cylinder request trace. The first line of the input sets the starting cylinder, and each following line is a requested cylinder.

```text
START:53
98
183
37
```

The simulator writes each algorithm's total seek distance and service order to stdout or to an output file.

## Scheduling Policies

- `FCFS` - First Come, First Served.
- `SSTF` - Shortest Seek Time First.
- `SCAN` - elevator scheduling across the full disk range.
- `CSCAN` - circular SCAN.
- `LOOK` - SCAN with early stopping at the last pending request.
- `CLOOK` - circular LOOK with early stopping.

## Files

- `disksim.c` - disk scheduling implementations and simulator driver.
- `disksim.h` - request and queue data structures.
- `list.h` - Linux-style linked list helpers used by the queues.
- `input_simple.csv` - sample cylinder trace.
- `result.csv`, `out.txt` - sample/generated output artifacts.
- `Makefile` - builds the `disksim` executable.

## Build

```sh
make
```

## Run

```sh
./disksim -i input_simple.csv
./disksim -i input_simple.csv -o result.csv
```

## Notes

`SCAN` and `CSCAN` include the disk boundary cylinders, while `LOOK` and `CLOOK` stop at the furthest pending request in the active direction.
