# FrOS


IMPORTANT:  
* You need to add code from kernel_paging.c to kernel.c  
* Edit **kernel.c** only
* Editing makefile won't work!

## Description
Base on https://github.com/FRosner/FrOS/tree/minimal-c-kernel

Based on 
Carlos Fenollosas [OS tutorial](https://github.com/cfenollosa/os-tutorial), which is based on [Writing a Simple Operating System — from Scratch](https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf).

## Setup

### Install Assembler and Emulator

```bash
brew install qemu nasm
```

### Install Kernel Compiler

https://github.com/lordmilko/i686-elf-tools#mac-os-x

```
brew install i386-elf-binutils
brew install i386-elf-gcc
brew install i386-elf-gdb
```

```
export PATH="/usr/local/Cellar/x86_64-elf-binutils/<version>/bin/:/usr/local/Cellar/x86_64-elf-gcc/<version>/bin/:$PATH"
```

## Usage

- `make run`
