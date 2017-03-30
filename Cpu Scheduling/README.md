# CPU Scheduling

## Overview

This project simulates classic CPU scheduling algorithms over a CSV workload. Each input row describes a process with:

```text
pid,arrival_time,cpu_burst,priority
```

The simulator builds a run queue and prints the selected execution order, waiting time, and turnaround time for each process.

## Scheduling Policies

- `FCFS` - First Come, First Served.
- `SJF` - Shortest Job First.
- `STCF` - Shortest Time-to-Completion First, a preemptive shortest-job policy.

## Files

- `schedsim.c` - scheduler implementations, CSV parsing, and simulator driver.
- `schedsim.h` - scheduler data structures.
- `list.h` - Linux-style linked list helpers used by the run queues.
- `input_simple.csv`, `input_simple2.csv` - sample workloads.
- `Makefile` - builds the `schedsim` executable.

## Build

```sh
make
```

## Run

```sh
./schedsim -s FCFS -i input_simple.csv
./schedsim -s SJF -i input_simple.csv
./schedsim -s STCF -i input_simple.csv
```

## Notes

`SJF` and `STCF` are implemented with a dynamically ordered run queue so the simulator can model decisions that depend on arrival time and remaining burst length.
