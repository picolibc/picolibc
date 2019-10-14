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

## Supported Architectures

Picolibc inherited code for a *lot* of architectures from newlib, but
at this point only has code to build for the following targets:

 * ARM (32-bit only)
 * i386 (Linux hosted, for testing)
 * RISC-V (both 32- and 64- bit)
 * x86_64 (Linux hosted, for testing)

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

## Documentation

 * [Building Picolibc](doc/build.md)
 * [Using Picolibc](doc/using.md)
 * [Picolibc initialization](doc/init.md)
 * [Thread Local Storage](doc/tls.md)
 * [Picolibc as embedded source](doc/embedsource.md)
