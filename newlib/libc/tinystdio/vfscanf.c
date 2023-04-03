/* Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2008  Dmitry Xmelkov
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
*/

/* $Id: vfscanf.c 2191 2010-11-05 13:45:57Z arcanum $ */

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdio_private.h"
#include "scanf_private.h"

#ifndef SCANF_LONGLONG
# define SCANF_LONGLONG	(SCANF_FLOAT || defined(_WANT_IO_LONG_LONG))
#endif

#if defined(SCANF_FLOAT) || defined(_WANT_IO_POS_ARGS)
#define SCANF_POSITIONAL
#endif

#ifdef SCANF_LONGLONG
typedef unsigned long long uint_scanf_t;
typedef long long int_scanf_t;
#else
typedef unsigned long uint_scanf_t;
typedef long int_scanf_t;
#endif

#ifdef WIDE_CHARS
#include <wchar.h>
#define INT wint_t
#define CHAR wchar_t
#define UCHAR wchar_t
#define GETC(s) getwc(s)
#define UNGETC(c,s) ungetwc(c,s)
#undef vfscanf
#define vfscanf vfwscanf
#define IS_EOF(c)       ((c) == WEOF)
#else
#define INT int
#define CHAR char
#define UCHAR unsigned char
#define GETC(s) getc(s)
#define UNGETC(c,s) ungetc(c,s)
#define IS_EOF(c)       ((c) < 0)
#endif

static INT
scanf_getc(FILE *stream, int *lenp)
{
	INT c = GETC(stream);
	if (!IS_EOF(c))
		++(*lenp);
	return c;
}

static INT
scanf_ungetc(INT c, FILE *stream, int *lenp)
{
	c = UNGETC(c, stream);
	if (!IS_EOF(c))
		--(*lenp);
	return c;
}

static void
putval (void *addr, int_scanf_t val, uint16_t flags)
{
    if (!(flags & FL_STAR)) {
	if (flags & FL_CHAR)
	    *(char *)addr = val;
#ifdef SCANF_LONGLONG
        else if (flags & FL_LONGLONG)
            *(long long *)addr = val;
#endif
	else if (flags & FL_LONG)
	    *(long *)addr = val;
	else if (flags & FL_SHORT)
	    *(short *)addr = val;
	else
	    *(int *)addr = val;
    }
}

static unsigned char
conv_int (FILE *stream, int *lenp, width_t width, void *addr, uint16_t flags, unsigned int base)
{
    uint_scanf_t val;
    INT i;

    i = scanf_getc (stream, lenp);			/* after scanf_ungetc()	*/

    switch (i) {
      case '-':
        flags |= FL_MINUS;
	FALLTHROUGH;
      case '+':
        if (!--width || IS_EOF(i = scanf_getc(stream, lenp)))
	    goto err;
    }

    val = 0;

    /* Leading '0' digit -- check for base indication */
    if (i == '0') {
	if (!--width || IS_EOF(i = scanf_getc (stream, lenp)))
	    goto putval;

        flags |= FL_ANY;

        if (TOLOW(i) == 'x' && (base == 0 || base == 16)) {
            base = 16;
            if (!--width || IS_EOF(i = scanf_getc (stream, lenp)))
		goto putval;
#ifdef _WANT_IO_PERCENT_B
        } else if (i == 'b' && base <= 2) {
            base = 2;
            if (!--width || IS_EOF(i = scanf_getc (stream, lenp)))
		goto putval;
#endif
	} else if (base == 0 || base == 8) {
            base = 8;
        }
    } else if (base == 0)
        base = 10;

    do {
	unsigned int c = digit_to_val(i);
        if (c >= base) {
            scanf_ungetc (i, stream, lenp);
            break;
	}
        flags |= FL_ANY;
        val = val * base + c;
	if (!--width) goto putval;
    } while (!IS_EOF(i = scanf_getc(stream, lenp)));
    if (!(flags & FL_ANY))
        goto err;

  putval:
    if (flags & FL_MINUS) val = -val;
    putval (addr, val, flags);
    return 1;

  err:
    return 0;
}

