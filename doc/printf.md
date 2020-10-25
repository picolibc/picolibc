# Printf and Scanf in Picolibc

Embedded systems are often tightly constrained on code space, which
makes it important to be able to only include code and data actually
needed by the application. The format-string based interface to
printf and scanf makes it very difficult to determine which
conversion operations might be needed by the application.

Picolibc handles this by providing multiple printf and scanf
implementations in the library with varying levels of conversion
support. The application developer selects among these at compile and
link time based on knowledge of the requirements of the
application. The selection is done using a compiler command line
definition of a preprocessor symbol which maps the public printf and
scanf names to internal names in the linker.

The function name mapping happens in picolibc.specs file which scans
the compiler command line looking for the preprocessor token and uses
that to add --defsym options when linking. This means the preprocessor
definition must be set on the command line and not in a file.

Using the defsym approach, rather than renaming the functions with the
C preprocessor has a couple of benefits:

 * Printf uses within picolibc (which are all integer-only) now share
   whichever printf the application needs, avoiding including both
   integer-only and float versions

 * Printf optimizations in the compiler which map to puts or fputs now
   work even when using integer-only or float printf functions.

Because the linker gets --defsym flags for both vfprintf and vfscanf,
those functions will always get included in the link process. To avoid
linking them into the application when they aren't otherwise needed,
picolibc.specs includes the --gc-sections linker flag. This causes
those functions to be discarded if they aren't used in the
application.

## Printf and Scanf levels in Picolibc

There are three levels of printf support provided by Picolibc that can
be selected when building applications:

 * Default level. This offers full printf functionality, including
   both float and double conversions. This is what you get with no
   pre-processor definition.

 * PICOLIBC_INTEGER_PRINTF_SCANF. This removes support for all float
   and double conversions. The picolibc.specs stanza that matches this
   option maps __i_vfprintf to vfprintf and __i_vfscanf to
   vfscanf. This is equivalent to adding this when linking your
   application:

	cc -Wl,--defsym=vfprintf=__i_vfprintf -Wl,--defsym=vfscanf=__i_vfscanf

   If you're using a linker that supports -alias instead of --defsym,
   you  would use:

	cc -Wl,-alias,___i_vfprintf,_vfprintf -Wl,-alias,___i_vfscanf,_vfscanf

 * PICOLIBC_FLOAT_PRINTF_SCANF. This provides support for float, but
   not double conversions. When picolibc.specs finds
   -DPICOLIBC_FLOAT_PRINTF_SCANF on the command line during linking,
   it maps __f_vfprintf to vfprintf and __f_vfscanf to vfscanf. This
   is equivalent to adding this when linking your application:

	cc -Wl,--defsym=vfprintf=__f_vfprintf -Wl,--defsym=vfscanf=__f_vfscanf

   If you're using a linker that supports -alias instead of --defsym,
   you  would use:

	cc -Wl,-alias,___f_vfprintf,_vfprintf -Wl,-alias,___f_vfscanf,_vfscanf

PICOLIBC_FLOAT_PRINTF_SCANF requires a special macro for float values:
`printf_float`. To make it easier to switch between that and the default
level, that macro is also correctly defined for the other two levels.

Here's an example program to experiment with these options:

	#include <stdio.h>

	void main(void) {
		printf(" 2⁶¹ = %lld π ≃ %.17g\n", 1ll << 61, printf_float(3.141592653589793));
	}

Now we can build and run it with the default options:

	$ riscv64-unknown-elf-gcc -Os -march=rv32imac -mabi=ilp32 --specs=picolibc.specs --oslib=semihost -Wl,--defsym=__flash=0x80000000 -Wl,--defsym=__flash_size=0x00200000 -Wl,--defsym=__ram=0x80200000 -Wl,--defsym=__ram_size=0x200000 -o printf.elf printf.c
	$ riscv64-unknown-elf-size printf.elf
	   text	   data	    bss	    dec	    hex	filename
	   8998	     24	      8	   9030	   2346	printf.elf
	$ qemu-system-riscv32 -chardev stdio,mux=on,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine virt,accel=tcg -kernel printf.elf -nographic -bios none
	 2⁶¹ = 2305843009213693952 π ≃ 3.141592653589793

