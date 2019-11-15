# Linking Picolibc applications

Linking embedded applications requires significant target-specific
information, including the location of various memory sections along with
a range of application-specific memory settings.

You can create your own custom linker script, or you can use the
linker script provided by Picolibc. This document describes how to do
either.

## Creating a Custom Linker Script

Aside from the application and hardware specific aspects of creating a
linker script, if your application is using the Picolib startup code,
then you need to define the addresses used in that code, and set up
the data as required. Checkout the [Initializers in Picolibc](init.md) document
for details on what names to declare.

## Using picolibc.ld

Picolibc provides a default linker script which can often be used to
link applications, providing that your linking requirements are fairly
straightforward. To use picolibc.ld, you'll create a custom linker
script that sets up some variables and then INCLUDE's
picolibc.ld. Here's a sample custom linker script:

	__flash = 0x08000000;
	__flash_size = 128K;
	__ram = 0x20000000;
	__ram_size = 16k;
	__stack_size = 512;

	INCLUDE picolibc.ld

This is for an STM32L151 SoC with 128kB of flash and 16kB of RAM. We
want to make sure there's space for at least 512 bytes of stack.

### Defining Memory Regions

Picolibc.ld defines only two memory regions: `flash` and `ram`. Flash
is an addressable region of read-only memory which holds program text,
constant data and initializers for read-write data. Ram is read-write
memory which needs to be initialized before your application starts.

As shown above, you declare the base and size of both memory regions
in your linker script:

 * `__flash` specifies the lowest address in read-only memory used by
   your application. This needs to be in flash, but need not be the
   start of actual flash in the device.

 * `__flash_size` specifies the amount of read-only memory you want to
   allow the application to fill. This need not be all of the
   available memory.

 * `__ram` specifies the lowest address you want the linker to
   allocate to read-write data for the application.

 * `__ram_size` specifies the maximum amount of read-write memory you
   want to permit the application to use.

 * `__stack_size` reserves this much space at the top of ram for the
   initial stack.
   
### Arranging Code and Data in Memory

Where bits of code and data land in memory can be controlled to some
degree by placing variables and functions in various sections by
decorating them with `__attribute__ ((section(`*name*`)))`. You'll
find '*' used in the following defintions; that can be replaced with
any string. For instance, when you use -ffunction-sections or
-fdata-sections with gcc, that creates a section named
`.text.`*function-name* for each function and `.data.`*variable-name*
for each variable. Here are all of the section names used in
picolibc.ld:

#### Flash contents

These are stored in flash and used directly from flash.

 1) `.text.init.enter`
 2) `.data.init.enter`
 3) `.init`, `.init.*`. Contents located
   first in flash. These can be used for interrupt vectors or startup
   code.

 4) `.text.unlikely`, `.text.unlikely.*`
 5) `.text.startup`, `.text.startup.*`
 6) `.text`, `.text.*`
 7) `.gnu.linkonce.t.*`. Usually the bulk of the
    application code

 8) `.fini`, `.fini.*`. Usually cleanup routines.

 9) `.rdata`
 10) `.rodata`, `.rodata.*`
 11) `.gnu.linkonce.r.*`
 12) `.srodata.cst16`
 13) `.srodata.cst8`
 14) `.srodata.cst4`
 15) `.srodata.cst2`
 16) `.srodata .srodata.*`. These are generally used for read-only
     data

 17) `.preinit_array`. This secton contain addresses of
     pre-initialization functions. Each of the addresses in the list
     is called during program initialization, before `_init()`.

 18) `.init_array`, `.ctors`. These segments contain addresses of
     initializer/constructor functions. Each of the addresses in the
     list is called during program initialization, before `main()`.

 19) `.fini_array`, `.dtors`. These segments contain addresses of
     de-iniitializer/destructor functions. Each of the addresses in
     the list is called after the program finishes, after `main()`.

#### Initialized ram contents

picolibc.ld places values for variables with explicit initializers in
flash and marks the location in flash and in RAM. At application
startup, picocrt uses those recorded addresses to copy data from flash
to RAM. As a result, any initialized data takes twice as much memory;
the initialization values stored in flash and the runtime values
stored in ram. Making values read-only where possible saves the RAM.

 1) `.data`, `.data.*`

 2) `.gnu.linkonce.d.*`

 3) `.sdata`, `.sdata.*`, `.sdata2.*`

 4) `.gnu.linkonce.s.*`
 5) `.srodata.cst16`
 6) `.srodata.cst8`
 7) `.srodata.cst4`
 8) `.srodata.cst2`
 
Picolibc uses native toolchain TLS support for values which should be
per-thread. This means that variables like `errno` will be referenced
using TLS mechanisms. To make these work even when the application
doesn't support threading, Picolibc allocates a static copy of the TLS
data in RAM. Picocrt initializes the architecture TLS mechanism to
reference this static copy.

By arranging the static copy of initialized and zero'd TLS data right
at the data/bss boundary, picolibc can initialize the TLS block as a
part of initializing RAM with no additional code. This requires a bit
of a trick as the linker doesn't allocate any memory for TLS bss
segments; picolibc.ld makes space by simply advancing the memory
location by the size of the TLS bss segment.

 1) `.tdata`, `.tdata.*`, `.gnu.linkonce.td.*`

#### Cleared ram contents

Variables without any explicit initializers are set to zero by picocrt
at startup time. The first chunk of these is part of the TLS block:

 1) `.tbss`, `.tbss.*`, `.gnu.linkonce.tb.*`
 2) `.tcommon`

After the TLS bss section comes the regular BSS variables:

 1) `.sbss*`
 2) `.gnu.linkonce.sb.*
 3) `.bss`, `.bss.*`
 4) `.gnu.linkonce.b.*`
 5) `COMMON`