#if  SCANF_BRACKET
static const CHAR *
conv_brk (FILE *stream, int *lenp, width_t width, CHAR *addr, const CHAR *fmt)
{
    unsigned char msk[32];
    unsigned char fnegate;
    unsigned char frange;
    unsigned char cabove;
    INT i;

    memset (msk, 0, sizeof(msk));
    fnegate = 0;
    frange = 0;
    cabove = 0;			/* init to avoid compiler warning	*/

    for (i = 0; ; i++) {
	unsigned char c = *fmt++;

	if (c == 0) {
	    return 0;
	} else if (c == '^' && !i) {
	    fnegate = 1;
	    continue;
	} else if (i > fnegate) {
	    if (c == ']') break;
	    if (c == '-' && !frange) {
		frange = 1;
		continue;
	    }
	}

	if (!frange) cabove = c;

	for (;;) {
	    msk[c >> 3] |= 1 << (c & 7);
	    if (c == cabove) break;
	    if (c < cabove)
		c++;
	    else
		c--;
	}

	frange = 0;
    }
    if (frange)
	msk['-'/8] |= 1 << ('-' & 7);

    if (fnegate) {
	unsigned char *p = msk;
	do {
	    unsigned char c = *p;
	    *p++ = ~c;
	} while (p != msk + sizeof(msk));
    }

    /* And now it is a flag of fault.	*/
    fnegate = 1;

    /* NUL ('\0') is consided as normal character. This is match to Glibc.
       Note, there is no method to include NUL into symbol list.	*/
    do {
	i = scanf_getc (stream, lenp);
	if (IS_EOF(i)) break;
	if (!((msk[(unsigned char)i >> 3] >> (i & 7)) & 1)) {
	    scanf_ungetc (i, stream, lenp);
	    break;
	}
	if (addr) *addr++ = i;
	fnegate = 0;
    } while (--width);

    if (fnegate) {
	return 0;
    } else {
	if (addr) *addr = 0;
        return fmt;
    }
}
#endif	/* SCANF_BRACKET */

#if  SCANF_FLOAT
#define FLT_STREAM      FILE
#include "conv_flt.c"
#endif	/* SCANF_FLOAT	*/

static INT skip_spaces (FILE *stream, int *lenp)
{
    INT i;
    do {
	if (IS_EOF(i = scanf_getc (stream, lenp)))
	    return i;
    } while (ISSPACE (i));
    scanf_ungetc (i, stream, lenp);
    return i;
}

#ifdef SCANF_POSITIONAL

typedef struct {
    va_list     ap;
} my_va_list;

static void
skip_to_arg(my_va_list *ap, int target_argno)
{
    int current_argno = 1;

    /*
     * Fortunately, all scanf args are pointers,
     * and so are the same size as void *
     */
    while (current_argno < target_argno) {
        (void) va_arg(ap->ap, void *);
        current_argno++;
    }
}

#endif

