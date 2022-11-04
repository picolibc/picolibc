/* Copyright (c) 2002, 2005, 2007 Joerg Wunsch
   All rights reserved.

   Portions of documentation Copyright (c) 1990, 1991, 1993
   The Regents of the University of California.

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  $Id: stdio.h 2527 2016-10-27 20:41:22Z joerg_wunsch $
*/

#ifndef _STDIO_H_
#define	_STDIO_H_ 1

#ifndef __ASSEMBLER__

#include <inttypes.h>
#include <stdarg.h>
#include <sys/_types.h>

#ifndef __DOXYGEN__
#define __need_NULL
#define __need_size_t
#include <stddef.h>
#endif	/* !__DOXYGEN__ */

/** \file */
/** \defgroup avr_stdio <stdio.h>: Standard IO facilities
    \code #include <stdio.h> \endcode

    <h3>Introduction to the Standard IO facilities</h3>

    This file declares the standard IO facilities that are implemented
    in \c avr-libc.  Due to the nature of the underlying hardware,
    only a limited subset of standard IO is implemented.  There is no
    actual file implementation available, so only device IO can be
    performed.  Since there's no operating system, the application
    needs to provide enough details about their devices in order to
    make them usable by the standard IO facilities.

    Due to space constraints, some functionality has not been
    implemented at all (like some of the \c printf conversions that
    have been left out).  Nevertheless, potential users of this
    implementation should be warned: the \c printf and \c scanf families of functions, although
    usually associated with presumably simple things like the
    famous "Hello, world!" program, are actually fairly complex
    which causes their inclusion to eat up a fair amount of code space.
    Also, they are not fast due to the nature of interpreting the
    format string at run-time.  Whenever possible, resorting to the
    (sometimes non-standard) predetermined conversion facilities that are
    offered by avr-libc will usually cost much less in terms of speed
    and code size.

    <h3>Tunable options for code size vs. feature set</h3>

    In order to allow programmers a code size vs. functionality tradeoff,
    the function vfprintf() which is the heart of the printf family can be
    selected in different flavours using linker options.  See the
    documentation of vfprintf() for a detailed description.  The same
    applies to vfscanf() and the \c scanf family of functions.

    <h3>Outline of the chosen API</h3>

    The standard streams \c stdin, \c stdout, and \c stderr are
    provided, but contrary to the C standard, since avr-libc has no
    knowledge about applicable devices, these streams are not already
    pre-initialized at application startup.  Also, since there is no
    notion of "file" whatsoever to avr-libc, there is no function
    \c fopen() that could be used to associate a stream to some device.
    (See \ref stdio_note1 "note 1".)  Instead, the function \c fdevopen()
    is provided to associate a stream to a device, where the device
    needs to provide a function to send a character, to receive a
    character, or both.  There is no differentiation between "text" and
    "binary" streams inside avr-libc.  Character \c \\n is sent
    literally down to the device's \c put() function.  If the device
    requires a carriage return (\c \\r) character to be sent before
    the linefeed, its \c put() routine must implement this (see
    \ref stdio_note2 "note 2").

    As an alternative method to fdevopen(), the macro
    fdev_setup_stream() might be used to setup a user-supplied FILE
    structure.

    It should be noted that the automatic conversion of a newline
    character into a carriage return - newline sequence breaks binary
    transfers.  If binary transfers are desired, no automatic
    conversion should be performed, but instead any string that aims
    to issue a CR-LF sequence must use <tt>"\r\n"</tt> explicitly.

    stdin, stdout and stderr are undefined global FILE pointers. If
    you want to use this, your application must define these variables
    and initialize them. They are declared 'const' so that you can place
    them in ROM if you don't need to modify it after startup. FILEs
    cannot be placed in ROM as they have values which are modified
    during runtime.

    \anchor stdio_without_malloc
    <h3>Running stdio without malloc()</h3>

    By default, fdevopen() requires malloc().  As this is often
    not desired in the limited environment of a microcontroller, an
    alternative option is provided to run completely without malloc().

    The macro fdev_setup_stream() is provided to prepare a
    user-supplied FILE buffer for operation with stdio.

    <h4>Example</h4>

    \code
    #include <stdio.h>

    static int uart_putchar(char c, FILE *stream);

    static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
                                             _FDEV_SETUP_WRITE);

    static int
    uart_putchar(char c, FILE *stream)
    {

      if (c == '\n')
        uart_putchar('\r', stream);
      loop_until_bit_is_set(UCSRA, UDRE);
      UDR = c;
      return 0;
    }

    int
    main(void)
    {
      init_uart();
      stdout = &mystdout;
      printf("Hello, world!\n");

      return 0;
    }
    \endcode

    This example uses the initializer form FDEV_SETUP_STREAM() rather
    than the function-like fdev_setup_stream(), so all data
    initialization happens during C start-up.

    If streams initialized that way are no longer needed, they can be
    destroyed by first calling the macro fdev_close(), and then
    destroying the object itself.  No call to fclose() should be
    issued for these streams.  While calling fclose() itself is
    harmless, it will cause an undefined reference to free() and thus
    cause the linker to link the malloc module into the application.

    <h3>Notes</h3>

    \anchor stdio_note1 \par Note 1:
    It might have been possible to implement a device abstraction that
    is compatible with \c fopen() but since this would have required
    to parse a string, and to take all the information needed either
    out of this string, or out of an additional table that would need to be
    provided by the application, this approach was not taken.

    \anchor stdio_note2 \par Note 2:
    This basically follows the Unix approach: if a device such as a
    terminal needs special handling, it is in the domain of the
    terminal device driver to provide this functionality.  Thus, a
    simple function suitable as \c put() for \c fdevopen() that talks
    to a UART interface might look like this:

    \code
    int
    uart_putchar(char c, FILE *stream)
    {

      if (c == '\n')
        uart_putchar('\r', stream);
      loop_until_bit_is_set(UCSRA, UDRE);
      UDR = c;
      return 0;
    }
    \endcode

    \anchor stdio_note3 \par Note 3:
    This implementation has been chosen because the cost of maintaining
    an alias is considerably smaller than the cost of maintaining full
    copies of each stream.  Yet, providing an implementation that offers
    the complete set of standard streams was deemed to be useful.  Not
    only that writing \c printf() instead of <tt>fprintf(mystream, ...)</tt>
    saves typing work, but since avr-gcc needs to resort to pass all
    arguments of variadic functions on the stack (as opposed to passing
    them in registers for functions that take a fixed number of
    parameters), the ability to pass one parameter less by implying
    \c stdin or stdout will also save some execution time.
*/

