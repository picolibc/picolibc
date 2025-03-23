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

### General build options

These options control some general build configuration values.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| multilib                    | true    | Build every multilib configuration supported by the compiler                         |
| multilib-list               | <empty> | If non-empty, the set of multilib configurations to compile for                      |
| multilib-exclude            | <empty> | Multilib configurations containing any of these strings will not be built            |
| sanitize-bounds             | false   | Build the library with -fsanitize=bounds                                             |
| sanitize-trap-on-error      | false   | Build the library with -fsanitize-undefined-trap-on-error                            |
| profile                     | false   | Enable profiling by adding -pg -no-pie to compile flags                              |
| analyzer                    | false   | Enable the analyzer while compiling with -fanalyzer                                  |
| assert-verbose              | false   | Display file, line and expression in assert() messages                               |
| fast-strcmp                 | true    | Always optimize strcmp for performance (to make Dhrystone happy)                     |

### Installation options

These options select where to install the library. Picolibc supports
installing multiple versions of the library into the same destination
root with the build-type-subdir option. This can be used to install
both space and speed optimized versions of the library. Users can
select which version with the 

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| build-type-subdir
| specsdir                    | auto    | Where to install the .specs file (default is in the GCC directory). <br> If set to `none`, then picolibc.specs will not be installed at all.|
| sysroot-install             | false   | Install in GCC sysroot location                                                      |
| sysroot-install-skip-checks | false   | Skip sysroot path check when using sysroot-install.                                  |
| system-libc                 | false   | Install as system C library. Requires compiler support for picolibc                  |

### Testing options

Use these options to configure the library for testing. This includes
testing the library using the host compiler and host C library.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| tests                       | false   | Enable tests                                                                         |
| tests-cdefs                 | auto    | Ensure every public header ends up including sys/cdefs.h. When set to auto, follows tests value |
| tests-enable-stack-protector| true    | Build tests with stack protector enabled using -fstack-protector-all -fstack-protector-strong |
| tests-enable-full-malloc-stress | false | Also enable malloc stress testing when building full malloc. This is always enabled for nano-malloc |
| tests-enable-posix-io       | true    | Include tests which require full POSIX API                                           |
| test-long-double            | true    | Enable long double tests.                                                            |
| split-large-tests           | false   | For tests which generate large executables, split them apart for smaller targets     |
| test-stdin                  | false   | Include tests which require working stdin                                            |
| fortify-source              | 3       | Use this for _FORTIFY_SOURCE value when building tests                               |
| test-machine                | qemu    | Target machine type for tests. AArch64 also supports 'fvp'                           |
| freestanding                | false   | Build entire library with -ffreestanding. Used to be useful for Zephyr testing       |
| native-tests                | false   | Build tests against host libc (used to validate tests)                               |
| native-math-tests           | true    | Also build math tests against host libc when native-tests is enabled                 |
| use-stdlib                  | false   | Do not link tests with -nostdlib. Useful for native testing.                         |
| picolib                     | true    | Include 'picolib' bits. Disable when doing native testing.                           |
| semihost                    | true    | Build semihost libary. Disable when doing native testing.                            |
| fake-semihost               | false   | Create a fake semihost library to allow tests to link                                |

### Stdio options

These options select which stdio implementation to use along with
options common to both implementations.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| tinystdio                   | true    | Use tiny stdio from avr libc                                                         |
| io-c99-formats              | true    | Enable C99 support in IO functions like printf/scanf                                 |
| io-long-long                | false   | Enable long long type support in IO functions like printf/scanf. For tiny-stdio, this only affects the integer-only versions, the double and float versions always include long long support. |
| io-pos-args                 | false   | Enable printf-family positional arg support. For tiny-stdio, this only affects the integer-only versions, the double and float versions always include positional argument support. |
| io-long-double              | false   | Enable long double support in printf/scanf.                                          |

### Tinystdio options

Tinystdio offers a smaller implementation than the legacy code while
avoiding the use of malloc except when creating new FILE objects. Most
I/O operations are done one byte at a time.

