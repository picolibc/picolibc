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
application. The selection is done using compiler command line
options, `--printf` and `--scanf` which map the public printf and
scanf names to internal names in the linker. These both take one of
the available variants as the option value:
	
	cc --printf=i --scanf=i

The function name mapping happens in picolibc.specs file which uses
these options to add --defsym options when linking. These options also
set a preprocessor symbol which handles the 'f' variant, so you need
to pass these options during both compilation and linking.

Using the defsym approach, rather than renaming the functions with the
C preprocessor has a couple of benefits:

 * Printf uses within picolibc (which are all integer-only) now share
   whichever printf the application needs, avoiding including both
   integer-only and float versions

 * Printf optimizations in the compiler which map to puts or fputs now
   work even when using integer-only or float printf functions.

Make sure you use only the option associated with the desired function
as --defsym will force the underlying code to be included in the
output even if unused.

The defsym approach does not work with link-time optimization. In that
case, applications can only use the default. The library must be
compiled with the desired variant as the default (using the
`-Dformat-default` configuration option).

## Printf and Scanf levels in Picolibc

There are five levels of printf support provided by Picolibc that can
be selected when building applications. One of these is the default
used when no symbol definitions are applied; that is selected using
the picolibc built-time option, `-Dformat-default`, which defaults to
`double`. The picolibc specs for GCC includes support for --printf and
--scanf options which set both the preprocessor defines as well as
configuring the linker to alias vfprintf and vfscanf to the desired
function using the --defsym linker command line option. For instance,
if you use cc --printf=d --scanf=l, the compiler will see

	-D_PICOLIBC_PRINTF='d' -D_PICOLIBC_SCANF='l'

and the linker will see

	--defsym=vfprintf=__d_vfprintf --defsym=vfscanf=__l_vfscanf

A linker that supports -alias instead of --defsym would see

	-alias,__d_vfprintf,vfprintf -alias,__l_vfscanf,vfscanf

When not using the picolibc specs, you can manually specify these on
the compiler command line

These are listed in order of decreasing functionality and
size.

 * 'd' format (default when `-Dformat-default=d`). This offers full printf
   functionality, including both float and double conversions along
   with the C99 extensions and POSIX positional parameters. There is
   optional support for the C23 %b format specifier via the
   `io-percent-b` setting and optional support for long double values
   via the `io-long-double` setting.

 * 'f' format (default when `-Dformat-default=f`). This is the
   same as 'd' format except that it supports only float for scanf and
   supports float instead of double for printf. It also doesn't
   support long double conversions.

 * 'l' format (default when `-Dformat-default=l`). This
   removes support for all float and double conversions and makes
   support for C99 extensions and POSIX positional parameters optional
   via the `io-c99-formats` and `io-pos-args` settings.

 * 'i' format (default when `-Dformat-default=i`). This removes
   support for long long conversions where those values are larger
   than long values.

 * 'm' format (default when`-Dformat-default=m`). This removes support
   for width and precision, alternate presentation modes and alternate
   sign presentation. All of these are still parsed correctly, so
   applications needn't adjust format strings in most cases, but the
   output will not be the same. It also disables any support for
   positional arguments and %b formats that might be selected with
   `io-pos-args` or `io-percent-b`.

'f' format requires a special macro for float values:
`printf_float`. To make it easier to switch between that and the
default level, that macro is also correctly defined for the other two
levels.

Here's an example program to experiment with these options:

```c
#include <stdio.h>

void main(void) {
	printf(" 2⁶¹ = %lld π ≃ %.17g\n", 1ll << 61, printf_float(3.141592653589793));
}
```

Now we can build and run it with the double options:

	$ arm-none-eabi-gcc --printf=d -Os -march=armv7-m --specs=picolibc.specs --oslib=semihost --crt0=hosted -Wl,--defsym=__flash=0 -Wl,--defsym=__flash_size=0x00200000 -Wl,--defsym=__ram=0x20000000 -Wl,--defsym=__ram_size=0x200000 -o printf.elf printf.c
	$ arm-none-eabi-size printf.elf
	   text	   data	    bss	    dec	    hex	filename
           7748	     80	   4104	  11932	   2e9c	printf.elf
	$ qemu-system-arm -chardev stdio,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine mps2-an385,accel=tcg -kernel printf.elf -nographic
	 2⁶¹ = 2305843009213693952 π ≃ 3.141592653589793

Switching to float-only reduces the size but lets this still work,
although the floating point value has reduced precision:

	$ arm-none-eabi-gcc --printf=f -Os -march=armv7-m --specs=picolibc.specs --oslib=semihost --crt0=hosted -Wl,--defsym=__flash=0 -Wl,--defsym=__flash_size=0x00200000 -Wl,--defsym=__ram=0x20000000 -Wl,--defsym=__ram_size=0x200000 -o printf-float.elf printf.c
	$ arm-none-eabi-size printf-float.elf
	   text	   data	    bss	    dec	    hex	filename
	   6284	     80	   4104	  10468	   28e4	printf-float.elf
	$ qemu-system-arm -chardev stdio,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine mps2-an385,accel=tcg -kernel printf-float.elf -nographic
	 2⁶¹ = 2305843009213693952 π ≃ 3.1415927