/**
   Formatted input.  This function is the heart of the \b scanf family of
   functions.

   Characters are read from \a stream and processed in a way described by
   \a fmt.  Conversion results will be assigned to the parameters passed
   via \a ap.

   The format string \a fmt is scanned for conversion specifications.
   Anything that doesn't comprise a conversion specification is taken as
   text that is matched literally against the input.  White space in the
   format string will match any white space in the data (including none),
   all other characters match only itself. Processing is aborted as soon
   as the data and format string no longer match, or there is an error or
   end-of-file condition on \a stream.

   Most conversions skip leading white space before starting the actual
   conversion.

   Conversions are introduced with the character \b %.  Possible options
   can follow the \b %:

   - a \c * indicating that the conversion should be performed but
     the conversion result is to be discarded; no parameters will
     be processed from \c ap,
   - the character \c h indicating that the argument is a pointer
     to <tt>short int</tt> (rather than <tt>int</tt>),
   - the 2 characters \c hh indicating that the argument is a pointer
     to <tt>char</tt> (rather than <tt>int</tt>).
   - the character \c l indicating that the argument is a pointer
     to <tt>long int</tt> (rather than <tt>int</tt>, for integer
     type conversions), or a pointer to \c double (for floating
     point conversions),

   In addition, a maximal field width may be specified as a nonzero
   positive decimal integer, which will restrict the conversion to at
   most this many characters from the input stream.  This field width is
   limited to at most 255 characters which is also the default value
   (except for the <tt>%c</tt> conversion that defaults to 1).

   The following conversion flags are supported:

   - \c % Matches a literal \c % character.  This is not a conversion.
   - \c d Matches an optionally signed decimal integer; the next
     pointer must be a pointer to \c int.
   - \c i Matches an optionally signed integer; the next pointer must
     be a pointer to \c int.  The integer is read in base 16 if it
     begins with \b 0x or \b 0X, in base 8 if it begins with \b 0, and
     in base 10 otherwise.  Only characters that correspond to the
     base are used.
   - \c o Matches an octal integer; the next pointer must be a pointer to
     <tt>unsigned int</tt>.
   - \c u Matches an optionally signed decimal integer; the next
     pointer must be a pointer to <tt>unsigned int</tt>.
   - \c x Matches an optionally signed hexadecimal integer; the next
     pointer must be a pointer to <tt>unsigned int</tt>.
   - \c f Matches an optionally signed floating-point number; the next
     pointer must be a pointer to \c float.
   - <tt>e, g, F, E, G</tt> Equivalent to \c f.
   - \c s
     Matches a sequence of non-white-space characters; the next pointer
     must be a pointer to \c char, and the array must be large enough to
     accept all the sequence and the terminating \c NUL character.  The
     input string stops at white space or at the maximum field width,
     whichever occurs first.
   - \c c
     Matches a sequence of width count characters (default 1); the next
     pointer must be a pointer to \c char, and there must be enough room
     for all the characters (no terminating \c NUL is added).  The usual
     skip of leading white space is suppressed.  To skip white space
     first, use an explicit space in the format.
   - \c [
     Matches a nonempty sequence of characters from the specified set
     of accepted characters; the next pointer must be a pointer to \c
     char, and there must be enough room for all the characters in the
     string, plus a terminating \c NUL character.  The usual skip of
     leading white space is suppressed.  The string is to be made up
     of characters in (or not in) a particular set; the set is defined
     by the characters between the open bracket \c [ character and a
     close bracket \c ] character.  The set excludes those characters
     if the first character after the open bracket is a circumflex
     \c ^.  To include a close bracket in the set, make it the first
     character after the open bracket or the circumflex; any other
     position will end the set.  The hyphen character \c - is also
     special; when placed between two other characters, it adds all
     intervening characters to the set.  To include a hyphen, make it
     the last character before the final close bracket.  For instance,
     <tt>[^]0-9-]</tt> means the set of <em>everything except close
     bracket, zero through nine, and hyphen</em>.  The string ends
     with the appearance of a character not in the (or, with a
     circumflex, in) set or when the field width runs out.  Note that
     usage of this conversion enlarges the stack expense.
   - \c p
     Matches a pointer value (as printed by <tt>%p</tt> in printf()); the
     next pointer must be a pointer to \c void.
   - \c n
     Nothing is expected; instead, the number of characters consumed
     thus far from the input is stored through the next pointer, which
     must be a pointer to \c int.  This is not a conversion, although it
     can be suppressed with the \c * flag.

     These functions return the number of input items assigned, which can
     be fewer than provided for, or even zero, in the event of a matching
     failure.  Zero indicates that, while there was input available, no
     conversions were assigned; typically this is due to an invalid input
     character, such as an alphabetic character for a <tt>%d</tt>
     conversion.  The value \c EOF is returned if an input failure occurs
     before any conversion such as an end-of-file occurs.  If an error or
     end-of-file occurs after conversion has begun, the number of
     conversions which were successfully completed is returned.

     By default, all the conversions described above are available except
     the floating-point conversions and the width is limited to 255
     characters.  The float-point conversion will be available in the
     extended version provided by the library \c libscanf_flt.a.  Also in
     this case the width is not limited (exactly, it is limited to 65535
     characters).  To link a program against the extended version, use the
     following compiler flags in the link stage:

     \code
     -Wl,-u,vfscanf -lscanf_flt -lm
     \endcode

     A third version is available for environments that are tight on
     space.  In addition to the restrictions of the standard one, this
     version implements no <tt>%[</tt> specification.  This version is
     provided in the library \c libscanf_min.a, and can be requested using
     the following options in the link stage:

     \code
     -Wl,-u,vfscanf -lscanf_min -lm
     \endcode
*/
int vfscanf (FILE * stream, const CHAR *fmt, va_list ap_orig)
{
    unsigned char nconvs;
    UCHAR c;
    width_t width;
    void *addr;
#ifdef SCANF_POSITIONAL
    my_va_list my_ap;
#define ap my_ap.ap
    va_copy(ap, ap_orig);
#else
#define ap ap_orig
#endif
    uint16_t flags;
    INT i;
    int scanf_len = 0;
#define lenp (&scanf_len)

    nconvs = 0;

    /* Initialization of stream_flags at each pass simplifies the register
       allocation with GCC 3.3 - 4.2.  Only the GCC 4.3 is good to move it
       to the begin.	*/
    while ((c = *fmt++) != 0) {

	if (ISSPACE (c)) {
	    skip_spaces (stream, lenp);

	} else if (c != '%'
		   || (c = *fmt++) == '%')
	{
	    /* Ordinary character.	*/
	    if (IS_EOF(i = scanf_getc (stream, lenp)))
		goto eof;
	    if ((UCHAR)i != c) {
		scanf_ungetc (i, stream, lenp);
		break;
	    }

	} else {
	    flags = 0;

	    if (c == '*') {
		flags = FL_STAR;
		c = *fmt++;
	    }

            for (;;) {
                width = 0;
                while ((c -= '0') < 10) {
                    flags |= FL_WIDTH;
                    width = width * 10 + c;
                    c = *fmt++;
                }
                c += '0';
                if (flags & FL_WIDTH) {
#ifdef SCANF_POSITIONAL
                    if (c == '$') {
                        flags &= ~FL_WIDTH;
                        va_end(ap);
                        va_copy(ap, ap_orig);
                        skip_to_arg(&my_ap, width);
                        c = *fmt++;
                        continue;
                    }
#endif
                    /* C99 says that width must be greater than zero.
                       To simplify program do treat 0 as error in format.	*/
                    if (!width) break;
                } else {
                    width = ~0;
                }
                break;
            }

	    switch (c) {
	      case 'h':
                flags |= FL_SHORT;
		c = *fmt++;
                if (c == 'h') {
                    flags |= FL_CHAR;
                    c = *fmt++;
                }
		break;
	      case 'l':
		flags |= FL_LONG;
		c = *fmt++;
#ifdef SCANF_LONGLONG
                if (c == 'l') {
                    flags |= FL_LONGLONG;
                    c = *fmt++;
                }
#endif
		break;
#ifdef SCANF_LONGLONG
              case 'L':
                flags |= FL_LONGLONG;
                c = *fmt++;
                break;
#endif
#ifdef _WANT_IO_C99_FORMATS
#ifdef SCANF_LONGLONG
#define CHECK_LONGLONG(type)                                    \
                else if (sizeof(type) == sizeof(long long))     \
                    flags |= FL_LONGLONG
#else
#define CHECK_LONGLONG(type
#endif

#define CHECK_INT_SIZE(letter, type)				\
	    case letter:					\
		if (sizeof(type) != sizeof(int)) {		\
		    if (sizeof(type) == sizeof(long))		\
			flags |= FL_LONG;                       \
		    else if (sizeof(type) == sizeof(short))     \
			flags |= FL_SHORT;                      \
                    CHECK_LONGLONG(type);                       \
		}						\
		c = *fmt++;					\
		break;

	    CHECK_INT_SIZE('j', intmax_t);
	    CHECK_INT_SIZE('z', size_t);
	    CHECK_INT_SIZE('t', ptrdiff_t);
#endif
	    }

#ifdef _WANT_IO_PERCENT_B
#define CNV_BASE	"cdinopsuxXb"
#else
#define CNV_BASE	"cdinopsuxX"
#endif

#if	SCANF_BRACKET
# define CNV_BRACKET	"["
#else
# define CNV_BRACKET	""
#endif
#if	SCANF_FLOAT
# define CNV_FLOAT	"efgEFG"
#else
# define CNV_FLOAT	""
#endif
#define CNV_LIST	CNV_BASE CNV_BRACKET CNV_FLOAT
	    if (!c || !strchr (CNV_LIST, c))
		break;

	    addr = (flags & FL_STAR) ? 0 : va_arg (ap, void *);

	    if (c == 'n') {
		putval (addr, (unsigned)(scanf_len), flags);
		continue;
	    }

	    if (c == 'c') {
		if (!(flags & FL_WIDTH)) width = 1;
		do {
		    if (IS_EOF(i = scanf_getc (stream, lenp)))
			goto eof;
		    if (addr) {
#ifdef WIDE_CHARS
                            if (flags & FL_LONG) {
                                *(wchar_t *)addr = i;
                                addr = (wchar_t *)addr + 1;
                            } else
#endif
                            {
                                *(char *)addr = i;
                                addr = (char*)addr + 1;
                            }
                    }
		} while (--width);
		c = 1;			/* no matter with smart GCC	*/

#if  SCANF_BRACKET
	    } else if (c == '[') {
		fmt = conv_brk (stream, lenp, width, addr, fmt);
		c = (fmt != 0);
#endif

	    } else {

                unsigned int base = 0;

		if (IS_EOF(skip_spaces (stream, lenp)))
		    goto eof;

		switch (c) {

		  case 's':
		    /* Now we have 1 nospace symbol.	*/
		    do {
			if (IS_EOF(i = scanf_getc (stream, lenp)))
			    break;
			if (ISSPACE (i)) {
			    scanf_ungetc (i, stream, lenp);
			    break;
			}
			if (addr) {
#ifdef WIDE_CHARS
                            if (flags & FL_LONG) {
                                *(wchar_t *)addr = i;
                                addr = (wchar_t *)addr + 1;
                            } else
#endif
                            {
                                *(char *)addr = i;
                                addr = (char*)addr + 1;
                            }
			}
		    } while (--width);
                    if (addr){
#ifdef WIDE_CHARS
                        if (flags & FL_LONG)
                            *(wchar_t *)addr = 0;
                        else
#endif
                            *(char *)addr = 0;
                    }
		    c = 1;		/* no matter with smart GCC	*/
		    break;

#if  SCANF_FLOAT
	          case 'p':
                      if (sizeof(void *) > sizeof(int))
                          flags |= FL_LONG;
                      FALLTHROUGH;
		  case 'x':
	          case 'X':
                    base = 16;
		    goto conv_int;

#ifdef _WANT_IO_PERCENT_B
                  case 'b':
                    base = 2;
                    goto conv_int;
#endif

	          case 'd':
		  case 'u':
                    base = 10;
		    goto conv_int;

	          case 'o':
                    base = 8;
		    FALLTHROUGH;
		  case 'i':
		  conv_int:
                    c = conv_int (stream, lenp, width, addr, flags, base);
		    break;

	          default:		/* e,E,f,F,g,G	*/
		      c = conv_flt (stream, lenp, width, addr, flags);
#else
	          case 'd':
		  case 'u':
                    base = 10;
		    goto conv_int;

#ifdef _WANT_IO_PERCENT_B
                  case 'b':
                    base = 2;
                    goto conv_int;
#endif

	          case 'o':
                    base = 8;
		    FALLTHROUGH;
		  case 'i':
		    goto conv_int;

                  case 'p':
                      if (sizeof(void *) > sizeof(int))
                          flags |= FL_LONG;
                      FALLTHROUGH;
		  default:			/* p,x,X	*/
                    base = 16;
		  conv_int:
		    c = conv_int (stream, lenp, width, addr, flags, base);
#endif
		}
	    } /* else */

	    if (!c) {
		if (stream->flags & (__SERR | __SEOF))
		    goto eof;
		break;
	    }
	    if (!(flags & FL_STAR)) nconvs += 1;
	} /* else */
    } /* while */
#ifdef PRINTF_POSITIONAL
    va_end(ap);
#endif
    return nconvs;

  eof:
#ifdef PRINTF_POSITIONAL
    va_end(ap);
#endif
#undef ap
    return nconvs ? nconvs : EOF;
}

#if defined(FORMAT_DEFAULT_DOUBLE) && !defined(vfscanf)
#ifdef _HAVE_ALIAS_ATTRIBUTE
__strong_reference(vfscanf, __d_vfscanf);
#else
int __d_vfscanf (FILE * stream, const char *fmt, va_list ap) { return vfscanf(stream, fmt, ap); }
#endif
#endif
