# Reproduce (WIP)
A tool that makes builds reproducible 
by removing sources of randomness from processes and all its children.

The idea is to use this tool inside the 
nix build sandbox wich doesn't have internet access. 

## How To Use Reproduce
Just execute reproduce on your top level build program and it will track
all the children and hook their syscalls.
```bash
./reproduce <program> [args...]
```


### How It Works
Reproduce will hook syscalls that could introduce randomness 
into the process and return pseudo randomness 
to make things reproducible.

Currently



