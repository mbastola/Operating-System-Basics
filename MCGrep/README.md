# MCGrep

## Overview

`mcgrep` is a threaded grep-style utility. It memory maps an input file, splits it into temporary chunks, searches each chunk in parallel, and then combines the per-thread output.

The program can either print matching lines or replace a search character before printing.

## Features

- Search for lines containing a single character.
- Optionally replace that character in matching output.
- Split work across a configurable number of POSIX threads.
- Write combined output to stdout or to a file.

## Files

- `mcgrep.c` - command-line parsing, mmap chunking, thread work, and output wrapping.
- `filein.txt` - small sample input.
- `Makefile` - builds the `mcgrep` executable with pthread support.

## Build

```sh
make
```

## Run

```sh
./mcgrep -i filein.txt -c a
./mcgrep -i filein.txt -c a -r A -t 2
./mcgrep -i filein.txt -c a -t 2 -o output.txt
```

## Notes

The chunking step writes temporary `.tmp` and `.out` files before joining thread results. Run `make clean` to remove those generated files after experimenting.