#if !defined(__DOXYGEN__)

/*
 * This is an internal structure of the library that is subject to be
 * changed without warnings at any time.  Please do *never* reference
 * elements of it beyond by using the official interfaces provided.
 */

#ifdef ATOMIC_UNGETC
#ifdef __riscv
/*
 * Use 32-bit ungetc storage when doing atomic ungetc on RISC-V, which
 * has 4-byte swap intrinsics but not 2-byte swap intrinsics. This
 * increases the size of the __file struct by four bytes.
 */
#define __PICOLIBC_UNGETC_SIZE	4
#endif
#endif

#ifndef __PICOLIBC_UNGETC_SIZE
#define __PICOLIBC_UNGETC_SIZE	2
#endif

#if __PICOLIBC_UNGETC_SIZE == 4
typedef uint32_t __ungetc_t;
#endif

#if __PICOLIBC_UNGETC_SIZE == 2
typedef uint16_t __ungetc_t;
#endif

struct __file {
	__ungetc_t unget;	/* ungetc() buffer */
	uint8_t	flags;		/* flags, see below */
#define __SRD	0x0001		/* OK to read */
#define __SWR	0x0002		/* OK to write */
#define __SERR	0x0004		/* found error */
#define __SEOF	0x0008		/* found EOF */
#define __SCLOSE 0x0010		/* struct is __file_close */
#define __SEXT  0x0020          /* struct is __file_ext */
#define __SBUF  0x0040          /* struct is __file_bufio */
	int	(*put)(char, struct __file *);	/* function to write one char to device */
	int	(*get)(struct __file *);	/* function to read one char from device */
	int	(*flush)(struct __file *);	/* function to flush output to device */
};

