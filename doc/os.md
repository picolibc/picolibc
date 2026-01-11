# Picolibc and Operating Systems

Picolibc is designed to be operating-system independent, so it doesn't
embed any operating system support into libc. Some portions of
Picolibc need system support, like I/O and task termination. Picolibc
expects these interfaces to conform to POSIX, both as a way to
simplify integration into POSIX-conforming operating system and as a
way to provide a well defined interface between picolibc and the
target system.

When using picolibc in a non-POSIX environment, applications may
constrain themselves to portions of picolibc which do not rely upon
these functions, provide wrapping functions which expose POSIX apis on
top of their native implementation or replace portions of picolibc
with code designed atop their interfaces.

## stdin/stdout/stderr

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

```c
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
```

It also contains a pointer to an optional `flush` function, which, if
provide, will be called to flush pending output to the file, e.g.,
when the `fflush()` function is called:

```c
static int
sample_flush(FILE *file)
{
	/* This function doesn't need to do anything */
	(void) file;		/* Not used in this function */
	return 0;
}
```

These functions are used to initialize a FILE structure:

```c
static FILE __stdio = FDEV_SETUP_STREAM(sample_putc,
					sample_getc,
					NULL,
					_FDEV_SETUP_RW);
```

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

```c
FILE *const stdin = &__stdio;
__strong_reference(stdin, stdout);
__strong_reference(stdin, stderr);
```

## Operating System Interfaces used by Picolibc

Here's the full list of external operating system functions used by
Picolibc and which source files reference them. 

 * int close(int fd)

   + libc/search/hash.c
   + libc/stdio/fdopen.c
   + libc/stdio/fopen.c
   + libc/stdio/freopen.c
   + libc/stdio/mktemp.c
   + libc/stdio/tmpfile.c

 * void _exit(int status)

   + libc/signal/signal.c
   + libc/stdlib/abort.c
   + libc/stdlib/exit.c
   + libc/stdlib/_Exit.c

 * int fstat(int fd, struct stat *statbuf)

   + libc/search/hash.c

 * int getentropy(void *buffer, size_t length)

   + libc/ssp/stack/protector.c
   + libc/stdlib/arc4random.c

 * int gettimeofday(struct timeval *tv, sruct timezone *tz)

   + libc/time/time.c

 * off_t lseek(int fd, off_t offset, int whence)

   + libc/search/hash.c
   + libc/search/hash_page.c
   + libc/stdio/fdopen.c
   + libc/stdio/freopen.c
   + libc/xdr/xdr_rec.c

 * int open(char *path, int flags, ...)

   + libc/search/hash.c
   + libc/stdio/fopen.c
   + libc/stdio/freopen.c
   + libc/stdio/mktemp.c

 * ssize_t read(int fd, void *buf, size_t count)

   + libc/search/hash.c
   + libc/search/hash_page.c
   + libc/stdio/fdopen.c
   + libc/stdio/freopen.c

 * int sigprocmask(int how, sigset_t *set, sigset_t *oldset)

   + libc/search/hash_page.c

 * int stat(char *path, struct stat *statbuf)

   + libc/search/hash.c

 * clock_t times(struct tms *buf)

   + libc/time/clock.c

 * int unlink(char *path)

   + libc/search/hash_page.c
   + libc/stdio/remove.c
   + libc/stdio/tmpfile.c

 * ssize_t write(int fd, void *buf, size_t count)

   + libc/search/hash.c
   + libc/search/hash_page.c
   + libc/stdio/fdopen.c
   + libc/stdio/freopen.c
   + libc/stdio/vdprintf.c

### fopen, fdopen and freopen

Support for these requires malloc/free along with a handful of
POSIX-compatible functions:

```c
int     close (int fd);
off_t   lseek (int fd, off_t offset, int whence);
int     open (const char *, int, ...);
ssize_t read (int fd, void *buf, size_t nbyte);
ssize_t write (int fd, const void *buf, size_t nbyte);
```

### dprintf, vdprintf

These functions directly operate on file descriptors, so they use
`write`:

```c
ssize_t write (int fd, const void *buf, size_t nbyte);
```

### mktemp, mkstemp, mkostemp, mkstemps, mkostemps, tmpnam and tmpfile

All of these functions share a common implementation which operates on
file descriptors, so they all need a selection of POSIX functions:

```c
int close (int fd);
int open (const char *, int, ...);
int unlink(char *path)
```

### exit and _Exit

`_Exit` is a trivial wrapper around `_exit`; `exit` is a wrapper around
`_exit` that also calls destructors and callbacks registered with `atexit`
or `on_exit`. To make these work, you'll need to implement the `_exit`
function:

