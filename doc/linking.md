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

To use a custom linker script when linking with gcc using
`-specs=picolibc.specs`, you'll need to use the gcc `-T` option
instead of using the `-Wl,-T` linker pass through option. This causes
`picolibc.specs` to not add the picolibc linker script along with your
custom one:

	gcc -specs=picolibc.specs -Tcustom.ld

## Using picolibc.ld

Picolibc provides a default linker script which can often be used to
link applications, providing that your linking requirements are fairly
straightforward. To use picolibc.ld, you'll create a custom linker
script that sets up some variables and then INCLUDE's
picolibc.ld. Here's a sample custom linker script `sample.ld`:

	__flash = 0x08000000;
	__flash_size = 128K;
	__ram = 0x20000000;
	__ram_size = 16k;
	__stack_size = 512;

	INCLUDE picolibc.ld

This is for an STM32L151 SoC with 128kB of flash and 16kB of RAM. We
want to make sure there's space for at least 512 bytes of stack. To use
this with gcc, the command line would look like this:

	gcc -specs=picolibc.specs -Tsample.ld

Alternatively, you can specify those values using `--defsym` and use
picolibc.ld as the linker script:

	cc -Wl,--defsym=__flash=0x08000000 -Wl,--defsym=__flash_size=128K ... -Tpicolibc.ld

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
   
 * `__heap_size_min` is an optional value that you can set to ensure
   there is at least this much memory available for the heap used by
   malloc. Malloc will still be able to use all memory between the end
   of pre-allocate data and the bottom of the stack area.

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

 1. Contents located first in flash. These can be used for interrupt
    vectors or startup code.

    * `.text.init.enter`
    * `.data.init.enter`
    * `.init`, `.init.*`

 2. The bulk of the application code

    * `.text.unlikely`, `.text.unlikely.*`
    * `.text.startup`, `.text.startup.*`
    * `.text`, `.text.*`
    * `.gnu.linkonce.t.*`

 3. Cleanup routines

    * `.fini`, `.fini.*`

 4. Read-only data

    * `.rdata`
    * `.rodata`, `.rodata.*`
    * `.gnu.linkonce.r.*`
    * `.srodata.cst16`
    * `.srodata.cst8`
    * `.srodata.cst4`
    * `.srodata.cst2`
    * `.srodata`,  `.srodata.*`
    * `.data.rel.ro`, `.data.rel.ro.*`
    * `.got`, `.got.*`

 5. Addresses of pre-initialization functions. Each of the addresses
    in the list is called during program initialization, before
    `_init()`.

    * `.preinit_array`

 6. Addresses of initializer/constructor functions. Each of the
    addresses in the list is called during program initialization,
    before `main()`.

    * `.init_array`, `.ctors`

 7. Addresses of de-initializer/destructor functions. Each of the
    addresses in the list is called after the program finishes, after
    `main()`.

    * `.fini_array`, `.dtors`

#### Uninitialized ram contents

You can place items in RAM that is *not* initialized by
picolibc. These can be handy if you need values in memory to survive
reset, perhaps as a way to communicate from the application to a boot
loader or similar. These are placed first in RAM and are sorted by
name so that the order is consistent across linking operations:

 1. `.preserve.*`

 2. `.preserve`

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
 2) `.gnu.linkonce.sb.*`
 3) `.bss`, `.bss.*`
 4) `.gnu.linkonce.b.*`
 5) `COMMON`

#### Stack area

The stack is placed at the end of RAM; the `__stack_size` value in the linker
script specifies how much space to reserve for it. If there isn't
enough available RAM, linking will fail.

#### Heap area

Memory between the end of the cleared ram contents and the stack is
available for malloc. If you need to ensure that there is at least a
certain amount of heap space available, you can set the
`__heap_size_min` value in the linker script.
