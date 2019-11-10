# PicoLibc
Copyright © 2018,2019 Keith Packard

PicoLibc is library offering standard C library APIs that targets
small embedded systems with limited RAM. PicoLibc was formed by blending
code from [Newlib](http://sourceware.org/newlib/) and
[AVR Libc](https://www.nongnu.org/avr-libc/).

## License

Picolibc source comes from a variety of places and has a huge variety
of copyright holders and license texts. While much of the code comes
from newlib, none of the GPL-related bits are left in the repository,
so all of the source code uses BSD-like licenses, a mixture of 2- and
3- clause BSD itself and a variety of other (mostly older) licenses
with similar terms.

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

## Documentation

 * [Building Picolibc](doc/build.md)
 * [Using Picolibc](doc/using.md)
 * [Picolibc initialization](doc/init.md)
 * [Thread Local Storage](doc/tls.md)
 * [Hello World](hello-world/hello-world.md)