For stdin/stdout/stderr, the application will need to provide
`stdin`, `stdout` and `stderr`, which are three pointers to FILE
structures (which can all reference a single shared FILE structure,
and which can be aliases to the same underlying global pointer).

Note that while posix-io support is enabled by default, using it will
require that the underlying system offer the required functions. POSIX
console support offers built-in `stdin`, `stdout` and `stderr`
definitions which use the same POSIX I/O functions.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| io-float-exact              | true    | Provide round-trip support in float/string conversions                               |
| atomic-ungetc               | true    | Make getc/ungetc re-entrant using atomic operations                                  |
| posix-console               | false   | Use POSIX I/O for stdin/stdout/stderr                                                |
| format-default              | double  | Sets the default printf/scanf style ('d, 'f', 'l', 'i' or 'm')                       |
| printf-aliases              | true    | Support link-time printf aliases to set the default printf/scanf variant             |
| io-percent-b                | false   | Support the C23 %b printf specifier for binary formatted integers                    |
| printf-small-ultoa          | false   | Avoid soft division routine during integer binary to decimal conversion in printf    |
| printf-percent-n            | false   | Support the dangerous %n format specifier in printf                                  |
| minimal-io-long-long        | false   | Support long long values in the minimal ('m') printf and scanf variants              |
| fast-bufio                  | false   | Improve performance of some I/O operations when using bufio                          |
| io-wchar                    | false   | Enable wide character support in printf and scanf when mb-capable is not set         |

### Legacy stdio options

Normally, Picolibc is built with the small stdio library adapted from
avrlibc (tinystdio=true). It still has the original newlib stdio bits
and those still work (tinystdio=false), but depend on POSIX I/O
functions from the underlying system, and perform many malloc calls at
runtime. These options are relevant only in that configuration

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| newlib-elix-level           | 4       | Extends stdio API based on level                                                     |
| newlib-fseek-optimization   | false   | Enable fseek optimization                                                            |
| newlib-fvwrite-in-streamio  | false   | Enable iov in streamio                                                               |
| newlib-io-float             | false   | Enable printf/scanf family float support                                             |
| newlib-nano-formatted-io    | false   | Use nano version formatted IO                                                        |
| newlib-stdio64              | true    | Include 64-bit APIs                                                                  |
| newlib-unbuf-stream-opt     | false   | Enable unbuffered stream optimization in streamio                                    |
| newlib-wide-orient          | false   | Track wide character orientation in streamio                                         |
| newlib-have-fcntl           | false   | System has fcntl function available                                                  |

### Internationalization options

These options control how much internationalization support is included
in the library. Picolibc supports locale names of the form '<locale>' or
'<locale>.<charset>'; e.g. POSIX or C.UTF-8. When no charset is
specified, picolibc selects ASCII when multi-byte support is disabled
or locale is C. Otherwise, picolibc selects UTF-8.

Picolibc supports the same charsets for locales and iconv.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| mb-capable                  | false   | Enable multibyte support including UTF-8                                             |
| mb-extended-charsets        | false   | Enable all additional UCS, ISO, Windows and JIS charsets                             |
| mb-ucs-charsets             | false   | Enable additional UCS charsets (UCS-2 and UCS-4)                                     |
| mb-iso-charsets             | false   | Enable ISO-8859 charsets (ISO 8859-1 through ISO 8859-16)                            |
| mb-jis-charsets             | false   | Enable JIS charsets (JIS X 0208, EUC-JP and Shift-JIS)                               |
| mb-windows-charsets         | false   | Enable various code-page based charsets                                              |

### Startup/shutdown options

These control how much support picolibc includes for calling functions
at startup and shutdown times.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| picocrt                     | true    | Build crt0.o (C startup function)                                                    |
| picocrt-enable-mmu          | true    | Enable memory management unit in picocrt where supported by the hardware             |
| picocrt-lib                 | true    | Also wrap crt0.o into a library -lcrt0, which is easier to find via the library path |
| picoexit                    | true    | Use smaller implementation that allows only 32 init and fini handlers                |
| initfini-array              | true    | Use .init_array and .fini_array sections in picocrt                                  |
| initfini                    | false   | Support _init() and _fini() functions in picocrt                                    |
| crt-runtime-size            | false   | Compute .data/.bss sizes at runtime rather than linktime. <br> This option exists for targets where the linker can't handle a symbol that is the difference between two other symbols |

### Legacy (non-picoexit) startup/shutdown options

These options cover the (non-default) legacy exit handler support when
picoexit is disabled.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| newlib-atexit-dynamic-alloc | false   | Allocate additional space for handlers as necessary                                  |
| newlib-global-atexit        | false   | Place exit handlers in global variables instead of thread-local ones                 |
| newlib-register-fini        | false   | Register function that invokes destructors using atexit                              |
| newlib-register-fini        | false   | Register 

### Malloc options

Picolibc offers two malloc implementations, the larger version offers
better performance on large memory systems and for applications doing
a lot of variable-sized allocations and deallocations. The smaller,
default, implementation works best when applications perform few,
persistent allocations.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| newlib-nano-malloc          | true    | Use small-footprint nano-malloc implementation                                       |
| nano-malloc-clear-freed     | false   | Set contents of freed memory to zero when using nano-malloc                          |

### Locking options

This option selects whether to disable locking support within the
library. By default, locking support is enabled, but the default
locking functions are just stubs which don't do anything. Applications
can replace these stubs with their own implementation and enable full
locking as described in [locking.md](locking.md).

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| single-thread               | false   | Disable support for locks                                                            |

### Thread local storage options

By default, Picolibc can uses native TLS support as provided by the
compiler, this allows re-entrancy into the library if the run-time
environment supports that. A TLS model is specified only when TLS is
enabled. The default TLS model is local-exec.

As a separate option, you can make `errno` not use TLS if necessary.

| Option                      | Default | Description                                                                          |
| ------                      | ------- | -----------                                                                          |
| thread-local-storage        | auto    | Use TLS for global variables. Default is automatic based on compiler support         |
| tls-model                   | local-exec | Select TLS model (global-dynamic, local-dynamic, initial-exec or local-exec)      |
| newlib-global-errno         | false   | Use single global errno even when thread-local-storage=true                          |
| errno-function              | <empty> | If set, names a function which returns the address of errno. 'auto' will try to auto-detect. |
| tls-rp2040                  | false   | Use Raspberry PI RP2040 CPUID register to index thread local storage value           |
| stack-protector-guard       | auto    | Select stack protection canary type (global, tls or auto)                            |

### Math options

There are two versions of many libm functions, old ones from SunPro
and new ones from ARM. The new ones are generally faster for targets
with hardware double support, except that the new float-valued
functions use double-precision computations. On sytems without
hardware double support, that's going to pull in soft double
code. Measurements show the old routines are generally more accurate,
which is why they are enabled by default.

POSIX requires many of the math functions to set errno when exceptions
occur; disabling that makes them only support fenv() exception
reporting, which is what IEEE floating point and ANSI C standards
require.

| Option                      | Default | Description                                             |
| ------                      | ------- | -----------                                             |
| newlib-obsolete-math        | true    | Use old code for both float and double valued functions |
| newlib-obsolete-math-float  | auto    | Use old code for float-valued functions                 |
| newlib-obsolete-math-double | auto    | Use old code for double-valued functions                |
| want-math-errno             | false   | Set errno when exceptions occur                         |

newlib-obsolete-math provides the default value for the
newlib-obsolete-math-float and newlib-obsolete-math-double parameters;
those control the compilation of the individual fucntions.

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

    [properties]
    c_args = [ '-nostdlib', '-msave-restore', '-fno-common' ]
    # default multilib is 64 bit
    c_args_ = [ '-mcmodel=medany' ]
    needs_exe_wrapper = true
    skip_sanity_check = true

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

    [properties]
    c_args = [ '-nostdlib', '-fno-common' ]
    needs_exe_wrapper = true
    skip_sanity_check = true

If those programs aren't in your path, you can edit the file to point
wherever they may be.

### Auto-detecting the compiler multi-lib configurations

The PicoLibc configuration detects the processor configurations
supported by the compiler using the `--print-multi-lib` command-line option:

    $ riscv64-unknown-elf-gcc --print-multi-lib
    .;
    rv32e/ilp32e;@march=rv32e@mabi=ilp32e
    rv32ea/ilp32e;@march=rv32ea@mabi=ilp32e
    rv32em/ilp32e;@march=rv32em@mabi=ilp32e
    rv32eac/ilp32e;@march=rv32eac@mabi=ilp32e
    rv32emac/ilp32e;@march=rv32emac@mabi=ilp32e
    rv32i/ilp32;@march=rv32i@mabi=ilp32
    rv32if/ilp32f;@march=rv32if@mabi=ilp32f
    rv32ifd/ilp32d;@march=rv32ifd@mabi=ilp32d
    rv32ia/ilp32;@march=rv32ia@mabi=ilp32
    rv32iaf/ilp32f;@march=rv32iaf@mabi=ilp32f
    rv32imaf/ilp32f;@march=rv32imaf@mabi=ilp32f
    rv32iafd/ilp32d;@march=rv32iafd@mabi=ilp32d
    rv32im/ilp32;@march=rv32im@mabi=ilp32
    rv32imf/ilp32f;@march=rv32imf@mabi=ilp32f
    rv32imfc/ilp32f;@march=rv32imfc@mabi=ilp32f
    rv32imfd/ilp32d;@march=rv32imfd@mabi=ilp32d
    rv32iac/ilp32;@march=rv32iac@mabi=ilp32
    rv32imac/ilp32;@march=rv32imac@mabi=ilp32
    rv32imafc/ilp32f;@march=rv32imafc@mabi=ilp32f
    rv32imafdc/ilp32d;@march=rv32imafdc@mabi=ilp32d
    rv64i/lp64;@march=rv64i@mabi=lp64
    rv64if/lp64f;@march=rv64if@mabi=lp64f
    rv64ifd/lp64d;@march=rv64ifd@mabi=lp64d
    rv64ia/lp64;@march=rv64ia@mabi=lp64
    rv64iaf/lp64f;@march=rv64iaf@mabi=lp64f
    rv64imaf/lp64f;@march=rv64imaf@mabi=lp64f
    rv64iafd/lp64d;@march=rv64iafd@mabi=lp64d
    rv64im/lp64;@march=rv64im@mabi=lp64
    rv64imf/lp64f;@march=rv64imf@mabi=lp64f
    rv64imfc/lp64f;@march=rv64imfc@mabi=lp64f
    rv64imfd/lp64d;@march=rv64imfd@mabi=lp64d
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

On RISC-V, PicoLibc is compiled 36 times, while on ARM, the library is
compiled 20 times with the specified compiler options (replace the
'@'s with '-' to see what they will be).

### Running meson

Because Picolibc targets smaller systems like the SiFive FE310 or ARM
Cortex-M0 parts with only a few kB of RAM and flash, the default
values for all of the configuration options are designed to minimize
the library code size. Here's the
[do-riscv-configure](../scripts/do-riscv-configure) script from the repository
that configures the library for small RISC-V systems:

```sh
#!/bin/sh
ARCH=riscv64-unknown-elf
DIR=`dirname $0`
meson "$DIR" \
    -Dincludedir=picolibc/$ARCH/include \
    -Dlibdir=picolibc/$ARCH/lib \
    --cross-file "$DIR"/cross-$ARCH.txt \
    "$@"
```

This script is designed to be run from a build directory, so you'd do:

    $ mkdir build-riscv64-unknown-elf
    $ cd build-riscv64-unknown-elf
    $ ../scripts/do-riscv-configure

You can select the installation directory by passing it to the meson script:

    $ ../scripts/do-riscv-configure -Dprefix=/path/to/install/dir/

### Compiling

Once configured, you can compile the libraries with

    $ ninja
    ...
    $ ninja install
    ...
    $