Selecting the long-long variant reduces the size further, but now the
floating point value is not displayed correctly:

	$ arm-none-eabi-gcc --printf=l -Os -march=armv7-m --specs=picolibc.specs --oslib=semihost --crt0=hosted -Wl,--defsym=__flash=0 -Wl,--defsym=__flash_size=0x00200000 -Wl,--defsym=__ram=0x20000000 -Wl,--defsym=__ram_size=0x200000 -o printf-long-long.elf printf.c
	$ arm-none-eabi-size printf-long-long.elf
	   text	   data	    bss	    dec	    hex	filename
	   2188	     80	   4104	   6372	   18e4	printf-long-long.elf
	$ qemu-system-arm -chardev stdio,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine mps2-an385,accel=tcg -kernel printf-long-long.elf -nographic
	 2⁶¹ = 2305843009213693952 π ≃ *float*

Going to integer-only reduces the size even further, but now it
doesn't output the long long value correctly:

	$ arm-none-eabi-gcc --printf=i -Os -march=armv7-m --specs=picolibc.specs --oslib=semihost --crt0=hosted -Wl,--defsym=__flash=0 -Wl,--defsym=__flash_size=0x00200000 -Wl,--defsym=__ram=0x20000000 -Wl,--defsym=__ram_size=0x200000 -o printf-int.elf printf.c
	$ arm-none-eabi-size printf-int.elf
	   text	   data	    bss	    dec	    hex	filename
	   2028	     80	   4104	   6212	   1844	printf-int.elf
	$ qemu-system-arm -chardev stdio,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine mps2-an385,accel=tcg -kernel printf-int.elf -nographic
	 2⁶¹ = 0 π ≃ *float*

To shrink things still further, use the 'minimal' variant. This
doesn't even look at the %g formatting instruction, so that value
displays as '%g'.

	$ arm-none-eabi-gcc --printf=m -Os -march=armv7-m --specs=picolibc.specs --oslib=semihost --crt0=hosted -Wl,--defsym=__flash=0 -Wl,--defsym=__flash_size=0x00200000 -Wl,--defsym=__ram=0x20000000 -Wl,--defsym=__ram_size=0x200000 -o printf-min.elf printf.c
	$ arm-none-eabi-size printf-min.elf
	   text	   data	    bss	    dec	    hex	filename
	   1508	     80	   4104	   5692	   163c	printf-min.elf
	$ qemu-system-arm -chardev stdio,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine mps2-an385,accel=tcg -kernel printf-min.elf -nographic
	 2⁶¹ = 0 π ≃ %g

There's a build-time option available that enables long-long support
in the minimal printf variant, `-Dminimal-io-long-long=true`. Building with
that increases the size modestly while fixing the long long output:

	$ arm-none-eabi-size printf-min.elf
	   text	   data	    bss	    dec	    hex	filename
	   1632	     80	   4104	   5816	   16b8	printf-min.elf
	$ qemu-system-arm -chardev stdio,id=stdio0 -semihosting-config enable=on,chardev=stdio0 -monitor none -serial none -machine mps2-an385,accel=tcg -kernel printf-min.elf -nographic
	 2⁶¹ = 2305843009213693952 π ≃ %g

## Picolibc build options for stdio

In addition to the application build-time options, picolibc includes a
number of picolibc build-time options to control the feature set (and
hence the size) of the library:

 * `-Dio-c99-formats=true` This option controls whether support for
   the C99 type-specific format modifiers 'j', 'z' and 't' and the hex
   float format 'a' are included in the long-long, integer and minimal
   printf and scanf variants. Support for the C99 format specifiers
   like PRId8 is always provided.  This option is enabled by default.

 * `-Dio-long-long=true` This deprecated option controls whether
   support for long long types is included in the integer variant of
   printf and scanf. Instead of using this option, applications should
   select the long long printf and scanf variants. This option is
   disabled by default.

 * `-Dio-pos-args=true` This option add support for C99 positional
   arguments to the long long and integer printf and scanf variant
   (e.g. "%1$"). Positional arguments are always supported in the
   double and float variants and never supported in the minimal
   variant. This option is disabled by default.

 * `-Dio-long-double=true` This option add support for long double
   parameters. That is limited to systems using 80- and 128- bit long
   doubles, or systems for which long double is the same as
   double. This option is disabled by default

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

 * `-Dposix-console=true` This option provides stdin/stdout/stderr
   support using the picolibc POSIX I/O support.

 * `-Dformat=default={d,f,l,i,m}` Selects which printf and scanf
   variant is used by default.

 * `-Dprintf-aliases=true` This option, which is enabled by default,
   provides for linker alias support for the --printf and --scanf
   compiler options.

 * `-Dio-percent-b=true` This option, which is disabled by default,
   supports the C23 %b format specifier.

 * `-Dprintf-small-ultoa=true` This option, which is enabled by
   default, switches printf's binary-to-decimal conversion code to a
   version which avoids soft division for values larger than the
   machine registers as those functions are often quite large and
   slow. Applications using soft division on large values elsewhere
   will save space by disabling this option as that avoids including
   custom divide-and-modulus-by-ten implementations.

 * `-Dprintf-percent-n=true` This option, which is disabled by default,
   provides support for the dangerous %n printf format specifier.

 * `-Dminimal-io-long-long=true` This option controls whether support
   for long long types is included in the minimal variant of printf
   and scanf. This option is disabled by default.

 * `-Dfast-bufio=true` This option directly calls the read and write
   hooks from fread and fwrite when interacting with buffered streams.

 * `-Dio-wchar=true` This option enables wide character input and
   output even when picolibc is built without multi-byte character
   support.

Picolibc also allows use of the legacy newlib stdio library, but that
is outside the scope of this documentation.