/*
 * This variant includes a 'close' function which is
 * invoked from fclose when the __SCLOSE bit is set
 */
struct __file_close {
	struct __file file;			/* main file struct */
	int	(*close)(struct __file *);	/* function to close file */
};

#define FDEV_SETUP_CLOSE(put, get, flush, _close, rwflag) \
        {                                                               \
                .file = FDEV_SETUP_STREAM(put, get, flush, (rwflag) | __SCLOSE),   \
                .close = (_close),                                      \
        }

struct __file_ext {
        struct __file_close cfile;              /* close file struct */
        __off_t (*seek)(struct __file *, __off_t offset, int whence);
        int     (*setvbuf)(struct __file *, char *buf, int mode, size_t size);
};

#define FDEV_SETUP_EXT(put, get, flush, close, _seek, _setvbuf, rwflag) \
        {                                                               \
                .cfile = FDEV_SETUP_CLOSE(put, get, flush, close, (rwflag) | __SEXT), \
                .seek = (_seek),                                        \
                .setvbuf = (_setvbuf),                                  \
        }

#endif /* not __DOXYGEN__ */

/*@{*/
/**
   \c FILE is the opaque structure that is passed around between the
   various standard IO functions.
*/
typedef struct __file __FILE;
#if !defined(__FILE_defined)
typedef __FILE FILE;
# define __FILE_defined
#endif

/**
   This symbol is defined when stdin/stdout/stderr are global
   variables. When undefined, the old __iob array is used which
   contains the pointers instead
*/
#define PICOLIBC_STDIO_GLOBALS

/**
   Stream that will be used as an input stream by the simplified
   functions that don't take a \c stream argument.
*/
extern FILE *const stdin;

/**
   Stream that will be used as an output stream by the simplified
   functions that don't take a \c stream argument.
*/
extern FILE *const stdout;

/**
   Stream destined for error output.  Unless specifically assigned,
   identical to \c stdout.
*/
extern FILE *const stderr;

/**
   \c EOF declares the value that is returned by various standard IO
   functions in case of an error.  Since the AVR platform (currently)
   doesn't contain an abstraction for actual files, its origin as
   "end of file" is somewhat meaningless here.
*/
#define EOF	(-1)

#define	_IOFBF	0		/* setvbuf should set fully buffered */
#define	_IOLBF	1		/* setvbuf should set line buffered */
#define	_IONBF	2		/* setvbuf should set unbuffered */

#if defined(__DOXYGEN__)
/**
   \brief Setup a user-supplied buffer as an stdio stream

   This macro takes a user-supplied buffer \c stream, and sets it up
   as a stream that is valid for stdio operations, similar to one that
   has been obtained dynamically from fdevopen(). The buffer to setup
   must be of type FILE.

   The arguments \c put , \c get and \c flush are identical to those
   that need to be passed to fdevopen().

   The \c rwflag argument can take one of the values _FDEV_SETUP_READ,
   _FDEV_SETUP_WRITE, or _FDEV_SETUP_RW, for read, write, or read/write
   intent, respectively.

   \note No assignments to the standard streams will be performed by
   fdev_setup_stream().  If standard streams are to be used, these
   need to be assigned by the user.  See also under
   \ref stdio_without_malloc "Running stdio without malloc()".
 */
#define fdev_setup_stream(stream, put, get, flush, rwflag)
#else  /* !DOXYGEN */
#define fdev_setup_stream(stream, p, g, fl, f)	\
	do { \
		(stream)->flags = f; \
		(stream)->put = p; \
		(stream)->get = g; \
		(stream)->flush = fl; \
	} while(0)
#endif /* DOXYGEN */

#define _FDEV_SETUP_READ  __SRD	/**< fdev_setup_stream() with read intent */
#define _FDEV_SETUP_WRITE __SWR	/**< fdev_setup_stream() with write intent */
#define _FDEV_SETUP_RW    (__SRD|__SWR)	/**< fdev_setup_stream() with read/write intent */

/**
 * Return code for an error condition during device read.
 *
 * To be used in the get function of fdevopen().
 */
#define _FDEV_ERR (-1)

