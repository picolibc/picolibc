# PicoLibc
Copyright © 2018,2019 Keith Packard

PicoLibc is library offering standard C library APIs that targets
small embedded systems with limited RAM. PicoLibc was formed by blending
code from [Newlib](http://sourceware.org/newlib/) and
[AVR Libc](https://www.nongnu.org/avr-libc/).

### Selecting build options

Most of these options set configuration values for the newlib code
base and should match that configuration system. Use
-D<option-name>={true,false} to change from the default value.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| target-optspace             | false   | Compile with -Os                                                                     |
| hw-fp                       | false   | Turn on hardware floating point math                                                 |
| tests                       | false   | Enable tests                                                                         |
| newlib-tinystdio            | false   | Use tiny stdio from avr libc                                                         |
| newlib-io-pos-args          | false   | Enable printf-family positional arg support                                          |
| newlib-io-c99-formats       | false   | Enable C99 support in IO functions like printf/scanf                                 |
| newlib-register-fini        | false   | Enable finalization function registration using atexit                               |
| newlib-io-long-long         | false   | Enable long long type support in IO functions like printf/scanf                      |
| newlib-io-long-double       | false   | Enable long double type support in IO functions printf/scanf                         |
| newlib-mb                   | false   | Enable multibyte support                                                             |
| newlib-iconv-encodings      | false   | Enable specific comma-separated list of bidirectional iconv encodings to be built-in |
| newlib-iconv-from-encodings | false   | Enable specific comma-separated list of "from" iconv encodings to be built-in        |
| newlib-iconv-to-encodings   | false   | Enable specific comma-separated list of "to" iconv encodings to be built-in          |
| newlib-iconv-external-ccs   | false   | Enable capabilities to load external CCS files for iconv                             |
| newlib-atexit-dynamic-alloc | false   | Enable dynamic allocation of atexit entries                                          |
| newlib-global-atexit        | false   | Enable atexit data structure as global                                               |
| newlib-reent-small          | false   | Enable small reentrant struct support                                                |
| newlib-global-stdio-streams | false   | Enable global stdio streams                                                          |
| newlib-fvwrite-in-streamio  | false   | Disable iov in streamio                                                              |
| newlib-fseek-optimization   | false   | Disable fseek optimization                                                           |
| newlib_wide_orient          | false   | Turn off wide orientation in streamio                                                |
| newlib-nano-malloc          | false   | Use small-footprint nano-malloc implementation                                       |
| newlib-unbuf-stream-opt     | false   | Enable unbuffered stream optimization in streamio                                    |
| lite-exit                   | false   | Enable light weight exit                                                             |
| newlib_nano_formatted_io    | false   | Use nano version formatted IO                                                        |
| newlib-retargetable-locking | false   | Allow locking routines to be retargeted at link time                                 |
| newlib-long-time_t          | false   | Define time_t to long                                                                |
| newlib-multithread          | false   | Enable support for multiple threads                                                  |
| newlib-iconv                | false   | Enable iconv library support                                                         |
| newlib-io-float             | false   | Enable printf/scanf family float support                                             |
| newlib-supplied-syscalls    | false   | Enable newlib supplied syscalls                                                      |

## Building for embedded RISC-V and ARM systems

Meson sticks all of the cross-compilation build configuration bits in
a separate configuration file. There are a bunch of things you need to
set, which the build system really shouldn't care about. Example
configuration settings for RISC-V processors are in
cross-riscv32-unknown-elf.txt:

    [binaries]
    c = 'riscv32-unknown-elf-gcc'
    ar = 'riscv32-unknown-elf-ar'
    as = 'riscv32-unknown-elf-as'

    [host_machine]
    system = ''
    cpu_family = ''
    cpu = ''
    endian = ''

Settings for ARM processors are in cross-arm-none-eabi.txt:

    [binaries]
    c = 'arm-none-eabi-gcc'
    ar = 'arm-none-eabi-ar'
    as = 'arm-none-eabi-as'

    [host_machine]
    system = ''
    cpu_family = ''
    cpu = ''
    endian = ''

If those programs aren't in your path, you can edit the file to point
wherever they may be.

### Auto-detecting the compiler configurations

The PicoLibc configuration detects the processor configurations
supported by the compiler using the `--print-multi-lib` command-line option:

    $ riscv32-unknown-elf-gcc --print-multi-lib
    .;
    rv32i/ilp32;@march=rv32i@mabi=ilp32
    rv32im/ilp32;@march=rv32im@mabi=ilp32
    rv32iac/ilp32;@march=rv32iac@mabi=ilp32
    rv32imac/ilp32;@march=rv32imac@mabi=ilp32
    rv32imafc/ilp32f;@march=rv32imafc@mabi=ilp32f
    rv64imac/lp64;@march=rv64imac@mabi=lp64
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

On RISC-V, PicoLibc is compiled 8 times, while on ARM, the library is
compiled 20 times with the specified compiler options (replace the
'@'s with '-' to see what they will be).

### Running meson

Because I'm targeting smaller systems like the STM32F042 Cortex-M0
parts with 4kB of RAM and 32kB of flash, I enable all of the 'make it
smaller' options. This example is in the do-arm-configure file:

    #!/bin/sh
    ARCH=arm-none-eabi
    DIR=`dirname $0`
    meson $DIR \
	    -Dtarget-optspace=true \
	    -Dnewlib-tinystdio=true \
	    -Dnewlib-supplied-syscalls=false \
	    -Dnewlib-reentrant-small=true\
	    -Dnewlib-wide-orient=false\
	    -Dnewlib-nano-malloc=true\
	    -Dlite-exit=true\
	    -Dnewlib-global-atexit=true\
	    -Dincludedir=lib/newlib-nano/$ARCH/include \
	    -Dlibdir=lib/newlib-nano/$ARCH/lib \
	    --cross-file $DIR/cross-$ARCH.txt \
	    --buildtype plain

Note the use of '--buildtype plain'. This stops meson from adding
compilation options so that the '-Dtarget-optspace=true' option can
select '-Os'.

This script is designed to be run from a build directory, so you'd do:

    $ mkdir build-arm-none-eabi
    $ cd build-arm-none-eabi
    $ ../do-arm-configure

### Compiling

Once configured, you can compile the libraries with

    $ ninja
    ...
    $ ninja install
    ...
    $

### Using the library

We should configure the compiler so that selecting a suitable target
architecture combination would set up the library paths to match, but
at this point you'll have to figure out the right -L line by yourself
by matching the path name on the left side of the --print-multi-lib
output with the compiler options on the right side. For instance, my
STM32F042 cortex-M0 parts use

    $ arm-none-eabi-gcc -mlittle-endian -mcpu=cortex-m0 -mthumb

To gcc, '-mcpu=cortex=m0' is the same as '-march=armv6s-m', so looking
at the output above, the libraries we want are in

    /usr/local/lib/newlib-nano/arm-none-eabi/lib/thumb/v6-m

so, to link, we need to use:

    $ arm-none-eabi-gcc ... -L/usr/local/lib/newlib-nano/arm-none-eabi/lib/thumb/v6-m -lm -lc -lgcc

## Building for the local processor

If you want to compile the library for your local processor to test
changes in the library, the meson configuration is happy to do that
for you. You won't need a meson cross compilation configuration file,
so all you need is the right compile options. They're mostly the same
as the embedded version, but you don't want the multi-architecture
stuff and I prefer plain debug to an -Os, as that makes debugging the
library easier.

The do-native-configure script has an example:

    #!/bin/sh
    DIR=`dirname $0`
    meson $DIR \
	    -Dmultilib=false \
	    -Dnewlib-tinystdio=true \
	    -Dnewlib-supplied-syscalls=false \
	    -Dnewlib-wide-orient=false\
	    -Dnewlib-nano-malloc=true\
	    -Dlite-exit=true\
	    -Dnewlib-global-atexit=true\
	    -Dincludedir=lib/newlib-nano/include \
	    -Dlibdir=lib/newlib-nano/lib \
	    -Dtests=true \
	    --buildtype debug

Again, create a directory and build there:

    $ mkdir build-native
    $ cd build-native
    $ ../do-native-configure
    $ ninja

This will also build a test case for printf and scanf in the
'test' directory, which I used to fix up the floating point input and
output code.