```c
void _exit (int status);
```

### malloc, free (and friends)

Picolibc's `malloc` implementation relies on `sbrk`. Malloc can handle
sbrk returning dis-continuous memory.

```c
void *sbrk(intptr_t incr);
```

### sbrk

Picolibc includes a simple version of sbrk that can return chunks of
memory from a pre-defined contiguous heap. You can either specify
`-Dinternal-heap=<size-in-bytes>` while building picolibc to declare a
fixed-size pre-allocated heap, or have the linker script define
symbols that provide the bounds of the heap:

 * `__heap_start` — points at the start of the heap available for sbrk.
 * `__heap_end` — points at the end of the heap available for sbrk.

The sample linker script provided with picolibc defines these two
symbols to enclose all RAM which is not otherwise used by the
application.

### abort and raise

Posix says that `abort` sends `SIGABRT` to the calling process as if
the process called `raise(SIGABRT)`. It also says `abort` shall not
return. The picolibc implementation of `abort` calls `raise`; if that
returns, it then calls `_exit`. The picolibc version of `raise` also
calls `_exit` for uncaught and un-ignored signals.

```c
__noreturn void _exit (int status);
```

### ndbm

Picolibc provides the BSD ndbm APIs and those operate directly via a
POSIX OS interface, not through stdio. Support for this API set
require all of the functions required by `fopen` along a few
additional ones:

```c
int     close (int fd);
off_t   lseek (int fd, off_t offset, int whence);
int     open (const char *, int, ...);
ssize_t read (int fd, void *buf, size_t nbyte);
int     sigprocmask(int how, sigset_t *set, sigset_t *oldset);
int     stat(char *path, struct stat *statbuf);
int     unlink(char *path);
ssize_t write (int fd, const void *buf, size_t nbyte);
```

### xdr

The Sun XDR APIs are generally abstracted away from OS interfaces
except that when using xdrrec_create. As that interface provides no
seek interface, the underlying implementation assumes that the
provided handle actually contains a POSIX file descriptor and directly
calls lseek for xdr_setpos and xdr_getpos. This provides compatibility
with glibc which does the same thing.

```c
off_t lseek(int fd, off_t offset, int whence);
```

### arc4random, arc4random_buf

To generate secure random numbers, picolibc's arc4random
implementation is seeded from system entropy:

```c
int getentropy(void *buffer, size_t length);
```

### clock

The clock API returns a total of all values returned from the times
API.

```c
clock_t times(struct tms *buf);
```

## Linking with System Library

To get Picolibc to use a system library, that library needs to be
specified *after* libc on the linker command line. The picolibc.specs
file provides a way to specify a library after libc using the
`--oslib=` parameter:

```console
$ gcc -o program.elf program.o --oslib=myos
```

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

```console
$ meson \
        -Dtls-model=global-dynamic \
        -Dmultilib=false \
	-Dpicocrt=false \
	-Dsemihost=false \
	-Duse-stdlib=true \
	-Dposix-console=true \
	-Dthread-local-storage=true \
	-Dthread-local-storage-api=false \
	-Dinternal-heap=8388608 \
	-Derrno-function=auto \
	-Dinitfini-array=false\
	-Dincludedir=lib/picolibc/include \
	-Dlibdir=lib/picolibc/lib \
	-Dspecsdir=none \
```

 * -Dtls-model=global-dynamic makes picolibc use the default TLS model
   for GCC.

 * -Dmultilib=false makes picolibc build only a single library for the
   default GCC configuration.

 * -Dpicocrt=false disables building the C startup code as that is
   provided by the underlying system.

 * -Dsemihost=false disables building semihosting support as OS
   interfaces are provided by the underlying system.

 * -Duse-stdlib=true removes -nostdlib from the compile and link
   command lines so that the native C library will be used.

 * -Dposix-console=true uses POSIX I/O read/write APIs for stdin,
    stdout and stderr.

 * -Dthread-local-storage=true and -Dthread-local-storage-api=false
   set up the library to enable TLS but not attempt to provide the
   picolibc-specific TLS apis as those don't work with the system C
   library.

 * -Dinternal-heap=8388608 allocates a stack chunk of memory for use
   by the picolibc `sbrk` implementation.

 * -Derrno-function=auto detects the native C library errno implementation.

 * -Dinitfini-array=false skips the picolibc constructor bits and lets
    the native C library do that initialization step.

 * -Dincludedir, -Dlibdir and -Dspecsdir set up the installation so it
   doesn't conflict with the native system.

Once built, you can install and use picolibc on the host:

```console
$ cc -I/usr/local/lib/picolibc/include hello-world.c \
	/usr/local/lib/picolibc/lib/libc.a
```
