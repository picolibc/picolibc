# Using Picolibc in Embedded Systems

Picolibc is designed to be used in deeply embedded systems, those
running either no operating system or a small RTOS. It is designed to
be linked statically along with any operating system and application
code.

## Compiling with Picolibc

To compile source code to use Picolibc, you can use the GCC .specs
file delivered with Picolibc. This will set the system header file
path and the linker library path to point at Picolibc. For example, to
compile a single file and generate a .o file:

	$ gcc --specs=picolibc.specs -c foo.c

When installed directly into the system, picolibc.specs is placed in
the gcc directory so that it will be found using just the base name of
the file. If installed in a separate location, you will need to
provide an absolute pathname to the picolibc.specs file:

	$ gcc --specs=/usr/local/picolibc/picolibc.specs -c foo.c

When building for an embedded system, you'll probably need to use a
longer name for the compiler, something like `riscv-unknown-elf-gcc`
or `arm-none-eabi-gcc`.

## Picolibc startup

Initializing an embedded system usually requires a combination of
hardware, run-time, library and application setup. You may want to
perform all of these in your own code, or you be able to use the
Picolibc startup code. Either works fine, using the Picolibc code
means writing less of your own.

### Picocrt — Picolibc startup code

Picocrt is the crt0.o code provided by Picolibc. This is enabled by
default when using -specs=picolibc.specs:

	$ gcc -specs=picolibc.specs -o foo.elf foo.c

Picocrt goes through a sequence of initialization steps, many of which
you can plug your own code into:

 1) Architecture-specific runtime initialization. For instance, RISC-V
    needs the `gp` register initialized for quick access to global
    variables while ARM with hardware floating point needs to have the
    FPSCR register set up to match C semantics for rounding.
    
 2) Data initialization. Here's the code inside Picocrt:\
    \
    `memcpy(__data_start, __data_source, (uintptr_t) __data_size);`\
    \
    For this to work, the linker script must assign correct values to
    each of these symbols:

    * __data_start points to the RAM location where the .data segment
      starts.
    * __data_source points to the Flash location where the .data segment
      initialization values are stored.
    * __data_size is an absolute symbol noting the size of the
      initialized data segment

 3) BSS initialization. Here's the code inside Picocrt:\
    \
    `memset(__bss_start, '\0', (uintptr_t) __bss_size);`\
    \
    Assign these symbols in the linker script as follows:

    * __bss_start points to the RAM location where the .bss segment
      starts.
    * __bss_size is an absolute symbol noting the size of the cleared
      data segment

 4) Optionally call constructors:

    * The default and hosted crt0 variants call
      [initializers/constructors](init.md) using `__libc_init_array()`

    * The minimal crt0 variant doesn't call any constructors

 5) Call `main()`

 6) After main returns:

    * The default and minimal crt0 variants run an infinite
      loop if main returns.

    * The hosted crt0 variant calls `exit`, passing it the value
      returned from main.

## Linking

Picolibc provides two sample linker scripts: `picolibc.ld` for C
applications and `picolibcpp.ld` for C++ applications. These are
designed to be useful for fairly simple applications running on
embedded hardware which need read-only code and data stored in flash
and read-write data allocated in RAM and initialized by picolibc at
boot time. You can read more about this on the [linking](linking.md)
page.

## Semihosting

“Semihosting” is a mechanism used when the application is run under a
debugger or emulation environment that provides access to some
functionality of the OS running the debugger or emulator. Picolibc
has semihosting support for console I/O and the exit() call.

One common use for semihosting is to generate debug messages before
any of the target hardware has been set up.

Picolibc distributes the semihosting implementation as a separate
library, `libsemihost.a`. Because it provides interfaces that are used
by libc itself, it must be included in the linker command line *after*
libc. You can do this by using the GCC --oslib=semihost
command line flag defined by picolibc.specs:

	$ gcc --specs=picolibc.specs --oslib=semihost -o program.elf program.o

You can also list libc and libsemihost in the correct order
explicitly:

	$ gcc --specs=picolibc.specs -o program.elf program.o -lc -lsemihost

## Crt0 variants

The default `crt0` version provided by Picolibc calls any constructors
before it calls main and then goes into an infinite loop after main
returns to avoid requiring an `_exit` function.

In an environment which provides a useful `_exit` implementation, applications
may want to use the `crt0-hosted` variant that calls `exit` when main
returns, resulting in a clean return to the hosting environment (this
conforms to a hosted execution environment as per the C
specification). Select this using the `--crt0=hosted` flag:

	$ gcc --specs=picolibc.specs --crt0=hosted -o program.elf program.o

In a smaller environment which is not using any constructors (or any
Picolibc code which requires constructors) may want to use the
`crt0-minimal` variant that removes the call to
`__libc_init_array()`. This variant also runs an infinite loop in case
main returns. Select this using the `--crt0=minimal` flag:

	$ gcc --specs=picolibc.specs --crt0=minimal -o program.elf program.o
