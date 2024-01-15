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

#ifndef _SCANF_PRIVATE_H_
#define _SCANF_PRIVATE_H_

#if	!defined (SCANF_LEVEL)
# define SCANF_LEVEL SCANF_DBL
# ifndef _FORMAT_DEFAULT_DOUBLE
#  define vfscanf __d_vfscanf
# endif
#endif

#if defined(STRTOF)
# define _NEED_IO_FLOAT
#elif defined(STRTOD)
# define _NEED_IO_DOUBLE
#elif defined(STRTOLD)
# define _NEED_IO_DOUBLE
# define _NEED_IO_LONG_DOUBLE
#elif SCANF_LEVEL == SCANF_MIN
# define _NEED_IO_SHRINK
# if defined(_WANT_MINIMAL_IO_LONG_LONG) && __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# ifdef _WANT_IO_C99_FORMATS
#  define _NEED_IO_C99_FORMATS
# endif
#elif SCANF_LEVEL == SCANF_STD
# define _NEED_IO_BRACKET
# ifdef _WANT_IO_POS_ARGS
#  define _NEED_IO_POS_ARGS
# endif
# if defined(_WANT_IO_LONG_LONG) && __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# ifdef _WANT_IO_C99_FORMATS
#  define _NEED_IO_C99_FORMATS
# endif
# ifdef _WANT_IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif
int vfscanf (FILE * stream, const char *fmt, va_list ap) __attribute__((weak));
#elif SCANF_LEVEL == SCANF_LLONG
# define _NEED_IO_BRACKET
# ifdef _WANT_IO_POS_ARGS
#  define _NEED_IO_POS_ARGS
# endif
# if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# ifdef _WANT_IO_C99_FORMATS
#  define _NEED_IO_C99_FORMATS
# endif
# ifdef _WANT_IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif
#elif SCANF_LEVEL == SCANF_FLT
# define _NEED_IO_BRACKET
# if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# define _NEED_IO_POS_ARGS
# define _NEED_IO_C99_FORMATS
# ifdef _WANT_IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif
# define _NEED_IO_FLOAT
#elif SCANF_LEVEL == SCANF_DBL
# define _NEED_IO_BRACKET
# if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# define _NEED_IO_POS_ARGS
# define _NEED_IO_C99_FORMATS
# ifdef _WANT_IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif
# define _NEED_IO_DOUBLE
# if defined(_WANT_IO_LONG_DOUBLE) && __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__
#  define _NEED_IO_LONG_DOUBLE
# endif
#else
# error	 "Not a known scanf level."
#endif

typedef unsigned int width_t;

#define FL_STAR	    0x01	/* '*': skip assignment		*/
#define FL_WIDTH    0x02	/* width is present		*/
#define FL_LONG	    0x04	/* 'long' type modifier		*/
#if defined(_NEED_IO_LONG_LONG) || defined(_NEED_IO_LONG_DOUBLE)
#define FL_LONGLONG 0x08        /* 'long long' type modifier    */
#else
#define FL_LONGLONG 0x00        /* 'long long' type modifier    */
#endif
#define FL_CHAR	    0x10	/* 'char' type modifier		*/
#define FL_SHORT    0x20	/* 'short' type modifier	*/
#define FL_MINUS    0x40	/* minus flag (field or value)	*/
#define FL_ANY	    0x80	/* any digit was read	        */
#define FL_OVFL	    0x100	/* significand overflowed       */
#define FL_DOT	    0x200	/* decimal '.' was	        */
#define FL_MEXP	    0x400 	/* exponent 'e' is neg.	        */
#define FL_FHEX     0x800       /* hex significand              */

#endif /* _SCANF_PRIVATE_H_ */
