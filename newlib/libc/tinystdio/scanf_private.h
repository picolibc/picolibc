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
# define SCANF_LEVEL SCANF_FLT
# ifndef FORMAT_DEFAULT_DOUBLE
#  define vfscanf __d_vfscanf
# endif
#endif

#if	SCANF_LEVEL == SCANF_STD
# define SCANF_BRACKET	1
# define SCANF_FLOAT	0
int vfscanf (FILE * stream, const char *fmt, va_list ap) __attribute__((weak));
#elif	SCANF_LEVEL == SCANF_FLT
# define SCANF_BRACKET	1
# define SCANF_FLOAT	1
#else
# error	 "Not a known scanf level."
#endif

#define CASE_CONVERT    ('a' - 'A')
#define TOLOWER(c)        ((c) | CASE_CONVERT)

static inline int
ISSPACE(int c)
{
    return ('\011' <= c && c <= '\015') || c == ' ';
}

static inline int
ISDIGIT(int c)
{
    return '0' <= c && c <= '9';
}

typedef unsigned int width_t;

#define FL_STAR	    0x01	/* '*': skip assignment		*/
#define FL_WIDTH    0x02	/* width is present		*/
#define FL_LONG	    0x04	/* 'long' type modifier		*/
#define FL_LONGLONG 0x08        /* 'long long' type modifier    */
#define FL_CHAR	    0x10	/* 'char' type modifier		*/
#define FL_SHORT    0x20	/* 'short' type modifier	*/
#define FL_MINUS    0x40	/* minus flag (field or value)	*/
#define FL_ANY	    0x80	/* any digit was read	        */
#define FL_OVFL	    0x100	/* significand overflowed       */
#define FL_DOT	    0x200	/* decimal '.' was	        */
#define FL_MEXP	    0x400 	/* exponent 'e' is neg.	        */
#define FL_FHEX     0x800       /* hex significand              */

#ifndef	__AVR_HAVE_LPMX__
# if  defined(__AVR_ENHANCED__) && __AVR_ENHANCED__
#  define __AVR_HAVE_LPMX__	1
# endif
#endif

#ifndef	__AVR_HAVE_MOVW__
# if  defined(__AVR_ENHANCED__) && __AVR_ENHANCED__
#  define __AVR_HAVE_MOVW__	1
# endif
#endif

#endif /* _SCANF_PRIVATE_H_ */
