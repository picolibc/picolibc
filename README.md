# Picolibc
Copyright © 2018-2023 Keith Packard

Picolibc is library offering standard C library APIs that targets
small embedded systems with limited RAM. Picolibc was formed by blending
code from [Newlib](http://sourceware.org/newlib/) and
[AVR Libc](https://www.nongnu.org/avr-libc/).

Build status:

 * ![Linux](https://github.com/picolibc/picolibc/workflows/Linux/badge.svg?branch=main)
 * ![Zephyr](https://github.com/picolibc/picolibc/workflows/Zephyr/badge.svg?branch=main)
 * ![Mac OS X](https://github.com/picolibc/picolibc/workflows/Mac%20OS%20X/badge.svg)

## License

Picolibc source comes from a variety of places and has a huge variety
of copyright holders and license texts. While much of the code comes
from Newlib, none of the GPL-related bits used to build the library
are left in the repository, so all of the source code uses BSD-like
licenses, a mixture of 2- and 3- clause BSD itself and a variety of
other (mostly older) licenses with similar terms.

There are two files used for testing printf, test/printf-tests.c and
test/testcases.c which are licensed under the GPL version 2 or
later. There is also a shell script, GeneratePicolibcCrossFile.sh
which is licensed under the AGPL version 3 or later which is provided
as a helper for people building the library, but not used by picolibc
otherwise.

The file COPYING.picolibc contains all of the current copyright and
license information in the Debian standard machine-readable format. It
was generated using the make-copyrights and find-copyright
scripts.

## Supported Architectures

Picolibc has integrated testing support for many architectures which
is used to validate the code for all patch integration:

 * ARC (32- and 64- bit)
 * ARM (32- and 64- bit)
 * i386 (Native and Linux hosted, for testing)
 * Motorola 68000 (m68k)
 * MIPS
 * MSP430
 * Nios II
 * Power9
 * RISC-V (both 32- and 64- bit)
 * SparcV8 (32 bit)
 * x86_64 (Native and Linux hosted, for testing)

There is also build infrastructure and continuous build validation,
but no integrated testing available for additional architectures:

 * Microblaze (32-bit, big and little endian)
 * PowerPC (big and little endian)
 * Sparc64
 * Xtensa (ESP8266, ESP32)

Supporting architectures that already have Newlib code requires:

 1. newlib/libc/machine/_architecture_/meson.build to build the
    architecture-specific libc bits. This should at least include
    setjmp/longjmp support as these cannot be performed in
    architecture independent code and are needed by libstdc++.

 2. Checking for atomic support for tinystdio. Tinystdio requires
    atomics for ungetc to work correctly in a reentrant
    environment. By default, it stores them in 16-bit values, but
    some architectures only have 32-bit atomics. To avoid ABI
    issues, the size selected isn't detected automatically, instead
    it must be configured in newlib/libc/tinystdio/stdio.h.

 3. newlib/libm/machine/_architecture_/meson.build to build any
    architecture-specific libm bits

 4. picocrt/machine/_architecture_ source code and build bits
    for startup code needed for the architecture. Useful in all
    cases, but this is necessary to run tests under qemu if your
    platform can do that.

 5. cross-_gcc-triple_.txt to configure the meson cross-compilation
    mechanism to use the right tools

 6. do-_architecture_-configure to make testing the cross-compilation
    setup easier.

 7. newlib/libc/picolib support. This should include whatever startup
    helpers are required (like ARM interrupt vector) and TLS support
    (if your compiler includes this).

 8. run-_architecture_ script to run tests under QEMU. Look at the ARM
    and RISC-V examples to get a sense of what this needs to do and
    how it gets invoked from the cross-_gcc-triple_.txt configuration
    file.

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

Introductory documentation. Read these first:

 * [Building Picolibc](doc/build.md). Explains how to compile picolibc yourself.
 * [Using Picolibc](doc/using.md). Shows how to compile and link
   applications once you have picolibc built and installed.
 * [Linking with Picolibc.ld](doc/linking.md). Provides more details
   about the linking process.
 * [Hello World](hello-world/README.md). Build and run a stand-alone C
   application by following step-by-step instructions

Detailed documentation. Use these to learn more details about how to
use Picolibc:

 * [Picolibc initialization](doc/init.md)
 * [Operating System Support](doc/os.md).
 * [Printf and Scanf in Picolibc](doc/printf.md)
 * [Thread Local Storage](doc/tls.md)
 * [Re-entrancy and Locking](doc/locking.md)
 * [Selecting ctype implementation](doc/ctype.md)
 * [Picolibc as embedded source](doc/embedsource.md)
 * [Releasing Picolibc](doc/releasing.md)
 * [Copyright and license information](COPYING.picolibc)

## Releases

### Picolibc release 1.8.next

 * Use common clang/gcc feature detection macros on arm.

 * Additional clang/compiler-rt work-arounds for arm which is less
   consistent in handling exceptions.

 * Use clang multilib support for aarch64

 * Build fix on arc which would build two strchr versions in release
   mode.

 * Add picocrt and semihost support for xtensa. Test xtensa dc233c.

 * Add C11's <uchar.h> header and implementation.

 * Add nano-malloc-clear-freed option to erase memory released in free
   or realloc.

 * Add memset_explicit from C23.

 * Work around broken clang builtin malloc which fails to set errno.

 * Widen C++ _CTYPE_DATA array to fix mis-classification of \t; C++
   requires bitmasks for all ctype operations, and 8 bits is not
   enough. Thanks to M-Moawad.

 * Update case conversion tables to Unicode 15.1.0

 * Fix documentation formatting. Thanks to Eduard Tanase.

 * Fix support for using long-long vfprintf version by default. Thanks
   to Louis Peens.

 * Remove arm unaligned memcpy asm code. This couldn't support targets
   that only supported unaligned access. Use a new faster C version
   for this case.

 * Add asnprintf and vasnprintf as provided by newlib

 * Support ARM's FVP emulator. Thanks to Oliver Stannard.

 * Remove arc strlen asm code as it would access memory *before* the
   provided buffer and fall afoul of the stack bounds checking
   hardware.

 * Support --printf={d,f,l,i,m} in place of
   -DPICOLIBC_*_PRINTF_SCANF. This is the syntax proposed in the
   patches submitted to gcc for picolibc support.

 * Add LoongArch support, including testing. Thanks to Jiaxun Yang.

 * Use new picolibc-ci-tools project which builds custom toolchain
   bits automatically. Thanks to Jiaxun Yang.

 * Add OpenRisc support, including testing. Thanks to Joel Holdsworth.

 * Add LatticMico32 support, including testing. Thanks to Jiaxun Yang.

### Picolibc release 1.8.8

 * Fixed 3 bugs in the powf computation. Thanks to Fabian Schriever.

 * Fixed a bunch of build issues found by Zephyr.

 * Improve C++ testing and compatibility.

### Picolibc release 1.8.7

 * Support ARM v8.1-m BTI and PAC features

 * Fix stdio buffered backend automatic flushing of stdout when
   reading stdin.

 * Support _FORTIFY_SOURCE=3

 * Fix several fesetround implementations to return an error when
   passed an invalid argument. Thanks to Abdallah Abdelhafeez.

 * Document headers which the compiler must provide. Thanks to Alexey
   Brodkin.

 * Generate mktemp/tmpnam filenames using random() so they don't
   repeat even if they aren't used before another name is generated.

 * Set error flag when fgetc is called on an file without read
   mode. Thanks to Mohamed Moawad.

 * Add type casting to CMPLX, CMPLXF and CMPLXL macros (as glibc
   does). Thanks to Mostafa Salman.

 * Add mips64 support and build the library during CI.

 * Make fgets return any accumulated string on EOF instead of
   always returning NULL. Thanks to Hana Ashour.

 * Use C99 minimum array size in asctime_r and ctime_r API
   declarations ('[static 26]'). Bounds check the generated value and
   return NULL/EOVERFLOW on overflow.

 * Make Zephyr's -Oz cmake option enable
   PREFER_SIZE_OVER_SPEED. Thanks to Jonathon Penix.

 * Add funopen to tinystdio.

 * Validate all public headers with a C++ compiler to make sure they
   at least compile successfully. Fix time.h.

 * Stop using -include picolibc.h during library build.

 * Add -Wmissing-declarations and -Wmissing-prototypes to library
   build flags. Fix a rather large pile of missing prototypes caused
   by source files failing to add _GNU_SOURCE or _DEFAULT_SOURCE
   definitions.

 * Add POSIX "unlocked" I/O functions to tinystdio. These don't
   actually do anything because tinystdio doesn't do any
   locking. However, flockfile/funlockfile grab the global C library
   lock so applications synchronizing with that API will "work".

 * Fix wide orientation handling in tinystdio. Thanks to Ahmed Shehab.

 * Add aarch64 soft float support for armv8. Clang allows this with
   -march=armv8-a+nofp -mabi=aapcs-soft. This required building a
   custom toolchain that included a compiler-rt library built with the
   right options.

 * Add fgetpos and fsetpos to tinystdio. Thanks to Hana Ashour.

 * Restore missing members of 'struct sigevent'. Over eager removal of
   _POSIX_THREADS support caused these to be accidentally deleted some
   time ago.

 * Test on i386 native target.

 * Fix hex float scanning and printing. Thanks to Hana Ashour and
   Ahmed Shehab.

 * Fix double rounding in %f printf. Thanks to Ahmed Shehab for
   constructing a test case that identified the issue.

 * Add mem_align to the "big" malloc version. Thanks to Simon Tatham.

 * Adjust POSIX and C headers to limit symbol exposure to that
   specified in the standards.

 * Fix rounding in float scanf. This does round twice for input longer
   than the required number of digits, but that's permitted by the C
   specification.

 * Support %a/%A in scanf. Support arbitrary precision in %a/%A
   printf. Fix NaN/INF formatting in %a/%A printf. Thanks to Ahmed
   Shehab.

 * Provide a build-time option to enable %n in printf. This is
   disabled by default for security concerns, but supported in case
   someone needs strict C conformance. Thanks to Ahmed Shehab.

 * Make freopen clear the unget buffer. Thanks to Mostafa Salman.

 * Fix wide and multi-byte character support in printf and scanf. For
   strict standards conformance, there's now an option that enables
   %lc/%ls in printf even if multi-byte support is not enabled.

 * Enable MMU in picocrt on A profile ARM and AARCH64 targets when
   present. This is required by the latest qemu which now more
   accurately emulates this hardware. Thanks to Alex Richardson.

 * Fix AARCH64 asm code in ILP32 mode.

 * Parse NaN(<string>) in sscanf. This is required by the standard,
   although picolibc doesn't do anything with <string>. Thanks to
   Mohamed Moawad.

 * Clean up header files. Picolibc tries to limit symbol definitions
   to those specified in the C and POSIX specs.

 * Add support for C's Annex K functions. These are bounds-checking
   versions of various memory and string functions. Thanks to Mostafa
   Salman.

 * Perform locale string validation in newlocale even when _MB_CAPABLE
   isn't defined. Thanks to Mostafa Salman.

 * Place compiler-rt library after C library when linking
   tests. Thanks to Oliver Stannard.

### Picolibc version 1.8.6

 * Fix some FORTITY_SOURCE issues with tinystdio

 * Add __eh_* symbols to picolibc.ld for LLVM libunwind. Thanks Alex
   Richardson.

 * Merge in newlib annual release (4.4.0). Some minor updates to
   aarch64 assembly code formatting (thanks to Sebastian Huber) and a
   few other fixes.

 * Enable 32-bit SPARC for testing.

 * Fix a bunch of fmemopen bugs and add some tests. Thanks to Alex
   Richardson.

 * Finish support for targets with unusual float types, mapping
   target types to 32-, 64-, 80- and 128- bit picolibc code.

 * Add SuperH support, including testing infrastructure. Thanks to
   Adrian Siekierka for help with this.

 * Improve debugger stack trace in risc-v exception code. Thanks to
   Alex Richardson.

 * Add an option (-Dfast-bufio=true) for more efficient fread/fwrite
   implementations when layered atop bufio. Thanks for the suggestion
   from Zachary Yedidia.

 * Fix cmake usage of FORMAT_ variables (note the lack of a leading
   underscore).

 * Remove explicit _POSIX_C_SOURCE definition in zephyr/zephr.cmake.

 * Clean up public inline functions to share a common mechanism for
   using gnu_inline semantics. Fix isblank. This ensures that no
   static inline declarations exist in public API headers which are
   required to be external linkage ("real") symbols.

 * Create an alternate ctype implementation that avoids using the
   _ctype_ array and just does direct value comparisons. This only
   works when picolibc is limited to ASCII. Applications can select
   whether they want this behavior at application compilation time
   without needing to rebuild the C library. Thanks to P. Frost for
   the suggestion.

 * Unify most fenv implementations to use gnu_inline instead of
   regular functions to improve performance. x86 was left out because
   those fenv functions are complicated by the mix of 8087 and modern
   FPU support.

 * Add a separate FILE for stderr when using POSIX I/O. Split
   stdin/stdout/stderr into three files to avoid pulling in
   those which aren't used. Thanks to Zachary Yedidia.

### Picolibc version 1.8.5

 * Detect clang multi-lib support correctly by passing compiler flags.
   Thanks to xbjfk for identifying the problem.

 * Create a new 'long-long' printf variant. This provides enough
   variety to satisfy the Zephyr cbprintf options without needing to
   build the library from scratch.

 * Adjust use of custom binary to decimal conversion code so that it
   is only enabled for types beyond the register size of the
   target. This avoids the cost of this code when the application is
   already likely to be using the soft division routines.

### Picolibc version 1.8.4

 * Make math overflow and underflow handlers respect rounding modes.

 * Add full precision fma/fmaf fallbacks by adapting the long-double
   code which uses two floats/doubles and some careful exponent
   management to ensure that only a single rounding operation occurs.

 * Fix more m68k 80-bit float bugs

 * Fix m68k asm ABI by returning pointers in %a0 and %d0

 * Use an m68k-unknown-elf toolchain for m68k testing, including
   multi-lib to check various FPU configurations on older and more
   modern 68k targets.

 * Improve CI speed by using ccache on zephyr and mac tests,
   compressing the docker images and automatically canceling jobs when
   the related PR is updated. Thanks to Peter Jonsson.

 * Move a bunch of read-only data out of RAM and into flash by adding
   'const' attributes in various places.

 * Add a new linker symbol, `__heap_size_min`, which specifies a
   minimum heap size. The linker will emit an error if this much space
   is not available between the end of static data and the stack.

 * Fix a bunch of bugs on targets with 16-bit int type. Thanks to
   Peter Jonsson for many of these.

 * Work around a handful of platform bugs on MSP430. I think these are
   compiler bugs as they occur using both the binutils simulator and
   mspsim.

 * Run tests on MSP430 using the simulator that comes with gdb. Thanks to
   Peter Jonsson for spliting tests apart to make them small enough to
   link in under 1MB. This requires a patch adding primitive
   semihosting to the simulator.

 * Provide a division-free binary to decimal conversion option for
   printf at friends. This is useful on targets without hardware
   divide as it avoids pulling in a (usually large) software
   implementation. This is controlled with the 'printf-small-ultoa'
   meson option and is 'false' by default.

 * Add 'minimal' printf and scanf variants. These reduce functionality
   by removing code that acts on most data modifers including width
   and precision fields and alternate presentation modes. A new config
   variable, minimal-io-long-long, controls whether that code supports
   long long types.

 * Add a 'assert-verbose' option which controls whether the assert
   macro is chatty by default. It is 'true' by default, which
   preserves the existing code, but when set to 'false', then a
   failing assert calls __assert_no_msg with no arguments, saving the
   memory usually occupied by the filename, function name and
   expression.

 * Fix arm asm syntax for mrc/mcr instructions to make clang happy.
   Thanks to Radovan Blažek for this patch.

### Picolibc version 1.8.3

 * Fix bugs in floor and ceil implementations.

 * Use -fanalyzer to find and fix a range of issues.

 * Add __ubsan_handle_out_of_bounds implementation. This enables
   building applications with -fsanitize=bounds and
   -fno-sanitize-undefined-trap-on-error.

 * Validate exception configuration on targets with mixed exception
   support where some types have exceptions and others don't. Right
   now, that's only arm platforms where any soft float implementations
   don't build with exception support.

 * Fix bugs in nexttowards/nextafter on clang caused by the compiler
   re-ordering code and causing incorrect exception generation.

 * Use the small/slow string code when -fsanitize=address is used
   while building the library. This avoids reading beyond the end of
   strings and triggering faults.

 * Handle soft float on x86 and sparc targets. That mostly required
   disabling the hardware exception API, along with a few other minor
   bug fixes.

 * Add runtime support for arc, mips, nios2 and m68k. This enables CI
   testing on these architectures using qemu.

 * Fix 80-bit floating math library support for m68k targets.

 * Fix arm testing infra to use various qemu models that expand
   testing to all standard multi-lib configurations.

 * Adjust floating exception stubs to return success when appropriate,
   instead of always returning ENOSYS.

 * Make sure sNaN raises FE_INVALID and is converted to qNaN in
   truncl, frexpl and roundl

 * Avoid NaN result from fmal caused by multiply overflow when
   addend is infinity (-inf + inf results in NaN in that case).

### Picolibc version 1.8.2

 * Support _ZEPHYR_SOURCE macro which, like _POSIX_SOURCE et al,
   controls whether the library expresses the Zephyr C library API.
   This is also automatically selected when the __ZEPHYR__ macro is
   defined and no other _*_SOURCE macro is defined.

 * Add another cross compile property, 'libgcc', which specifies the
   library containing soft float and other compiler support routines.

 * Fix a couple of minor imprecisions in pow and 80-bit powl.

 * Merge newlib changes that included an update to the ARM assembly
   code.

 * Replace inexact float/string conversion code with smaller code that
   doesn't use floating point operations to save additional space on
   soft float targets.

 * More cmake fixes, including making the inexact printf and locale
   options work.

### Picolibc version 1.8.1

 * Fix cmake build system to auto-detect compiler characteristics
   instead of assuming the compiler is a recent version of GCC. This
   allows building using cmake with clang.

 * Fix cmake build system to leave out TLS support when TLS is
   disabled on the cmake command line.

 * Replace inline asm with attributes for __weak_reference macro

 * Add allocation attributes to malloc and stdio functions. This
   allows the compiler to detect allocation related mistakes as well
   as perform some additional optimizations. Bugs found by this change
   were also addressed.

 * Add wchar_t support to tinystdio, eliminating the last missing
   feature compared with the legacy stdio bits from newlib. With this,
   libstdc++ can be built with wide char I/O support, eliminating the
   last missing feature there as well.

 * Eliminate use of command line tools when building with a new enough
   version of meson. Thanks to Michael Platings.

 * Add Microblaze support. Thanks to Alp Sayin.

 * Switch semihosting to use binary mode when opening files. Thanks to
   Hardy Griech.

 * Build and install static library versions of the crt0 startup
   code. These allows developers to reference them as libraries on the
   command line instead of needing special compiler support to locate
   the different variants, which is useful when using clang. Thanks to
   Simon Tatham.

 * Simplify the signal/raise implementation to use a single global
   array of signal handlers and to not use getpid and kill, instead
   raise now directly invokes _exit. This makes using assert and abort
   simpler and doesn't cause a large TLS block to be allocated. Thanks
   to Joe Nelson for discovering the use of a TLS variable here.

### Picolibc version 1.8

With the addition of nearly complete long double support in the math
library, it seems like it's time to declare a larger version increment
than usual.

 * Improve arc and xtensa support, adding TLS helpers and other build fixes

 * Fix FPSCR state for Arm8.1-M low overhead loops (thanks to David
   Green)

 * Add -Werror=double-promotion to default error set and fix related
   errors. (thanks to Ryan McClelland)

 * Fix locking bug in malloc out-of-memory path and freeing a locked
   mutex in the tinystdio bufio code. These were found with lock
   debugging code in Zephyr.

 * Add some missing functions in tinystdio, strto*l_l, remove,
   tmpname/tmpfile which were published in stdio.h but not included in
   the library.

 * Switch read/write functions to use POSIX types instead of legacy
   cygwin types. This makes mapping to existing an POSIX api work
   right.

 * Add %b support to tinystdio printf and scanf. These are disabled by
   default as they aren't yet standardized.

 * Fix avr math function support. The avr version of gcc has modes
   where double and long double are 32 or 64 bits, so the math library
   code now detects all of that at compile time rather than build time
   and reconfigures the functions to match the compiler types.

 * Add nearly complete long double support from openlibm for 80-bit
   Intel and 128-bit IEEE values (in addition to supporting 64-bit
   long doubles). Still missing are Bessel functions and decimal
   printf/scanf support.

 * Add limited long double support for IBM 'double double' form. This
   is enough to run some simple tests, but doesn't have any
   significant math functions yet.

 * Get Power9 code running under qemu with OPAL. This was mostly
   needed to validate the big-endian and exception code for 128-bit
   long doubles, but was also used to validate the double double
   support.

 * Provide times() and sysconf() implementations in semihosting. You
   can now build and run the dhrystone benchmark without any further
   code.

 * Fix use of TLS variables with stricter alignment requirements in
   the default linker script and startup code. (thanks to Joakim
   Nohlgård and Alexander Richardson who found this issue while
   working on lld support).

### Picolibc version 1.7.9

 * Support all Zephyr SDK targets

 * Support relocating the toolchain by using GCC_EXEC_PREFIX for
   sysroot-install when compiler doesn't use sysroot.

 * Add MIPS, SPARC and ARC support

 * Deal with RISC-V changes in gcc that don't reliably include zicsr

 * Support Picolibc as default C library with -Dsystem-libc option.
   With this, you can use picolibc without any extra compiler options.

 * Merge current newlib bits to get code that doesn't use struct _reent

 * Get rid of struct _reent in legacy stdio code

 * Support 16-bit int targets by fixing a few places assuming
   sizeof(int) == 4, object sizes not using size_t, wint_t for
   ucs-4 values

 * Add MSP430 support

 * Fix a couple of clang bugs (one on Cortex M0)

 * Support libc++ by adding non-standard mbstate_t.h

 * Merge i686 and x86_64 code to allow x86 multilib builds

 * Merge Xtensa newlib bits

 * Support Xtensa ESP32 targets

 * Add Nios II support

### Picolibc version 1.7.8

 1. Fix el/ix level 4 code type errors

 2. Fix out-of-source CMake build (thanks Max Behensky)

 3. Improve build.md docs (thanks Kalle Raiskila)

 4. Fix cmake build for various architectures

 5. Initialize lock in fdopen

 6. Remove %M from linker paths in single-arch builds

 7. Shrink tinystdio vfprintf and vfscanf a bit

 8. Use -fno-builtin-malloc -fno-builtin-free (GCC 12 compat)

 9. Use -fno-builtin-copysignl (GCC 12 compat)

 10. Add _zicsr to -march for risc-v picocrt (binutils 2.38 compat)

 11. Add -no-warn-rwx-segments to link spec (binutils 2.38 compat)

### Picolibc version 1.7.7

 1. Fix semihost gettimeofday, add a test.

 2. Fix config option documentation. (Thanks to rdiez)

 3. Document how re-entrant locking APIs are used. (Thanks to rdiez)

 4. Fix some 16-bit int issues in tinystdio. (Thanks to Ayke van
    Laethem)

 5. Make header files a bit more POSIX compliant, installing rpc
    headers, moving byte swapping macros to arpa/inet.h

 6. Fix some stdio bugs found by Zephyr test suite: snprintf return
    value on buffer overflow, add ftello/fseeko, fputc return value,
    %0a formatting, clear EOF status after ungetc/fseek.

 7. Re-do buffered I/O support to handle mixed read/write files
    correctly. This adds setbuf, setbuffer, setlinebuf, setvbuf.

 8. Add fmemopen and freopen.

 9. Add enough cmake support to allow Zephyr to build picolibc as a
    module using that, rather than meson.

 10. Merge current newlib bits
 
 11. Fix %p printf/scanf on ILP64 targets.

### Picolibc version 1.7.6

 1. Fix use with C++ applications caused by a syntax error in
    picolibc.specs

 2. Automatically include '-nostdlib' to options used while
    evaluating build rules to ensure tests work as expected.

 3. Publish aarch64 inline math functions, ensure that inline fma
    functions work in installed applications for arm and risc-v.

### Picolibc version 1.7.5

 1. Fix build on big-endian systems (thanks to Thomas Daede)

 2. Add m68k support (thanks to Thomas Daede).

 3. Fix build issues with ARM Cortex-a9 target (thanks to Ilia
    Sergachev).

 4. Fix fwrite(x,0,y,z) in both tinystdio and legacy stdio. tinystdio
    returned the wrong value and legacy stdio caused a divide-by-zero
    fault.

 5. Update Ryu code to match upstream (minor fixes)

 6. Fix various __NEWLIB and __PICOLIBC macros; they were using a
    single leading underscore instead of two (thanks to Vincent
    Palatin).

 7. Fix tinystdio error-handling bugs

 8. Merge recent newlib changes (fixed ltdoa in legacy stdio)

 9. Speed improvements for github CI system

 10. Big-endian PowerPC support

 11. Fail builds if most 'run_command' uses fail (thanks to Johan de
     Claville Christiansen)

 12. Positional parameters in tinystdio. With this, I think tinystdio
     is feature complete.

 13. Support for multiple build-styles of picolibc (minsize/release)
     in one binary package. This still requires separate meson runs.

 14. Testing with glibc test code. This uncovered numerous bugs,
     mostly math errno/exception mistakes, but also a few serious
     bugs, including a couple of places where the nano-malloc failed
     to check for out-of-memory. Picolibc now passes all of the glibc
     math tests except for jn, yn, lgamma and tgamma. The picolibc
     versions of those functions are too inaccurate. Picolibc also
     passes most other relevant glibc tests, including stdio,
     string and stdlib areas.

 15. Tinystdio version of fcvt now has a static buffer large enough to
     hold the maximum return size.

 16. Tinystdio versions of ecvtbuf and fcvtbuf have been replaced
     with ecvt_r and fcvt_r equivalents, which take a 'len' parameter
     to prevent buffer overruns.

 17. Add the GeneratePicolibcCrossFile.sh script which provides a way
     to isolate picolibc build scripts from the vagaries of meson
     version shifts (thanks to R. Diez).

 18. Add 'semihost' version of crt0 that calls 'exit' after main
     returns. The ARM and RISC-V versions of this also include trap
     handlers for exceptions that print out information and exit when
     an exception occurs.

### Picolibc version 1.7.4

 1. Clean up meson build bits, including use of 'fs module (thanks to
    Yasushi Shoji).

 2. Speed up github actions by sharing Debian docker image (thanks to
    Yasushi Shoji).

 3. Reduce use of intermediate static libraries during build

 4. Use standard Meson architecture names everywhere (thanks to
    Yasushi Shoji).

 5. Support building with -D_FORTIFY_SOURCE enabled.

 6. Clean up 32-bit arm assembly code, eliminating __aeabi wrappers
    where possible.

 7. Add basename, dirname and fnmatch back.

 8. Fix all old-style (K&R) function definitions.

 9. Enable lots more compiler warning flags.

 10. Remove last uses of alloca in legacy stdio code.

 11. Add tests from musl libc-testsuite. There aren't many tests, but
     these identified a few bugs.

 12. Add lots more exception and errno tests for the math functions.

 13. Restructure math library to always use the `__math_err` functions
     to raise exceptions and set errno. This removes the w_*.c wrapper
     functions and eliminates the `__ieee names`. This centralizes
     compiler work-arounds to ensure run-time evaluation of
     expressions intended to raise exceptions. In the process, all of
     the libm/math files were reformatted with clang-format.

 14. Make tinystdio '%a' compatible with glibc, including supporting
     rounding and trimming trailing zeros when possible.

 15. Remove floating point exception generation code on targets
     without floating point exception support. This reduces code size
     on soft float machines without affecting results.

### Picolibc version 1.7.3

 1. Add -Wall -Wextra to default builds. Fixed warnings this raised.

 2. Add htonl and friends (based on __htonl). Thanks to Johan de
    Claville Christiansen

 3. Set errno in scalbn and scalbnf (patch forwarded to newlib).

 4. Merge newlib recent changes which includes a couple of libm fixes.

### Picolibc version 1.7.2

 1. Fix picolibc.ld to split C++ exceptions back apart (thanks to
    Khalil Estell)

 2. Add vsscanf to tinystdio (required for libstdc++).

 3. Also stick -isystem in C++ compile command to try and get
    picolibc headers to be used instead of newlib.

### Picolibc version 1.7.1

 1. Add __cxa_atexit implementation to 'picoexit' path as required by
    C++

 2. Fix lack of 'hh' support in integer-only tinystdio printf path.

 3. Fix tinystdio __file flag initialization for C++ apps

### Picolibc version 1.7

 1. Merge libc and libm into a single library. Having them split
    doesn't offer any advantages while requiring that applications add
    '-lm' to link successfully. Having them merged allows use of libm
    calls from libc code.

 2. Add hex float format to *printf, *scanf and strto{d,f,ld}. This is
    required for C99 support.

 3. Unify strto{d,f,ld} and *scanf floating point parsing code. This
    ensures that the library is consistent in how floats are parsed.

 4. Make strto{d,f,ld} set errno to ERANGE on overflow/underflow,
    including when the result is a subnormal number.

### Picolibc version 1.6.2

 1. Change `restrict` keyword in published headers to `__restrict` to
    restore compatibility with applications building with --std=c18.

 2. Additional cleanups in time conversion funcs (Thanks to R. Riez)

### Picolibc version 1.6.1

 1. Code cleanups for time conversion funcs (Thanks to R. Diez)

 2. Add '-fno-stack-protector' when supported by the C compiler
    to avoid trouble building with native Ubuntu GCC.

 3. Bug fix for converting denorms with sscanf and strto{d,f,ld}.

 4. Use __asm__ for inline asm code to allow building applications
    with --std=c18

 5. Fix exit code for semihosting 'abort' call to make it visible
    to the hosting system.

 6. Add strfromf and strfromd implementations. These are simple
    wrappers around sscanf, but strfromf handles float conversions
    without requiring a pass through 'double' or special linker hacks.

### Picolibc version 1.6

 1. Bugfix for snprintf(buf, 0) and vsnprintf(buf, 0) to avoid
    smashing memory

 2. Support building libstdc++ on top of picolibc

 3. Add 'hosted' crt0 variant that calls exit when main
    returns. This makes testing easier without burdening embedded apps
    with unused exit processing code.

 4. Add 'minimal' crt0 variant that skips constructors to
    save space on systems known to not use any.

 5. Fix HW floating point initialization on 32-bit ARM processors to
    perform 'dsb' and 'isb' instructions to ensure the FPU enabling
    write is complete before executing any FPU instructions.

 6. Create a new '--picolibc-prefix' GCC command line parameter that
    sets the base of all picolibc file names.

 7. Add bare-metal i386 and x86_64 initializatiton code (thanks to
    Mike Haertel). These initalize the processor from power up to
    running code without requiring any BIOS.

 8. Merge newlib as of late April, 2021

 9. Add 'timegm' function (thanks to R. Diez).

10. Fix a number of tinystdio bugs: handle fread with size==0, parse
    'NAN' and 'INF' in fscanf in a case-insensitive manner, fix
    negative precision to '*' arguments in printf, fix handling of
    'j', 'z' and 't' argument size specifiers (thanks to Sebastian
    Meyer).

11. Make the fenv API more consistent and more conformant with the
    spec. All architectures now fall back to the default code
    for soft float versions, which avoids having the various exception
    and rounding modes get defined when not supported.

### Picolibc version 1.5.1

 1. Make riscv crt0 '_exit' symbol 'weak' to allow linking without
    this function.

### Picolibc version 1.5

 1. Make picolibc more compatible with C++ compilers.
   
 2. Add GCC specs file and linker script for building C++ applications
    with G++ that enable exception handling by linking in call stack
    information.

 3. A few clang build fixes, including libm exception generation

 4. Nano malloc fixes, especially for 'unusual' arguments

 5. Merge in newlib 4.1.0 code

 6. More libm exception/errno/infinity fixes, mostly in the gamma funcs.

 7. Add tests for all semihost v2.0 functions.

 8. A few RISC-V assembly fixes and new libm code.

 9. Build fixes to reliably replace generic code with
    architecture-specific implementations.

With a patch which is pending for GCC 11, we'll be able to build C++
applications that use picolibc with exceptions and iostream.

### Picolibc version 1.4.7

 1. Fix numerous libm exception and errno bugs. The math functions are
    all now verified to match the C19 and Posix standards in this
    area.

 2. Change behavior of 'gamma' function to match glibc which returns
    lgamma for this function. Applications should not use this
    function, they should pick either lgamma or tgamma as appropriate.
 
 3. Fix fma/fmaf on arm and RISC-V so that the machine-specific versions
    are used when the hardware has support. Also fix the math library
    to only use fma/fmaf when it is supported by the hardware.

 4. Fix numerous nano-malloc bugs, especially with unusual parameters.

 5. Change nano-malloc to always clear returned memory.

 6. Improve nano-realloc to perform better in various ways, including
    merging adjacent free blocks and expanding the heap.

 7. Add malloc tests, both a basic functional test and a stress test.

 8. Improve build portability to Windows. Picolibc should now build
    using mingw.

 9. Use hardware TLS register on ARM when available.

 10. Support clang compiler. Thanks to Denis Feklushkin
     <denis.feklushkin@gmail.com> and Joakim Nohlgård <joakim@nohlgard.se>.

 11. Avoid implicit float/double conversions. Check this by having
     clang builds use -Wdouble-promotion -Werror=double-promotion
     flags

 12. Have portable code check for machine-specific overrides by
     matching filenames. This avoids building libraries with
     duplicate symbols and retains compatibility with newlib (which
     uses a different mechanism for this effect).

 13. Patches to support building with [CompCert](http://compcert.inria.fr/), a
     formally verified compiler. Thanks to Sebastian Meyer
     <meyer@absint.com>.

### Picolibc version 1.4.6

 1. Install 'ssp' (stack smashing protection) header files. This fixes
    compiling with -D_FORTIFY_SOURCE.

 2. Make getc/ungetc re-entrant. This feature, which is enabled by
    default, uses atomic instruction sequences that do not require
    OS support.

 3. Numerous iconv fixes, including enabling testing and switching
    external CCS file loading to use stdio. By default, iconv provides
    built-in CCS data for all of the supported encodings, which takes
    a fairly large amount of read-only memory. Iconv is now always
    included in picolibc as  it isn't included in applications unless
    explicitly referenced by them.

 4. Add __getauxval stub implementation to make picolibc work with
    GCC version 10 compiled for aarch64-linux-gnu.

 5. Change how integer- and float- only versions of printf and scanf
    are selected. Instead of re-defining the symbols using the C
    preprocessor, picolibc now re-defines the symbols at link
    time. This avoids having applications compiled with a mixture of
    modes link in multiple versions of the underlying functions, while
    still preserving the smallest possible integer-only
    implementation.

 6. Document how to use picolibc on a native POSIX system for
    testing. Check out the [os.md](doc/os.md) file for details.

 7. Merge current newlib bits in. This includes better fenv support,
    for which tests are now included in the picolibc test suite.

### Picolibc version 1.4.5

 1. Fix section order in picolibc.ld to give applications correct
    control over the layout of .preserve, .init and .fini regions.

 2. Add startup and TLS support for aarch64 and non Cortex-M 32-bit
    arm.

### Picolibc version 1.4.4

 1. Fix floating point 'g' format output in tinystdio. (e.g.,
    for 10.0, print '10' instead of '1e+01'). There are tests which
    verify a range of 'g' cases like these now.
    
 2. Merge current newlib bits. The only thing which affects picolibc
    is the addition of fenv support for arm.

### Picolibc version 1.4.3

 1. Make fix for CVE 2019-14871 - CVE 2019-14878 in original newlib
    stdio code not call 'abort'. Allocation failures are now reported
    back to the application.

 2. Add 'exact' floating point print/scan code to tinystdio. Thanks
    to Sreepathi Pai for pointing me at the Ryu code by Ulf
    Adams.

 3. Add regular expression functions from newlib. These were removed
    by accident while removing POSIX filesystem-specific code.

 4. Make tinystdio versions of [efg]cvt functions. This means that the
    default tinystdio version of picolibc no longer calls malloc from
    these functions.    

 5. More clang-compatibility fixes. (Thanks to Denis Feklushkin)

 6. Remove stdatomic.h and tgmath.h. (they should not be provide by picolibc)

### Picolibc version 1.4.2

 1. Clang source compatibility. Clang should now be able to compile
    the library. Thanks to Denis Feklushkin for figuring out how
    to make this work.

 2. aarch64 support. This enables the existing aarch64 code and
    provides an example configuration file for getting it
    built.  Thanks for Anthony Anderson for this feature.

 3. Testing on github on push and pull-request. For now, this is
    limited to building the library due to a bug in qemu.

 4. Get newlib stdio working again. You can now usefully use Newlib's
    stdio. This requires a working malloc and is substantially larger
    than tinystdio, but has more accurate floating point input. This
    requires POSIX functions including read, write and a few others.

 5. Fix long double strtold. The working version is only available
    when using tinystdio; if using newlib stdio, strtold is simply not
    available.

 6. Improve tinystdio support for C99 printf/scanf additions.

 7. Check for correct prefix when sysroot-install option is
    selected. The value of this option depends on how gcc was
    configured, and (alas) meson won't let us set it at runtime, so
    instead we complain if the wrong value was given and display the
    correct value.

 8. Sync up with current newlib head.

### Picolibc version 1.4.1

This release contains an important TLS fix for ARM along with a few
minor compatibility fixes

 1. Make __aeabi_read_tp respect ARM ABI register requirements to
    avoid clobbering register contents during TLS variable use.

 2. Use cpu_family instead of cpu in meson config, which is 'more
    correct' when building for a single cpu instead of multilib.

 3. Make arm sample interrupt vector work with clang

 4. Use __inline instead of inline in published headers to allow
    compiling with -ansi

 5. Make 'naked' RISC-V _start function contain only asm
    statements as required by clang (and recommended by gcc).

 6. Use -msave-restore in sample RISC-V cross-compile
    configuration. This saves text space.

### Picolibc version 1.4

This release was focused on cleaning up the copyright and license
information.

 1. Copyright information should now be present in every source file.

 2. License information, where it could be inferred from the
    repository, was added to many files.

 3. 4-clause BSD licenses were changed (with permission) to 3-clause

 4. Fix RISC-V ieeefp.h exception bits

 5. Merge past newlib 3.2.0

 6. Add PICOLIBC_TLS preprocessor define when the library has TLS support

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
    space definitions. These re-definitions would fail and the default
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

