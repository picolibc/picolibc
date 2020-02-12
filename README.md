# PicoLibc
![CI](https://github.com/TheDennenning/picolibc/workflows/CI/badge.svg)
Copyright © 2018,2019 Keith Packard

PicoLibc is library offering standard C library APIs that targets
small embedded systems with limited RAM. PicoLibc was formed by blending
code from [Newlib](http://sourceware.org/newlib/) and
[AVR Libc](https://www.nongnu.org/avr-libc/).

garbage

## License

Picolibc source comes from a variety of places and has a huge variety
of copyright holders and license texts. While much of the code comes
from newlib, none of the GPL-related bits used to build the library
are left in the repository, so all of the source code uses BSD-like
licenses, a mixture of 2- and 3- clause BSD itself and a variety of
other (mostly older) licenses with similar terms.

There are two files used for testing printf, test/printf-tests.c and
test/testcases.c which are licensed under the GPL version 2 or
later.

Please see the file COPYING.NEWLIB in this distribution for newlib
license terms.

## Supported Architectures

Picolibc inherited code for a *lot* of architectures from newlib, but
at this point only has code to build for the following targets:

 * ARM (32-bit only)
 * i386 (Linux hosted, for testing)
 * RISC-V (both 32- and 64- bit)
 * x86_64 (Linux hosted, for testing)
 * PowerPC
 * ESP8266 (xtensa-lx106-elf)

Supporting architectures that already have newlib code requires:

 1. newlib/libc/machine/_architecture_/meson.build to build any
    architecture-specific libc bits
 2. newlib/libm/machine/_architecture_/meson.build to build any
    architecture-specific libm bits
 3. picocrt/machine/_architecture_ source code and build bits if
    you need custom startup code for the architecture.
 4. cross-_gcc-triple_.txt to configure the meson cross-compilation
    mechanism to use the right tools
 5. do-_architecture_-configure to make testing the cross-compilation
    setup easier.

## Relation to newlib

Picolibc is mostly built from pieces of newlib, and retains the
directory structure of that project. While there have been a lot of
changes in the build system and per-thread data storage, the bulk of
the source code remains unchanged.

To keep picolibc and newlib code in sync, newlib changes will be
regularly incorporated. To ease integration of these changes into
picolibc, some care needs to be taken while editing the code:

 * Files should not be renamed.
 * Fixes that also benefit users of newlib should also be sent to the
   newlib project
 * Changes, where possible, should be made in a way compatible with
   newlib design. For example, instead of using 'errno' (which is
   valid in picolibc), use __errno_r(r), even when 'r' is not defined
   in the local context.

The bulk of newlib changes over the last several years have been in
areas unrelated to the code used by picolibc, so keeping things in
sync has not been difficult so far.

## Releases

### Picolibc version 1.3

This release now includes tests, and fixes bugs found by them.

 1. ESP8266 support added, thanks to Jonathan McDowell.

 2. Numerous test cases from newlib have been fixed, and
    precision requirements adjusted so that the library now
    passes its own test suite on x86, RISC-V and ARM.

 3. String/number conversion bug fixes. This includes fcvt/ecvt/gcvt
    shared with newlib and tinystdio printf/scanf

 4. A few RISC-V ABI fixes, including setting the TLS base correctly,
    compiling with -mcmodel=medany, and enabling the FPU for libraries
    built to use it.

 5. Semihosting updates, including adding unlink, kill and getpid
    (which are used by some tests).

### Picolibc version 1.2

This release includes important fixes in picolibc.ld and more
semihosting support.

 1. File I/O and clock support for semihosting. This enables fopen/fdopen
    support in tinystdio along with an API to fetch a real time clock
    value.

 2. Fix picolibc.ld to not attempt to use redefined symbols for memory
    space definitions. These redefinitions would fail and the default
    values be used for system memory definitions. Instead, just use
    the ? : operators each place the values are needed. Linker scripts
    continue to mystify.

 3. Expose library definitions in 'picolibc.h', instead of 'newlib.h'
    and '_newlib_version.h'

 4. Define HAVE_SEMIHOST when semihosting support is available. This
    lets the 'hello-world' example do some semihost specific things.

### Picolibc version 1.1

A minor update from 1.0, this release includes:

 1. semihost support. This adds console I/O and exit(3) support on ARM
    and RISC-V hosts using the standard semihosting interfaces.

 2. Posix I/O support in tinystdio. When -Dposix-io=true is included
    in the meson command line (which is the default), tinystdio adds
    support for fopen and fdopen by using malloc, open, close, read,
    write and lseek. If -Dposix-console=true is also passed to meson,
    then picolibc will direct stdin/stdout/stderr to the posix
    standard file descriptors (0, 1, 2).

 3. Merge recent upstream newlib code. This brings picolibc up to date
    with current newlib sources.

 4. Hello world example. This uses a simple Makefile to demonstrate
    how to us picolibc when installed for ARM and RISC-V embedded
    processors. The resulting executables can be run under qemu.

 5. Remove newlib/libm/mathfp directory. This experimental code never
    worked correctly anyways.

### Picolibc version 1.0

This is the first release of picolibc. Major changes from newlib
include:

 1. Remove all non-BSD licensed code. None of it was used in building
    the embedded library, and removing it greatly simplifies the
    license situation.

 2. Move thread-local values to native TLS mechanism

 3. Add smaller stdio from avr-libc, which is enabled by default

 4. Switch build system to meson. This has two notable benefits; the first
    is that building the library is much faster, the second is that
    it isolates build system changes from newlib making merging of
    newlib changes much easier.

 5. Add simple startup code. This can be used in environments that
    don't have complicated requirements, allowing small applications
    to avoid needing to figure this out.

## Documentation

 * [Building Picolibc](doc/build.md)
 * [Operating System Support](doc/os.md)
 * [Using Picolibc](doc/using.md)
 * [Picolibc initialization](doc/init.md)
 * [Thread Local Storage](doc/tls.md)
 * [Linking with Picolibc.ld](doc/linking.md)
 * [Hello World](hello-world/README.md)
 * [Picolibc as embedded source](doc/embedsource.md)
