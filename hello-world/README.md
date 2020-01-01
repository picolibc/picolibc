# Hello World

Building embedded applications is tricky in part because of the huge
number of configuration settings necessary to get something that
works. This example shows how to use the installed version of
picolibc, and uses 'make' instead of 'meson' to try and make the
operations as clear as possible.

## Selecting picolibc headers and C library

Picolibc provides a GCC '.specs' file _(generated from
picolibc.specs.in)_ which sets the search path for
header files and picolibc libraries.

	gcc --specs=picolibc.specs

## Semihosting

Our example program wants to display a string to stdout; because there
aren't drivers for the serial ports emulated by qemu provided, the
example uses Picolibc's semihosting support

	gcc --specs=picolibc.specs --oslib=semihost

## Target processor

For RISC-V, QEMU offers the same emulation as the "spike" simulator,
which looks like a SiFive E31 chip (sifive-e31). That's a 32-bit
processor with the 'imac' options (integer, multiply, atomics,
compressed) and uses the 'ilp32' ABI (32-bit integer, long and
pointer):

	riscv64-unknown-elf-gcc --specs=picolibc.specs --oslib-semihost -march=rv32imac -mabi=ilp32

For ARM, QEMU emulates a "mps2-an385" board which has a Cortex-M3
processor:

	arm-none-eabi-gcc --specs=picolibc.specs --oslib=semihost -mcpu=cortex-m3

## Target Memory Layout

The application needs to be linked at addresses which correspond to
where the target memories are addressed. The default linker script
provided with picolibc, `picolibc.ld`, assumes that the target device
will have two kinds of memory, one for code and read-only data and
another kind for read-write data. However, the linker script has no
idea where those memories are placed in the address space. The example
specifies those by setting a few values before including
`picolibc.ld`.

For 'spike', you can have as much memory as you like, but execution
starts at 0x80000000 so the first instruction in the application needs
to land there. Picolibc on RISC-V puts _start at the first location in
read-only memory, so we set things up like this (this is
hello-world-riscv.ld):

	__flash = 0x80000000;
	__flash_size = 0x00080000;
	__ram = 0x80080000;
	__ram_size = 0x40000;
	__stack_size = 1k;

	INCLUDE picolibc.ld

The mps2-an385 has at least 16kB of flash starting at 0. Picolibc
places a small interrupt vector there which points at the first
instruction of _start.  The mps2-an385 also has 64kB of RAM starting
at 0x20000000, so hello-world-arm.ld looks like this:

	__flash =      0x00000000;
	__flash_size = 0x00004000;
	__ram =        0x20000000;
	__ram_size   = 0x00010000;
	__stack_size = 1k;

	INCLUDE picolibc.ld

The `-T` flag is used to specify the linker script in the compile
line:

	riscv64-unknown-elf-gcc --specs=picolibc.specs --oslib=semihost -march=rv32imac -mabi=ilp32 -Thello-world-riscv.ld

	arm-none-eabi-gcc --specs=picolibc.specs --oslib=semihost -mcpu=cortex-m3 -Thello-world-arm.ld

## Final Commands

The rest of the command line tells GCC what file to compile
(hello-world.c) and where to put the output (hello-world-riscv.elf and
hello-world-arm.elf):

	riscv64-unknown-elf-gcc --specs=picolibc.specs --oslib=semihost
	-march=rv32imac -mabi=ilp32 -Thello-world-riscv.ld -o
	hello-world-riscv.elf hello-world.c

	arm-none-eabi-gcc --specs=picolibc.specs --oslib=semihost
	-mcpu=cortex-m3 -Thello-world-arm.ld -o hello-world-arm.elf
	hello-world.c