/**
 * Return code for an end-of-file condition during device read.
 *
 * To be used in the get function of fdevopen().
 */
#define _FDEV_EOF (-2)

#if defined(__DOXYGEN__)
/**
   \brief Initializer for a user-supplied stdio stream

   This macro acts similar to fdev_setup_stream(), but it is to be
   used as the initializer of a variable of type FILE.

   The remaining arguments are to be used as explained in
   fdev_setup_stream().
 */
#define FDEV_SETUP_STREAM(put, get, flush, rwflag)
#else  /* !DOXYGEN */
#define FDEV_SETUP_STREAM(p, g, fl, f)		\
	{ \
                .flags = (f),                   \
                .put = (p),                     \
                .get = (g),                     \
                .flush = (fl),                  \
	}
#endif /* DOXYGEN */

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__DOXYGEN__)
/*
 * Doxygen documentation can be found in fdevopen.c.
 */

extern FILE *fdevopen(int (*__put)(char, FILE*), int (*__get)(FILE*), int(*__flush)(FILE *));

#endif /* not __DOXYGEN__ */

/**
   This function closes \c stream, and disallows and further
   IO to and from it.

   When using fdevopen() to setup the stream, a call to fclose() is
   needed in order to free the internal resources allocated.

   If the stream has been set up using fdev_setup_stream() or
   FDEV_SETUP_STREAM(), use fdev_close() instead.

   It currently always returns 0 (for success).
*/
extern int	fclose(FILE *__stream);

/**
   This macro frees up any library resources that might be associated
   with \c stream.  It should be called if \c stream is no longer
   needed, right before the application is going to destroy the
   \c stream object itself.

   All that is needed is to flush any pending output.
*/
#if defined(__DOXYGEN__)
# define fdev_close(f)
#else
# define fdev_close(f) (fflush(f))
#endif