Switching to float-only reduces the size but lets this still work,
although the floating point value has reduced precision:

	$ riscv64-unknown-elf-gcc -DPICOLIBC_FLOAT_PRINTF_SCANF -Os -march=rv32imac -mabi=ilp32 --specs=picolibc.specs --oslib=semihost -Wl,--defsym=__flash=0x80000000 -Wl,--defsym=__flash_size=0x00200000 -Wl,--defsym=__ram=0x80200000 -Wl,--defsym=__ram_size=0x200000 -o printf-float.elf printf.c
	$ riscv64-unknown-elf-size printf-float.elf
	   text	   data	    bss	    dec	    hex	filename
	   6214	     24	      8	   6246	   1866	printf-float.elf
	$ qemu-system-riscv32 -chardev stdio,mux=on,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine virt,accel=tcg -kernel printf-float.elf -nographic -bios none
	 2⁶¹ = 2305843009213693952 π ≃ 3.1415927

Going to integer-only reduces the size even further, but now it doesn't output
the values correctly:

	$ riscv64-unknown-elf-gcc -DPICOLIBC_INTEGER_PRINTF_SCANF -Os -march=rv32imac -mabi=ilp32 --specs=picolibc.specs --oslib=semihost -Wl,--defsym=__flash=0x80000000 -Wl,--defsym=__flash_size=0x00200000 -Wl,--defsym=__ram=0x80200000 -Wl,--defsym=__ram_size=0x200000 -o printf-int.elf printf.c
	$ riscv64-unknown-elf-size printf-int.elf
	   text	   data	    bss	    dec	    hex	filename
	   2266	     24	      8	   2298	    8fa	printf-int.elf
	$ qemu-system-riscv32 -chardev stdio,mux=on,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine virt,accel=tcg -kernel printf-int.elf -nographic -bios none
	 2⁶¹ = 0 π ≃ *float*

## Picolibc build options for printf and scanf options 

In addition to the application build-time options, picolibc includes a
number of picolibc build-time options to control the feature set (and
hence the size) of the library:

 * `-Dio-c99-formats=true` This option controls whether support for
   the C99 type-specific format modifiers 'j', 'z' and 't' is included
   in the library. Support for the C99 format specifiers like PRId8 is
   always provided.  This option is enabled by default.

 * `-Dio-long-long=true` This option controls whether support for long
   long types is included in the integer-only version of printf and
   scanf. long long support is always included in the full and
   float-only versions of printf and scanf. This option is disabled by
   default.

 * `-Dio-float-exact=true` This option, which is enabled by default,
   controls whether the tinystdio code uses exact algorithms for
   printf and scanf. When enabled, printing at least 9 digits
   (e.g. "%.9g") for 32-bit floats and 17 digits (e.g. "%.17g") for
   64-bit floats ensures that passing the output back to scanf will
   exactly re-create the original value.

 * `-Datomic-ungetc=true` This option, which is enabled by default,
   controls whether getc/ungetc use atomic instruction sequences to
   make them re-entrant. Without this option, multiple threads using
   getc and ungetc may corrupt the state of the input buffer.

For even more printf and scanf functionality, picolibc can be compiled
with the original newlib stdio code. That greatly increases the code
and data sizes of the library, including adding a requirement for heap
support in the run time system. Here are the picolibc build options for that code:

 * `-Dtinystdio=false` This disables the tinystdio code and uses
   original newlib stdio code.

 * `-Dnewlib-io-pos-args=true` This option add support for C99
   positional arguments (e.g. "%1$"). This option is disabled by default.

 * `-Dnewlib-io-long-double=true` This option add support long double
   parameters. That is limited to systems using 80- and 128- bit long
   doubles, or systems for which long double is the same as
   double. This option is disabled by default

 * `-Dnewlib-stdio64=true` This option changes the newlib stdio code
   to use 64 bit values for file sizes and offsets. It also adds
   64-bit versions of stdio interfaces which are defined with types
   which may be 32-bits (like 'long'). This option is enabled by default.
