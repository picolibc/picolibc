# Building Picolibc

Picolibc is designed to be cross-compiled for embedded systems on a
Linux host using GCC. There is some support for Clang, but that
doesn't include the built-in multilib support. Picolibc uses the meson
build system, which is a slightly quirky build system designed to
replace autotools with a single language.

Picolibc requires meson version 0.50 or newer. If your operating
system provides an older version, you can get the latest using
pip. For example, on a Debian or Ubuntu system, you would do:

    $ sudo apt install pip
    $ pip install meson

On POSIX systems, meson uses the low-level 'ninja' build tool and
currently requires at least ninja version 1.5. If your operating
system doesn't provide at least this version, head over to
[ninja-build.org](https://ninja-build.org) to find out how to
download and install the latest bits.

## Selecting build options

Use -D<option-name>=<value> on the meson command line to change from
the default value. Many of these options set configuration values for
the newlib code base and should match that configuration system. The
defaults should be reasonable for small embedded systems.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| fast-strcmp                 | true    | Always optimize strcmp for performance (to make Dhrystone happy)                     |
| multilib                    | true    | Build every multilib configuration supported by the compiler                         |
| picolib                     | true    | Include picolib bits for tls and sbrk support                                        |
| thread-local-storage        | true    | Use TLS for global variables                                                         |
| tls-model                   | local-exec | Select TLS model (global-dynamic, local-dynamic, initial-exec or local-exec)      |
| newlib-global-errno         | false   | Use single global errno even when thread-local-storage=true                          |
| newlib-initfini-array       | true    | Use .init_array and .fini_array sections in picocrt                                  |
| newlib-initfini             | true    | Support _init() and _fini() functions in picocrt                                     |
| hw-fp                       | false   | Turn on hardware floating point math                                                 |
| have-alias-attribute        | true    | Compiler supports __alias__ attribute                                                |
| tests                       | false   | Enable tests                                                                         |
| newlib-tinystdio            | true    | Use tiny stdio from avr libc                                                         |
| specsdir                    | auto    | Where to install the .specs file (default is in the GCC directory)                   |
| newlib-mb                   | false   | Enable multibyte support                                                             |
| newlib-iconv-encodings      | false   | Enable specific comma-separated list of bidirectional iconv encodings to be built-in |
| newlib-iconv-from-encodings | false   | Enable specific comma-separated list of "from" iconv encodings to be built-in        |
| newlib-iconv-to-encodings   | false   | Enable specific comma-separated list of "to" iconv encodings to be built-in          |
| newlib-iconv-external-ccs   | false   | Enable capabilities to load external CCS files for iconv                             |
| newlib-register-fini        | false   | Enable finalization function registration using atexit                               |
| newlib-atexit-dynamic-alloc | false   | Enable dynamic allocation of atexit entries                                          |
| newlib-global-atexit        | false   | Enable atexit data structure as global                                               |
| newlib_wide_orient          | false   | Turn off wide orientation in streamio                                                |
| newlib-nano-malloc          | true    | Use small-footprint nano-malloc implementation                                       |
| lite-exit                   | true    | Enable light weight exit                                                             |
| newlib-retargetable-locking | false   | Allow locking routines to be retargeted at link time                                 |
| newlib-long-time_t          | false   | Define time_t to long                                                                |
| newlib-multithread          | false   | Enable support for multiple threads                                                  |
| newlib-iconv                | false   | Enable iconv library support                                                         |
| newlib-locale-info          | false   | Enable locale support                                                                |
| newlib-locale-info-extended | false   | Enable even more locale support                                                      |

### Options when using legacy stdio bits

Normally, Picolibc is built with the small stdio library adapted from
avrlibc (newlib-tinystdio=true). It still has the original newlib
stdio bits and those might still work (newlib-tinystdio=false). These
options are relevant only in that configuration

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| newlib-reent-small          | false   | Enable small reentrant struct support                                                |
| newlib-io-pos-args          | false   | Enable printf-family positional arg support                                          |
| newlib-io-c99-formats       | false   | Enable C99 support in IO functions like printf/scanf                                 |
| newlib-io-long-long         | false   | Enable long long type support in IO functions like printf/scanf                      |
| newlib-io-long-double       | false   | Enable long double type support in IO functions printf/scanf                         |
| newlib-global-stdio-streams | false   | Enable global stdio streams                                                          |
| newlib-fvwrite-in-streamio  | false   | Disable iov in streamio                                                              |
| newlib-fseek-optimization   | false   | Disable fseek optimization                                                           |
| newlib-unbuf-stream-opt     | false   | Enable unbuffered stream optimization in streamio                                    |
| newlib_nano_formatted_io    | false   | Use nano version formatted IO                                                        |
| newlib-io-float             | false   | Enable printf/scanf family float support                                             |
| newlib-supplied-syscalls    | false   | Enable newlib supplied syscalls                                                      |

## Building for embedded RISC-V and ARM systems

Meson sticks all of the cross-compilation build configuration bits in
a separate configuration file. There are a bunch of things you need to
set, which the build system really shouldn't care about. Example
configuration settings for RISC-V processors are in
cross-riscv64-unknown-elf.txt:

    [binaries]
    c = 'riscv64-unknown-elf-gcc'
    ar = 'riscv64-unknown-elf-ar'
    as = 'riscv64-unknown-elf-as'
    ld = 'riscv64-unknown-elf-ld'
    strip = 'riscv64-unknown-elf-strip'

    [host_machine]
    system = 'unknown'
    cpu_family = 'riscv'
    cpu = 'riscv'
    endian = 'little'

Settings for ARM processors are in cross-arm-none-eabi.txt:

    [binaries]
    c = 'arm-none-eabi-gcc'
    ar = 'arm-none-eabi-ar'
    as = 'arm-none-eabi-as'
    ld = 'arm-none-eabi-ld'
    strip = 'arm-none-eabi-strip'

    [host_machine]
    system = 'none'
    cpu_family = 'arm'
    cpu = 'arm'
    endian = 'little'

If those programs aren't in your path, you can edit the file to point
wherever they may be.

### Auto-detecting the compiler configurations

The PicoLibc configuration detects the processor configurations
supported by the compiler using the `--print-multi-lib` command-line option:

    $ riscv64-unknown-elf-gcc --print-multi-lib
    .;
    rv32e/ilp32e;@march=rv32e@mabi=ilp32e
    rv32em/ilp32e;@march=rv32em@mabi=ilp32e
    rv32eac/ilp32e;@march=rv32eac@mabi=ilp32e
    rv32emac/ilp32e;@march=rv32emac@mabi=ilp32e
    rv32i/ilp32;@march=rv32i@mabi=ilp32
    rv32im/ilp32;@march=rv32im@mabi=ilp32
    rv32imf/ilp32f;@march=rv32imf@mabi=ilp32f
    rv32iac/ilp32;@march=rv32iac@mabi=ilp32
    rv32imac/ilp32;@march=rv32imac@mabi=ilp32
    rv32imafc/ilp32f;@march=rv32imafc@mabi=ilp32f
    rv32imafdc/ilp32d;@march=rv32imafdc@mabi=ilp32d
    rv64i/lp64;@march=rv64i@mabi=lp64
    rv64im/lp64;@march=rv64im@mabi=lp64
    rv64imf/lp64f;@march=rv64imf@mabi=lp64f
    rv64iac/lp64;@march=rv64iac@mabi=lp64
    rv64imac/lp64;@march=rv64imac@mabi=lp64
    rv64imafc/lp64f;@march=rv64imafc@mabi=lp64f
    rv64imafdc/lp64d;@march=rv64imafdc@mabi=lp64d


    $ arm-none-eabi-gcc --print-multi-lib
    .;
    thumb;@mthumb
    hard;@mfloat-abi=hard
    thumb/v6-m;@mthumb@march=armv6s-m
    thumb/v7-m;@mthumb@march=armv7-m
    thumb/v7e-m;@mthumb@march=armv7e-m
    thumb/v7-ar;@mthumb@march=armv7
    thumb/v8-m.base;@mthumb@march=armv8-m.base
    thumb/v8-m.main;@mthumb@march=armv8-m.main
    thumb/v7e-m/fpv4-sp/softfp;@mthumb@march=armv7e-m@mfpu=fpv4-sp-d16@mfloat-abi=softfp
    thumb/v7e-m/fpv4-sp/hard;@mthumb@march=armv7e-m@mfpu=fpv4-sp-d16@mfloat-abi=hard
    thumb/v7e-m/fpv5/softfp;@mthumb@march=armv7e-m@mfpu=fpv5-d16@mfloat-abi=softfp
    thumb/v7e-m/fpv5/hard;@mthumb@march=armv7e-m@mfpu=fpv5-d16@mfloat-abi=hard
    thumb/v7-ar/fpv3/softfp;@mthumb@march=armv7@mfpu=vfpv3-d16@mfloat-abi=softfp
    thumb/v7-ar/fpv3/hard;@mthumb@march=armv7@mfpu=vfpv3-d16@mfloat-abi=hard
    thumb/v7-ar/fpv3/hard/be;@mthumb@march=armv7@mfpu=vfpv3-d16@mfloat-abi=hard@mbig-endian
    thumb/v8-m.main/fpv5-sp/softfp;@mthumb@march=armv8-m.main@mfpu=fpv5-sp-d16@mfloat-abi=softfp
    thumb/v8-m.main/fpv5-sp/hard;@mthumb@march=armv8-m.main@mfpu=fpv5-sp-d16@mfloat-abi=hard
    thumb/v8-m.main/fpv5/softfp;@mthumb@march=armv8-m.main@mfpu=fpv5-d16@mfloat-abi=softfp
    thumb/v8-m.main/fpv5/hard;@mthumb@march=armv8-m.main@mfpu=fpv5-d16@mfloat-abi=hard

On RISC-V, PicoLibc is compiled 19 times, while on ARM, the library is
compiled 20 times with the specified compiler options (replace the
'@'s with '-' to see what they will be).

### Running meson

Because Picolibc targets smaller systems like the SiFive FE310 or ARM
Cortex-M0 parts with only a few kB of RAM and flash, the default
values for all of the configuration options are designed to minimize
the library code size. Here's the
[do-riscv-configure](../do-riscv-configure) script from the repository
that configures the library for small RISC-V systems:

    #!/bin/sh
    ARCH=riscv64-unknown-elf
    DIR=`dirname $0`
    meson "$DIR" \
	    -Dincludedir=picolibc/$ARCH/include \
	    -Dlibdir=picolibc/$ARCH/lib \
	    --cross-file "$DIR"/cross-$ARCH.txt \
	    "$@"

This script is designed to be run from a build directory, so you'd do:

    $ mkdir build-riscv64-unknown-elf
    $ cd build-riscv64-unknown-elf
    $ ../do-riscv-configure

### Compiling

Once configured, you can compile the libraries with

    $ ninja
    ...
    $ ninja install
    ...
    $