/**
   \c vfprintf is the central facility of the \c printf family of
   functions.  It outputs values to \c stream under control of a
   format string passed in \c fmt.  The actual values to print are
   passed as a variable argument list \c ap.

   \c vfprintf returns the number of characters written to \c stream,
   or \c EOF in case of an error.  Currently, this will only happen
   if \c stream has not been opened with write intent.

   The format string is composed of zero or more directives: ordinary
   characters (not \c %), which are copied unchanged to the output
   stream; and conversion specifications, each of which results in
   fetching zero or more subsequent arguments.  Each conversion
   specification is introduced by the \c % character.  The arguments must
   properly correspond (after type promotion) with the conversion
   specifier.  After the \c %, the following appear in sequence:

   - Zero or more of the following flags:
      <ul>
      <li> \c # The value should be converted to an "alternate form".  For
            c, d, i, s, and u conversions, this option has no effect.
            For o conversions, the precision of the number is
            increased to force the first character of the output
            string to a zero (except if a zero value is printed with
            an explicit precision of zero).  For x and X conversions,
            a non-zero result has the string `0x' (or `0X' for X
            conversions) prepended to it.</li>
      <li> \c 0 (zero) Zero padding.  For all conversions, the converted
            value is padded on the left with zeros rather than blanks.
            If a precision is given with a numeric conversion (d, i,
            o, u, i, x, and X), the 0 flag is ignored.</li>
      <li> \c - A negative field width flag; the converted value is to be
            left adjusted on the field boundary.  The converted value
            is padded on the right with blanks, rather than on the
            left with blanks or zeros.  A - overrides a 0 if both are
            given.</li>
      <li> ' ' (space) A blank should be left before a positive number
            produced by a signed conversion (d, or i).</li>
      <li> \c + A sign must always be placed before a number produced by a
            signed conversion.  A + overrides a space if both are
            used.</li>
      </ul>

   -   An optional decimal digit string specifying a minimum field width.
       If the converted value has fewer characters than the field width, it
       will be padded with spaces on the left (or right, if the left-adjustment
       flag has been given) to fill out the field width.
   -   An optional precision, in the form of a period . followed by an
       optional digit string.  If the digit string is omitted, the
       precision is taken as zero.  This gives the minimum number of
       digits to appear for d, i, o, u, x, and X conversions, or the
       maximum number of characters to be printed from a string for \c s
       conversions.
   -   An optional \c l or \c h length modifier, that specifies that the
       argument for the d, i, o, u, x, or X conversion is a \c "long int"
       rather than \c int. The \c h is ignored, as \c "short int" is
       equivalent to \c int.
   -   A character that specifies the type of conversion to be applied.

   The conversion specifiers and their meanings are:

   - \c diouxX The int (or appropriate variant) argument is converted
           to signed decimal (d and i), unsigned octal (o), unsigned
           decimal (u), or unsigned hexadecimal (x and X) notation.
           The letters "abcdef" are used for x conversions; the
           letters "ABCDEF" are used for X conversions.  The
           precision, if any, gives the minimum number of digits that
           must appear; if the converted value requires fewer digits,
           it is padded on the left with zeros.
   - \c p  The <tt>void *</tt> argument is taken as an unsigned integer,
           and converted similarly as a <tt>%\#x</tt> command would do.
   - \c c  The \c int argument is converted to an \c "unsigned char", and the
           resulting character is written.
   - \c s  The \c "char *" argument is expected to be a pointer to an array
           of character type (pointer to a string).  Characters from
           the array are written up to (but not including) a
           terminating NUL character; if a precision is specified, no
           more than the number specified are written.  If a precision
           is given, no null character need be present; if the
           precision is not specified, or is greater than the size of
           the array, the array must contain a terminating NUL
           character.
   - \c %  A \c % is written.  No argument is converted.  The complete
           conversion specification is "%%".
   - \c eE The double argument is rounded and converted in the format
           \c "[-]d.ddde±dd" where there is one digit before the
           decimal-point character and the number of digits after it
           is equal to the precision; if the precision is missing, it
           is taken as 6; if the precision is zero, no decimal-point
           character appears.  An \e E conversion uses the letter \c 'E'
           (rather than \c 'e') to introduce the exponent.  The exponent
           always contains two digits; if the value is zero,
           the exponent is 00.
   - \c fF The double argument is rounded and converted to decimal notation
           in the format \c "[-]ddd.ddd", where the number of digits after the
           decimal-point character is equal to the precision specification.
           If the precision is missing, it is taken as 6; if the precision
           is explicitly zero, no decimal-point character appears.  If a
           decimal point appears, at least one digit appears before it.
   - \c gG The double argument is converted in style \c f or \c e (or
           \c F or \c E for \c G conversions).  The precision
           specifies the number of significant digits.  If the
           precision is missing, 6 digits are given; if the precision
           is zero, it is treated as 1.  Style \c e is used if the
           exponent from its conversion is less than -4 or greater
           than or equal to the precision.  Trailing zeros are removed
           from the fractional part of the result; a decimal point
           appears only if it is followed by at least one digit.
   - \c S  Similar to the \c s format, except the pointer is expected to
           point to a program-memory (ROM) string instead of a RAM string.

   In no case does a non-existent or small field width cause truncation of a
   numeric field; if the result of a conversion is wider than the field
   width, the field is expanded to contain the conversion result.

   Since the full implementation of all the mentioned features becomes
   fairly large, three different flavours of vfprintf() can be
   selected using linker options.  The default vfprintf() implements
   all the mentioned functionality except floating point conversions.
   A minimized version of vfprintf() is available that only implements
   the very basic integer and string conversion facilities, but only
   the \c # additional option can be specified using conversion
   flags (these flags are parsed correctly from the format
   specification, but then simply ignored).  This version can be
   requested using the following \ref gcc_minusW "compiler options":

   \code
   -Wl,-u,vfprintf -lprintf_min
   \endcode

   If the full functionality including the floating point conversions
   is required, the following options should be used:

   \code
   -Wl,-u,vfprintf -lprintf_flt -lm
   \endcode

   \par Limitations:
   - The specified width and precision can be at most 255.

   \par Notes:
   - For floating-point conversions, if you link default or minimized
     version of vfprintf(), the symbol \c ? will be output and double
     argument will be skiped. So you output below will not be crashed.
     For default version the width field and the "pad to left" ( symbol
     minus ) option will work in this case.
   - The \c hh length modifier is ignored (\c char argument is
     promouted to \c int). More exactly, this realization does not check
     the number of \c h symbols.
   - But the \c ll length modifier will to abort the output, as this
     realization does not operate \c long \c long arguments.
   - The variable width or precision field (an asterisk \c * symbol)
     is not realized and will to abort the output.

 */

