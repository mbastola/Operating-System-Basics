# Simple XSSHell

## Overview

`xssh` is a small shell implementation that supports interactive commands, script execution, simple variables, environment export helpers, and foreground/background process launching.

It can run in interactive mode or consume commands from a file with optional positional variables.

## Features

- Interactive prompt when started without `-f`.
- Script mode with `-f INFILE [$1] [$2]`.
- Built-ins for `show`, `set`, `unset`, `export`, `unexport`, `chdir`, `wait`, and `exit`.
- External command execution through `fork` and `execvp`.
- Basic background execution with `&`.
- Debug/display mode with `-x`.

## Files

- `xssh.c` - shell loop, built-ins, process launching, and signal setup.
- `xssh.h` - shared constants and helper declarations.
- `uthash.h` - hash table support for shell variables.
- `commands.txt`, `cmd.txt` - sample script inputs.
- `Makefile` - builds the `xssh` executable.

## Build

```sh
make xssh
```

## Run

```sh
./xssh
./xssh -f commands.txt hello world
./xssh -x -f cmd.txt
```

## Current Limitations

- Output redirection with `>` is stubbed out.
- Input redirection with `<` is partially implemented.
- Some bad external command paths can leave an extra child shell path, so exiting may require a second `exit`.
- Command and input buffers use fixed maximum sizes to keep the parsing logic simple.
