# Picolibc and Operating Systems

Picolibc is designed to be operating-system independent, so it doesn't
embed any operating system support into libc. Some portions of
Picolibc need system support, like I/O and task termination.

## System Interfaces used by Picolibc

Here's the full list of system functions used by Picolibc, split into
sections based on what libc support they enable

### stdin/stdout/stderr

Picolibc stdio splits support for simple console input/output and more
complex file operations so that a minimal system can easily support
the former with only a few functions.

To get stdin/stdout/stderr working, the application needs to define
the `stdin`, `stdout` and `stderr` globals, which contain pointers to
FILE objects. The pointers may reside in read-only memory, but the
FILE objects may not. A single FILE object may be used for all three
pointers, and linker aliases may be used to make all three pointers be
stored in the same location. The FILE object contains function
pointers for putc, getc, which might be defined as follows:

	static int
	sample_putc(char c, FILE *file)
	{
		(void) file;		/* Not used in this function
		__uart_putc(c);		/* Defined by underlying system */
		return c;
	}

	static int
	sample_getc(FILE *file)
	{
		unsigned char c;
		(void) file;		/* Not used in this function */
		c = __uart_getc();	/* Defined by underlying system */
		return c;
	}

It also contains a pointer to an optional `flush` function, which, if
provide, will be called to flush pending output to the file, e.g.,
when the `fflush()` function is called:

	static int
	sample_flush(FILE *file)
	{
		/* This function doesn't need to do anything */
		(void) file;		/* Not used in this function */
		return 0;
	}

These functions are used to initialize a FILE structure:

	static FILE __stdio = FDEV_SETUP_STREAM(sample_putc,
						sample_getc,
						NULL,
						_FDEV_SETUP_RW);

This defines a FILE which can read and write characters using the putc
and getc functions described above, but not using any flush
function. The final paramter, which specifies the operations
supported, can be one of the following:

| Mode              | Operations | Required Functions |
|-------------------|------------|--------------------|
| _FDEV_SETUP_READ  | Read       | getc               |
| _FDEV_SETUP_WRITE | Write      | putc               |
| _FDEV_SETUP_RW    | Read/Write | putc, getc         |

Finally, the FILE is used to initialize the `stdin`, `stdout` and
`stderr` values, the latter two of which are simply aliases to `stdin`:

   FILE *const stdin = &__stdio;
   __strong_reference(stdin, stdout);
   __strong_reference(stdin, stderr);

### fopen, fdopen

Support for these requires malloc/free along with a handful of
POSIX-compatible functions:

	int open (const char *, int, ...);
	int close (int fd);
	ssize_t read (int fd, void *buf, size_t nbyte);
	ssize_t write (int fd, const void *buf, size_t nbyte);
	off_t lseek (int fd, off_t offset, int whence);

The code needed for this is built into Picolibc by default, but can be
disabled by specifying `-Dposix-io=false` in the meson command line.

### exit

Exit is just a wrapper around _exit that also calls destructors and
callbacks registered with atexit. To make it work, you'll need to
implement the `_exit` function:

	void	_exit (int status) _ATTRIBUTE ((__noreturn__));

### malloc and free

Both versions of malloc in picolibc require sbrk to be supported. The
smaller version, enabled (by default) with -Dnewlib-nano-malloc=true,
can handle sbrk returning dis-continuous memory while the larger
version (enabled with -Dnewlib-nano-malloc=false) requires sbrk return
contiguous chunks of memory.

### sbrk

Picolibc includes a simple version of sbrk that can return chunks of
memory from a pre-defined contiguous heap. To use this function, your
application linking process needs to define two symbols:

 * __heap_start — points at the start of the heap available for sbrk.
 * __heap_end — points at the end of the heap available for sbrk.

The sample linker script provided with picolibc defines these two
symbols to enclose all RAM which is not otherwise used by the
application.

## abort

Posix says that `abort` sends `SIGABRT` to the calling process as if
the process called `raise(SIGABRT)`. It also says `abort` shall not
return. The picolibc implementation of `abort` calls `raise`; if that
returns, it then calls `_exit`. The picolibc version of `raise` also
calls `_exit` for uncaught and un-ignored signals.

This means that an application needs to provide an implementation of
`_exit` to support `abort`.

## Linking with System Library

To get Picolibc to use a system library, that library needs to be
specified *after* libc on the linker command line. The picolibc.specs
file provides a way to specify a library after libc using the
`--oslib=` parameter:

	$ gcc -o program.elf program.o --oslib=myos

This will include -lmyos after -lc so that the linker can resolve
functions used by picolibc from libmyos.a. You can, alternatively,
include the functions in object files with the rest of your
application, which avoids the problem with libraries. Note that this
mechanism requires the definition of _exit in the myos library.

## Semihosting support

For RISC-V and ARM processors, Picolibc provides an implementation of
all of the above functions along with a couple more POSIX APIs used by
the Picolibc test suite. This implementation relies on semihosting
support from the execution environment, which is available when
running under qemu or when using openocd and gdb. Link this into your
application using `--oslib=semihost` in your link command line.
Please note that this will replace the default `crt0` with a variant
calling `exit` upon return from `main`. The default is to enter an
infinite loop, and the change ensure a clean return to the execution
environment.

## POSIX console support

As a build-time option, Picolibc can be configured to use POSIX read
and write APIs to support stdin, stdout and stderr. Add
`-Dposix-console=true` to enable this. This is incompatible with
semihosting support above.

## Building picolibc on native POSIX systems

To allow for testing of picolibc and applications using picolibc, you
can actually build picolibc on a full POSIX system. In this
configuration, picolibc provides the non-POSIX libc APIs while the
underlying system C library is used for the POSIX functions described
above. To build in this mode, you'll need to override a few default
picolibc configuration parameters:

	$ meson \
		-Dtls-model=global-dynamic \
		-Dmultilib=false \
		-Dpicolib=false \
		-Dpicocrt=false \
		-Dposix-console=true \
		-Dnewlib-global-atexit=true \
		-Dincludedir=lib/picolibc/include \
		-Dlibdir=lib/picolibc/lib \
		-Dspecsdir=none

 * -Dtls-model=global-dynamic makes picolibc use the default TLS model
   for GCC.

 * -Dmultilib-false makes picolibc build only a single library for the
   default GCC configuration.

 * -Dpicolib=false disables building the TLS and sbrk support built-in
   to picolibc so that the underlying system support is used instead.

 * -Dpicocrt=false disables building the C startup code as that is
   provided by the underlying system.

 * -Dposix-console=true uses POSIX I/O read/write APIs for stdin,
    stdout and stderr.

 * -Dnewlib-global-atexit=true disables the per-thread atexit behavior
   so that picolibc acts like a regular C library.

 * -Dincludedir and -Dlibdir specify install locations for the headers
   and library

 * -Dspecsdir=none disables installing picolibc.specs as that file
   is not useful in this environment

Once built, you can install and use picolibc on the host:

	$ cc -I/usr/local/lib/picolibc/include hello-world.c \
		/usr/local/lib/picolibc/lib/libc.a