#ifdef _HAVE_FORMAT_ATTRIBUTE
#ifdef PICOLIBC_FLOAT_PRINTF_SCANF
#pragma GCC diagnostic ignored "-Wformat"
#define __FORMAT_ATTRIBUTE__(__a, __s, __f) __attribute__((__format__ (__a, __s, 0)))
#else
#define __FORMAT_ATTRIBUTE__(__a, __s, __f) __attribute__((__format__ (__a, __s, __f)))
#endif
#else
#define __FORMAT_ATTRIBUTE__(__a, __s, __f)
#endif

#define __PRINTF_ATTRIBUTE__(__s, __f) __FORMAT_ATTRIBUTE__(printf, __s, __f)
#define __SCANF_ATTRIBUTE__(__s, _f) __FORMAT_ATTRIBUTE__(scanf, __s, __f)

int	vfprintf(FILE *__stream, const char *__fmt, va_list __ap) __PRINTF_ATTRIBUTE__(2, 0);

/**
   The function \c fputc sends the character \c c (though given as type
   \c int) to \c stream.  It returns the character, or \c EOF in case
   an error occurred.
*/
extern int	fputc(int __c, FILE *__stream);

#if !defined(__DOXYGEN__)

/* putc() function implementation, required by standard */
extern int	putc(int __c, FILE *__stream);

/* putchar() function implementation, required by standard */
extern int	putchar(int __c);

#endif /* not __DOXYGEN__ */

/**
   The macro \c putc used to be a "fast" macro implementation with a
   functionality identical to fputc().  For space constraints, in
   \c avr-libc, it is just an alias for \c fputc.
*/
#define putc(__c, __stream) fputc(__c, __stream)

/**
   The macro \c putchar sends character \c c to \c stdout.
*/
#define putchar(__c) fputc(__c, stdout)

/**
   The function \c printf performs formatted output to stream
   \c stdout.  See \c vfprintf() for details.
*/

int	printf(const char *__fmt, ...) __PRINTF_ATTRIBUTE__(1, 2);

/**
   The function \c vprintf performs formatted output to stream
   \c stdout, taking a variable argument list as in vfprintf().

   See vfprintf() for details.
*/
extern int	vprintf(const char *__fmt, va_list __ap) __PRINTF_ATTRIBUTE__(1, 0);

/**
   Variant of \c printf() that sends the formatted characters
   to string \c s.
*/
extern int	sprintf(char *__s, const char *__fmt, ...) __PRINTF_ATTRIBUTE__(2, 3);

/**
   Like \c sprintf(), but instead of assuming \c s to be of infinite
   size, no more than \c n characters (including the trailing NUL
   character) will be converted to \c s.

   Returns the number of characters that would have been written to
   \c s if there were enough space.
*/
extern int	snprintf(char *__s, size_t __n, const char *__fmt, ...) __PRINTF_ATTRIBUTE__(3, 4);

/**
   Like \c sprintf() but takes a variable argument list for the
   arguments.
*/
extern int	vsprintf(char *__s, const char *__fmt, va_list ap) __PRINTF_ATTRIBUTE__(2, 0);

/**
   Like \c vsprintf(), but instead of assuming \c s to be of infinite
   size, no more than \c n characters (including the trailing NUL
   character) will be converted to \c s.

   Returns the number of characters that would have been written to
   \c s if there were enough space.
*/
extern int	vsnprintf(char *__s, size_t __n, const char *__fmt, va_list ap) __PRINTF_ATTRIBUTE__(3, 0);

/**
   Variant of \c printf() that sends the formatted characters
   to allocated string \c *strp.
*/
int
asprintf(char **strp, const char *fmt, ...) __PRINTF_ATTRIBUTE__(2,3);

/**
   Variant of \c vprintf() that sends the formatted characters
   to allocated string \c *strp.
*/
int
vasprintf(char **strp, const char *fmt, va_list ap) __PRINTF_ATTRIBUTE__(2,0);

