# Reproduce (Work In Progress)

Reproduce is a tool designed to make software builds reliable and consistent by removing
all randomness from the build processes and any related subprocesses.
The goal is to use Reproduce in a nix build sandbox, which operates without internet access.

## How to Use Reproduce

To use Reproduce, simply run it alongside your main build program. 
It will monitor and manage all related processes,
controlling system calls that could introduce randomness.

Here's how you can run it:

```bash
nix run git+https://github.com/Qubasa/reproduce -- <your_program> [arguments...]
```

Example:

```bash
nix run git+https://github.com/Qubasa/reproduce -- date -R
```

Output:
```
Tue, 11 Jun 2024 11:45:06 +0200
```

## Setup Development Environment

```bash
nix develop
./run.sh
```


### How It Works

Reproduce identifies and handles system calls that may cause randomness in processes, providing predictable pseudo-randomness so results remain the same every time.

### Current Status

To find out which system calls are currently managed by Reproduce, check [./src/syscalls.c](./src/syscalls.c).
**Note:** Currently, any program attempting to read from `/dev/urandom` is redirected to read from standard input.
This means you must supply the pseudo-randomness externally,
or the process will not continue. This setup is temporary and needs improvement.