/**
   The function \c fprintf performs formatted output to \c stream.
   See \c vfprintf() for details.
*/
extern int	fprintf(FILE *__stream, const char *__fmt, ...) __PRINTF_ATTRIBUTE__(2, 3);

/**
   Write the string pointed to by \c str to stream \c stream.

   Returns 0 on success and EOF on error.
*/
extern int	fputs(const char *__str, FILE *__stream);

/**
   Write the string pointed to by \c str, and a trailing newline
   character, to \c stdout.
*/
extern int	puts(const char *__str);

/**
   Write \c nmemb objects, \c size bytes each, to \c stream.
   The first byte of the first object is referenced by \c ptr.

   Returns the number of objects successfully written, i. e.
   \c nmemb unless an output error occured.
 */
extern size_t	fwrite(const void *__ptr, size_t __size, size_t __nmemb,
		       FILE *__stream);

/**
   The function \c fgetc reads a character from \c stream.  It returns
   the character, or \c EOF in case end-of-file was encountered or an
   error occurred.  The routines feof() or ferror() must be used to
   distinguish between both situations.
*/
extern int	fgetc(FILE *__stream);

#if !defined(__DOXYGEN__)

/* getc() function implementation, required by standard */
extern int	getc(FILE *__stream);

/* getchar() function implementation, required by standard */
extern int	getchar(void);

#endif /* not __DOXYGEN__ */

/**
   The macro \c getc used to be a "fast" macro implementation with a
   functionality identical to fgetc().  For space constraints, in
   \c avr-libc, it is just an alias for \c fgetc.
*/
#define getc(__stream) fgetc(__stream)

/**
   The macro \c getchar reads a character from \c stdin.  Return
   values and error handling is identical to fgetc().
*/
#define getchar() fgetc(stdin)

/**
   The ungetc() function pushes the character \c c (converted to an
   unsigned char) back onto the input stream pointed to by \c stream.
   The pushed-back character will be returned by a subsequent read on
   the stream.

   Currently, only a single character can be pushed back onto the
   stream.

   The ungetc() function returns the character pushed back after the
   conversion, or \c EOF if the operation fails.  If the value of the
   argument \c c character equals \c EOF, the operation will fail and
   the stream will remain unchanged.
*/
extern int	ungetc(int __c, FILE *__stream);

/**
   Read at most <tt>size - 1</tt> bytes from \c stream, until a
   newline character was encountered, and store the characters in the
   buffer pointed to by \c str.  Unless an error was encountered while
   reading, the string will then be terminated with a \c NUL
   character.

   If an error was encountered, the function returns NULL and sets the
   error flag of \c stream, which can be tested using ferror().
   Otherwise, a pointer to the string will be returned.  */
extern char	*fgets(char *__str, int __size, FILE *__stream);

/**
   Similar to fgets() except that it will operate on stream \c stdin,
   and the trailing newline (if any) will not be stored in the string.
   It is the caller's responsibility to provide enough storage to hold
   the characters read.  */
extern char	*gets(char *__str);

/**
   Read \c nmemb objects, \c size bytes each, from \c stream,
   to the buffer pointed to by \c ptr.

   Returns the number of objects successfully read, i. e.
   \c nmemb unless an input error occured or end-of-file was
   encountered.  feof() and ferror() must be used to distinguish
   between these two conditions.
 */
extern size_t	fread(void *__ptr, size_t __size, size_t __nmemb,
		      FILE *__stream);

/**
   Clear the error and end-of-file flags of \c stream.
 */
extern void	clearerr(FILE *__stream);

#if !defined(__DOXYGEN__)
/* fast inlined version of clearerr() */
#define clearerror(s) do { (s)->flags &= ~(__SERR | __SEOF); } while(0)
#endif /* !defined(__DOXYGEN__) */

/**
   Test the end-of-file flag of \c stream.  This flag can only be cleared
   by a call to clearerr().
 */
extern int	feof(FILE *__stream);

#if !defined(__DOXYGEN__)
/* fast inlined version of feof() */
#define feof(s) ((s)->flags & __SEOF)
#endif /* !defined(__DOXYGEN__) */

/**
   Test the error flag of \c stream.  This flag can only be cleared
   by a call to clearerr().
 */
extern int	ferror(FILE *__stream);

#if !defined(__DOXYGEN__)
/* fast inlined version of ferror() */
#define ferror(s) ((s)->flags & __SERR)
#endif /* !defined(__DOXYGEN__) */

extern int	vfscanf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(scanf, 2, 0);

/**
   The function \c fscanf performs formatted input, reading the
   input data from \c stream.

   See vfscanf() for details.
 */
extern int	fscanf(FILE *__stream, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(scanf, 2, 3);

/**
   The function \c scanf performs formatted input from stream \c stdin.

   See vfscanf() for details.
 */
extern int	scanf(const char *__fmt, ...) __FORMAT_ATTRIBUTE__(scanf, 1, 2);

/**
   The function \c vscanf performs formatted input from stream
   \c stdin, taking a variable argument list as in vfscanf().

   See vfscanf() for details.
*/
extern int	vscanf(const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(scanf, 1, 0);

/**
   The function \c sscanf performs formatted input, reading the
   input data from the buffer pointed to by \c buf.

   See vfscanf() for details.
 */
extern int	sscanf(const char *__buf, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(scanf, 2, 3);

/**
   The function \c vscanf performs formatted input, reading the
   input data from the buffer pointed to by \c buf.

   See vfscanf() for details.
 */
extern int	vsscanf(const char *__buf, const char *__fmt, va_list ap) __FORMAT_ATTRIBUTE__(scanf, 2, 0);

/**
   Flush \c stream.

   If the stream provides a flush hook, use that. Otherwise return 0.
 */
extern int	fflush(FILE *stream);

#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

#ifndef __DOXYGEN__
/* only mentioned for libstdc++ support, not implemented in library */
#ifndef BUFSIZ
#define BUFSIZ 512
#endif
__extension__ typedef long long fpos_t;
extern int fgetpos(FILE *stream, fpos_t *pos);
extern FILE *fopen(const char *path, const char *mode);
extern FILE *freopen(const char *path, const char *mode, FILE *stream);
extern FILE *fdopen(int, const char *);
extern FILE *fmemopen(void *buf, size_t size, const char *mode);
extern int fseek(FILE *stream, long offset, int whence);
extern int fseeko(FILE *stream, __off_t offset, int whence);
extern int fsetpos(FILE *stream, fpos_t *pos);
extern long ftell(FILE *stream);
extern __off_t ftello(FILE *stream);
extern int fileno(FILE *);
extern void perror(const char *s);
extern int remove(const char *pathname);
extern int rename(const char *oldpath, const char *newpath);
extern void rewind(FILE *stream);
extern void setbuf(FILE *stream, char *buf);
extern void setbuffer(FILE *stream, char *buf, size_t size);
extern void setlinebuf(FILE *stream);
extern int setvbuf(FILE *stream, char *buf, int mode, size_t size);
extern FILE *tmpfile(void);
extern char *tmpnam (char *s);

/*
 * The format of tmpnam names is TXXXXXX, which works with mktemp
 */
#define L_tmpnam        8

/*
 * tmpnam files are created in the current directory
 */
#define P_tmpdir        ""

/*
 * We don't have any way of knowing any underlying POSIX limits,
 * so just use a reasonably small value here
 */
#define TMP_MAX         32

#endif	/* !__DOXYGEN__ */

#ifdef __cplusplus
}
#endif

/*@}*/

#ifndef __DOXYGEN__
/*
 * The following constants are currently not used by avr-libc's
 * stdio subsystem.  They are defined here since the gcc build
 * environment expects them to be here.
 */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#endif

#endif /* __ASSEMBLER */

static __inline uint32_t
__printf_float(float f)
{
	union {
		float		f;
		uint32_t	u;
	} u = { .f = f };
	return u.u;
}

#ifdef PICOLIBC_FLOAT_PRINTF_SCANF
#define printf_float(x) __printf_float(x)
#else
#define printf_float(x) ((double) (x))
#endif

#endif /* _STDIO_H_ */
