/****************************************************************
Copyright (C) 1997, 1999, 2001 Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
****************************************************************/

/* This implements most of ANSI C's printf, fprintf, and sprintf,
 * with %.0g and %.0G giving the shortest decimal string
 * that rounds to the number being converted, and with negative
 * precisions allowed for %f.
 */

/*
 * Extracted from the AMPL solvers library module
 * http://www.netlib.org/ampl/solvers/printf.c
 * and modified for use in libmingwex.a.
 *
 * libstdc++  amd libgfortran expect an snprintf that can handle host
 * widest float type.  This one handle 80 bit long double.  Printing
 * to streams using this alternative implementation is not yet
 * supported,
 *    
 *   Danny Smith  <dannysmith@users.sourceforge.net>
 *   2007-06-01
 */
   

#ifdef KR_headers
#include "varargs.h"
#else
#include "stddef.h"
#include "stdarg.h"
#include "stdlib.h"
#endif

#include <stdio.h>
#include "string.h"
#include "errno.h"

#ifdef KR_headers
#define Const /* const */
#define Voidptr char*
#ifndef size_t__
#define size_t int
#define size_t__
#endif

#else

#define Const const
#define Voidptr void*

#endif



#ifdef USE_FILE_OUTPUT 
#undef MESS
#ifndef Stderr
#define Stderr stderr
#endif

#ifdef _windows_
#undef PF_BUF
#define MESS
#include "mux0.h"
#define stdout_or_err(f) (f == stdout)
#else
#define stdout_or_err(f) (f == Stderr || f == stdout)
#endif

#endif /* USE_FILE_OUTPUT */

#include <math.h>
#include <stdint.h>
#include "gdtoaimp.h"
#ifdef USE_LOCALE
#include "locale.h"
#endif

/*
 * For a MinGW build, we provide the implementation dependent entries
 * `__mingw_snprintf' and `__mingw_vsnprintf', then alias them to provide
 * the C99 conforming implementations of `snprintf()' and `vsnprintf()'.
 */
#  define Snprintf   __mingw_snprintf
#  define Vsnprintf  __mingw_vsnprintf

int __cdecl __MINGW_NOTHROW
snprintf(char *, size_t, const char *, ...) __attribute__((alias("__mingw_snprintf")));
int __cdecl __MINGW_NOTHROW
vsnprintf (char *, size_t, const char *, __VALIST) __attribute__((alias("__mingw_vsnprintf")));


static char* __ldtoa  (long double ld, int mode, int ndig, int *decpt,
			int *sign, char **rve)
{

	static FPI fpi = { 64, 1-16383-64+1, 32766 - 16383 - 64 + 1, 1, 0 };
	ULong bits[2];
	int ex, kind;
	int fptype = __fpclassifyl (ld);
	union
	{
 	  unsigned short L[6];
 	  long double ld;
	} u;

        u.ld = ld;
          
	*sign = u.L[4] & 0x8000;
        ex = u.L[4] & 0x7fff;

	bits[1] = (u.L[3] << 16) | u.L[2];
	bits[0] = (u.L[1] << 16) | u.L[0];

	if (fptype & FP_NAN) /* NaN or Inf */
	  {
 	    if (fptype & FP_NORMAL)
	      kind = STRTOG_Infinite;
	    else
	      kind = STRTOG_NaN;
	  }
	else if (fptype & FP_NORMAL) /* Normal or subnormal */
	  {
	    if  (fptype & FP_ZERO)
	      {
		kind = STRTOG_Denormal;
		ex = 1;
	      }
	    else
	      kind = STRTOG_Normal;

	    ex -= 0x3fff + 63;
	  }
	else
 	    kind = STRTOG_Zero;

   return __gdtoa (&fpi, ex, bits, &kind, mode, ndig, decpt, rve);
}

#ifdef USE_ULDIV
/* This is for avoiding 64-bit divisions on the DEC Alpha, since */
/* they are not portable among variants of OSF1 (DEC's Unix). */

#define ULDIV(a,b) uldiv_ASL(a,(unsigned long)(b))

#ifndef LLBITS
#define LLBITS 6
#endif
#ifndef ULONG
#define ULONG unsigned long
#endif

 static int
klog(ULONG x)
{
	int k, rv = 0;

	if (x > 1L)
	    for(k = 1 << LLBITS-1;;) {
		if (x >= (1L << k)) {
			rv |= k;
			x >>= k;
			}
		if (!(k >>= 1))
			break;
		}
	return rv;
	}

 ULONG
uldiv_ASL(ULONG a, ULONG b)
{
	int ka;
	ULONG c, k;
	static ULONG b0;
	static int kb;

	if (a < b)
		return 0;
	if (b != b0) {
		b0 = b;
		kb = klog(b);
		}
	k = 1;
	if ((ka = klog(a) - kb) > 0) {
		k <<= ka;
		b <<= ka;
		}
	c = 0;
	for(;;) {
		if (a >= b) {
			a -= b;
			c |= k;
			}
		if (!(k >>= 1))
			break;
		a <<= 1;
		}
	return c;
	}

#else
#define ULDIV(a,b) a / b
#endif /* USE_ULDIV */

 typedef struct
Finfo {
	union {
		#ifdef USE_FILE_OUTPUT
		 FILE *cf;
		#endif
		 char *sf;
	      } u;
	char *ob0, *obe1;
	size_t lastlen;
	} Finfo;

 typedef char *(*Putfunc) ANSI((Finfo*, int*));

#ifdef USE_FILE_OUTPUT 
#ifdef PF_BUF
FILE *stderr_ASL = (FILE*)&stderr_ASL;
void (*pfbuf_print_ASL) ANSI((char*));
char *pfbuf_ASL;
static char *pfbuf_next;
static size_t pfbuf_len;
extern Char *mymalloc_ASL ANSI((size_t));
extern Char *myralloc_ASL ANSI((void *, size_t));

#undef fflush
#ifdef old_fflush_ASL
#define fflush old_fflush_ASL
#endif

 void
fflush_ASL(FILE *f)
{
	if (f == stderr_ASL) {
		if (pfbuf_ASL && pfbuf_print_ASL) {
			(*pfbuf_print_ASL)(pfbuf_ASL);
			free(pfbuf_ASL);
			pfbuf_ASL = 0;
			}
		}
	else
		fflush(f);
	}

 static void
pf_put(char *buf, int len)
{
	size_t x, y;
	if (!pfbuf_ASL) {
		x = len + 256;
		if (x < 512)
			x = 512;
		pfbuf_ASL = pfbuf_next = (char*)mymalloc_ASL(pfbuf_len = x);
		}
	else if ((y = (pfbuf_next - pfbuf_ASL) + len) >= pfbuf_len) {
		x = pfbuf_len;
		while((x <<= 1) <= y);
		y = pfbuf_next - pfbuf_ASL;
		pfbuf_ASL = (char*)myralloc_ASL(pfbuf_ASL, x);
		pfbuf_next = pfbuf_ASL + y;
		pfbuf_len = x;
		}
	memcpy(pfbuf_next, buf, len);
	pfbuf_next += len;
	*pfbuf_next = 0;
	}

 static char *
pfput(Finfo *f, int *rvp)
{
	int n;
	char *ob0 = f->ob0;
	*rvp += n = (int)(f->obe1 - ob0);
	pf_put(ob0, n);
	return ob0;
	}
#endif /* PF_BUF */

 static char *
Fput
#ifdef KR_headers
	(f, rvp) register Finfo *f; int *rvp;
#else
	(register Finfo *f, int *rvp)
#endif
{
	register char *ob0 = f->ob0;

	*rvp += f->obe1 - ob0;
	*f->obe1 = 0;
	fputs(ob0, f->u.cf);
	return ob0;
	}


#ifdef _windows_
int stdout_fileno_ASL = 1;

 static char *
Wput
#ifdef KR_headers
	(f, rvp) register Finfo *f; int *rvp;
#else
	(register Finfo *f, int *rvp)
#endif
{
	register char *ob0 = f->ob0;

	*rvp += f->obe1 - ob0;
	*f->obe1 = 0;
	mwrite(ob0, f->obe1 - ob0);
	return ob0;
	}
#endif /*_windows_*/
#endif /* USE_FILE_OUTPUT */

#ifndef INT_IS_LONG
#if defined (__SIZEOF_LONG__) && defined (__SIZEOF_INT__) \
  && (__SIZEOF_LONG__) == (__SIZEOF_INT__)	
#define INT_IS_LONG 1
#endif
#endif

#define put(x) { *outbuf++ = x; if (outbuf == obe) outbuf = (*fput)(f,&rv); }

 static int
x_sprintf
#ifdef KR_headers
	(obe, fput, f, fmt, ap)
	char *obe, *fmt; Finfo *f; Putfunc fput; va_list ap;
#else
	(char *obe, Putfunc fput, Finfo *f, const char *fmt, va_list ap)
#endif
{
	char *digits, *ob0, *outbuf, *s, *s0, *se;
	Const char *fmt0;
	char buf[32];
	long long i = 0;
	unsigned long long j;
	unsigned long long u = 0;

        double x;
	long double  xx;
	int flag_ld = 0;
	int alt, base, c, decpt, dot, conv, i1, lead0, left,
	    prec, prec1, psign, rv, sgn, sign, width;
	enum {
	  LEN_I,
	  LEN_L,
	  LEN_S, 
	  LEN_LL
	} len;
	long long Ltmp;
        intptr_t ip;
	short sh;
	long k;
	unsigned short us;
	unsigned long ui;
	static char hex[] = "0123456789abcdef";
	static char Hex[] = "0123456789ABCDEF";

#ifdef USE_LOCALE
	char decimalpoint = *localeconv()->decimal_point;
#else
	static const char decimalpoint = '.';
#endif

	ob0 = outbuf = f->ob0;
	rv = 0;
	for(;;) {
		for(;;) {
			switch(c = *fmt++) {
				case 0:
					goto done;
				case '%':
					break;
				default:
					put(c)
					continue;
				}
			break;
			}
		alt=dot=lead0=left=prec=psign=sign=width=0;
		len = LEN_I;
		fmt0 = fmt;
 fmtloop:
		switch(conv = *fmt++) {
			case ' ':
			case '+':
				sign = conv;
				goto fmtloop;
			case '-':
				if (dot)
					psign = 1;
				else
					left = 1;
				goto fmtloop;
			case '#':
				alt = 1;
				goto fmtloop;
			case '0':
				if (!lead0 && !dot) {
					lead0 = 1;
					goto fmtloop;
					}
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				k = conv - '0';
				while((c = *fmt) >= '0' && c <= '9') {
					k = 10*k + c - '0';
					fmt++;
					}
				if (dot)
					prec = psign ? -k : k;
				else
					width = k;
				goto fmtloop;
			case 'h':
				len = LEN_S;
				goto fmtloop;
#ifndef NO_MSVC_EXTENSIONS
			case 'I':
				if (fmt[0] == '3' && fmt[1] == '2')
				   {
 				      fmt += 2;
				      len = LEN_L;
				   }
				else if (fmt[0] == '6' && fmt[1] == '4')
				   {
 				      fmt += 2;
				      len = LEN_LL;
				   }
				else
				  len = sizeof (intptr_t) == sizeof (long long)
					? LEN_LL : LEN_L;
				goto fmtloop;
#endif
			case 'l':
				if (fmt[0] == 'l')
				  {
				    fmt++;
				    len = LEN_LL;
				  }
  				else
				  len = LEN_L;
				goto fmtloop;
			case 'L':
				flag_ld++;
				goto fmtloop;				
			case '.':
				dot = 1;
				lead0 = 0;
				goto fmtloop;
			case '*':
				k = va_arg(ap, int);
				if (dot)
					prec = k;
				else {
					if (k < 0) {
						sign = '-';
						k = -k;
						}
					width = k;
					}
				goto fmtloop;
			case 'c':
/* %lc (for wctomb conversion) is not implemented.  */
				c = va_arg(ap, int);
				put(c)
				continue;
			case '%':
				put(conv)
				continue;
			case 'u':
				switch(len) {
				  case LEN_I:
#if !INT_IS_LONG
					ui = va_arg(ap, int);
					i = ui;
					break;
#endif
				  case LEN_L:
					ui = va_arg(ap, long);
					i = ui;
					break;
				  case LEN_S:
					us = va_arg(ap, long);
					i = us;
					break;
				  case LEN_LL:
					i = va_arg(ap, long long);
				  }
				sign = 0;
				goto have_i;
			case 'i':
			case 'd':
				switch(len) {
				  case LEN_I:
#if !INT_IS_LONG
					k = va_arg(ap, int);
					i = k;
					break;
#endif
				  case LEN_L:
					k = va_arg(ap, long);
					i = k;
					break;
				  case LEN_S:
					sh = va_arg(ap, long);
					i = sh;
					break;
				  case LEN_LL:
					i = va_arg(ap, long long);

				  }
				if (i < 0) {
					sign = '-';
					i = -i;
					}
 have_i:
				base = 10;
				u = i;
				digits = hex;
 baseloop:
				s = buf;
				if (!u)
					alt = 0;
				do {
					j = ULDIV(u, base);
					*s++ = digits[u - base * j];
					}
					while((u = j));
				prec -= c = s - buf;
				if (alt && conv == 'o' && prec <= 0)
					prec = 1;
				if ((width -= c) > 0) {
					if (prec > 0)
						width -= prec;
					if (sign)
						width--;
					if (alt == 2)
						width--;
					}
				if (left) {
					if (alt == 2)
						put('0') /* for 0x */
					if (sign)
						put(sign)
					while(--prec >= 0)
						put('0')
					do put(*--s)
						while(s > buf);
					while(--width >= 0)
						put(' ')
					continue;
					}
				if (width > 0) {
					if (lead0) {
						if (alt == 2)
							put('0')
						if (sign)
							put(sign)
						while(--width >= 0)
							put('0')
						goto s_loop;
						}
					else
						while(--width >= 0)
							put(' ')
					}
				if (alt == 2)
					put('0')
				if (sign)
					put(sign)
 s_loop:
				while(--prec >= 0)
					put('0')
				do put(*--s)
					while(s > buf);
				continue;
			case 'n':
				ip = va_arg(ap, intptr_t);
				if (!ip)
					ip = (intptr_t) &Ltmp;
				c = outbuf - ob0 + rv;
				switch(len) {
				  case LEN_I:
#if !INT_IS_LONG
					*(int*)ip = c;
					break;
#endif
				  case LEN_L:
					*(long*)ip = c;
					break;
				  case LEN_S:
					*(short*)ip = c;
					break;
				  case LEN_LL:
					*(long long*) ip = c;
					break;
				  }
				break;
			case 'p':
				alt = 1;
				len = sizeof (intptr_t) == sizeof (long long)
				      ? LEN_LL : LEN_L;
				/* no break */
			case 'x':
				digits = hex;
				goto more_x;
			case 'X':
				digits = Hex;
 more_x:
				if (alt) {
					alt = 2;
					sign = conv;
					}
				else
					sign = 0;
				base = 16;
 get_u:
				switch(len) {
				  case LEN_I:
#if !INT_IS_LONG
					ui = va_arg(ap, int);
					u = ui;
					break;
#endif
				  case LEN_L:
					ui = va_arg(ap, long);
					u = ui;
					break;
				  case LEN_S:
					us = va_arg(ap, long);
					u = us;
					break;
				  case LEN_LL:
				   	u = va_arg(ap, long long);
				  }
				if (!u)
					sign = alt = 0;
				goto baseloop;
			case 'o':
				base = 8;
				digits = hex;
				goto get_u;
			case 's':
/* %ls (for wctombs conversion) is not implemented.  */
				s0 = 0;
				s = va_arg(ap, char*);
				if (!s)
					s = "<NULL>";
				if (prec < 0)
					prec = 0;
 have_s:
				if (dot) {
					for(c = 0; c < prec; c++)
						if (!s[c])
							break;
					prec = c;
					}
				else
					prec = strlen(s);
				width -= prec;
				if (!left)
					while(--width >= 0)
						put(' ')
				while(--prec >= 0)
					put(*s++)
				while(--width >= 0)
					put(' ')
				if (s0)
					__freedtoa(s0);
				continue;
			case 'f':
				if (!dot)
					prec = 6;
				if (flag_ld)
				  xx =  va_arg(ap, long double);
				else
				  {
				    x = va_arg(ap, double);
				    xx = x;
				  }

 				s = s0 = __ldtoa(xx, 3, prec, &decpt, &sgn, &se);
				if (decpt == -32768) {
 fmt9999:
					dot = prec = alt = 0;
					if (*s == 'N' || *s == 'I')
						goto have_s;
					decpt = strlen(s);
					}
 f_fmt:
				if (sgn && (xx||sign))
					sign = '-';
				if (prec > 0)
					width -= prec;
				if (width > 0) {
					if (sign)
						--width;
					if (decpt <= 0) {
						--width;
						if (prec > 0)
							--width;
						}
					else {
						if (s == se)
							decpt = 1;
						width -= decpt;
						if (prec > 0 || alt)
							--width;
						}
					}
				if (width > 0 && !left) {
					if (lead0) {
						if (sign)
							put(sign)
						sign = 0;
						do put('0')
							while(--width > 0);
						}
					else do put(' ')
						while(--width > 0);
					}
				if (sign)
					put(sign)
				if (decpt <= 0) {
					put('0')
					if (prec > 0 || alt)
						put(decimalpoint)
					while(decpt < 0) {
						put('0')
						prec--;
						decpt++;
						}
					}
				else {
					do {
						if ((c = *s))
							s++;
						else
							c = '0';
						put(c)
						}
						while(--decpt > 0);
					if (prec > 0 || alt)
						put(decimalpoint)
					}
				while(--prec >= 0) {
					if ((c = *s))
						s++;
					else
						c = '0';
					put(c)
					}
				while(--width >= 0)
					put(' ')
				__freedtoa(s0);
				continue;
			case 'G':
			case 'g':
				if (!dot)
					prec = 6;
				if (flag_ld)
				  xx =  va_arg(ap, long double);
				else
				  {
				    x = va_arg(ap, double);
				    xx = x;
				  }
				if (prec < 0)
					prec = 0;
				s = s0 = __ldtoa(xx, prec ? 2 : 0, prec, &decpt,
					&sgn, &se);
				if (decpt == -32768)
					goto fmt9999;
				c = se - s;
				prec1 = prec;
				if (!prec) {
					prec = c;
					prec1 = c + (s[1] || alt ? 5 : 4);
					/* %.0g gives 10 rather than 1e1 */
					}
				if (decpt > -4 && decpt <= prec1) {
					if (alt)
						prec -= decpt;
					else
						prec = c - decpt;
					if (prec < 0)
						prec = 0;
					goto f_fmt;
					}
				conv -= 2;
				if (!alt && prec > c)
					prec = c;
				--prec;
				goto e_fmt;
			case 'e':
			case 'E':
				if (!dot)
					prec = 6;
				if (flag_ld)
				  xx =  va_arg(ap, long double);
				else
				  {
				    x = va_arg(ap, double);
				    xx = x;
				  }
				if (prec < 0)
					prec = 0;
				s = s0 = __ldtoa(xx, prec ? 2 : 0, prec + 1, &decpt,
					&sgn, &se);
				if (decpt == -32768)
					goto fmt9999;
 e_fmt:
				if (sgn && (xx||sign))
					sign = '-';
				if ((width -= prec + 5) > 0) {
					if (sign)
						--width;
					if (prec || alt)
						--width;
					}
				
				if ((c = --decpt) < 0)
					c = -c;
				
				while(c >= 100) {
					--width;
					c /= 10;
					}
				if (width > 0 && !left) {
					if (lead0) {
						if (sign)
							put(sign)
						sign = 0;
						do put('0')
							while(--width > 0);
						}
					else do put(' ')
						while(--width > 0);
					}
				if (sign)
					put(sign)
				put(*s++)
				if (prec || alt)
					put(decimalpoint)
				while(--prec >= 0) {
					if ((c = *s))
						s++;
					else
						c = '0';
					put(c)
					}
				put(conv)
				if (decpt < 0) {
					put('-')
					decpt = -decpt;
					}
				else
					put('+')
				for(c = 2, k = 10; 10 * k <= decpt; c++, k *= 10);
				for(;;) {
					i1 = decpt / k;
					put(i1 + '0')
					if (--c <= 0)
						break;
					decpt -= i1*k;
					decpt *= 10;
					}
				while(--width >= 0)
					put(' ')
				__freedtoa(s0);
				continue;
			default:
				put('%')
				while(fmt0 < fmt)
					put(*fmt0++)
				continue;
			}
		}
 done:
	*outbuf = 0;
	return (f->lastlen = outbuf - ob0) + rv;
	}

#define Bsize 256
#ifdef USE_FILE_OUTPUT
 int
Printf
#ifdef KR_headers
	(va_alist)
 va_dcl
{
	char *fmt;

	va_list ap;
	int rv;
	Finfo f;
	char buf[Bsize];

	va_start(ap);
	fmt = va_arg(ap, char*);
	/*}*/
#else
	(const char *fmt, ...)
{
	va_list ap;
	int rv;
	Finfo f;
	char buf[Bsize];

	va_start(ap, fmt);
#endif
	f.u.cf = stdout;
	f.ob0 = buf;
	f.obe1 = buf + Bsize - 1;
#ifdef _windows_
	if (fileno(stdout) == stdout_fileno_ASL) {
		rv = x_sprintf(f.obe1, Wput, &f, fmt, ap);
		mwrite(buf, f.lastlen);
		}
	else
#endif
#ifdef PF_BUF
	if (stdout == stderr_ASL) {
		rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
		pf_put(buf, f.lastlen);
		}
	else
#endif /* PF_BUF */
		{
		rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
		fputs(buf, stdout);
		}
	va_end(ap);
	return rv;
	}

 static char *
Sput
#ifdef KR_headers
	(f, rvp) Finfo *f; int *rvp;
#else
	(Finfo *f, int *rvp)
#endif
{
	if (Printf("\nBUG! Sput called!\n", f, rvp))
		/* pass vp, rvp and return 0 to shut diagnostics off */
		exit(250);
	return 0;
	}

 int
Sprintf
#ifdef KR_headers
	(va_alist)
 va_dcl
{
	char *s, *fmt;
	va_list ap;
	int rv;
	Finfo f;

	va_start(ap);
	s = va_arg(ap, char*);
	fmt = va_arg(ap, char*);
	/*}*/
#else
	(char *s, const char *fmt, ...)
{
	va_list ap;
	int rv;
	Finfo f;

	va_start(ap, fmt);
#endif
	f.ob0 = s;
	rv = x_sprintf(s, Sput, &f, fmt, ap);
	va_end(ap);
	return rv;
	}

int
Fprintf
#ifdef KR_headers
	(va_alist)
 va_dcl
{
	FILE *F;
	char *s, *fmt;
	va_list ap;
	int rv;
	Finfo f;
	char buf[Bsize];

	va_start(ap);
	F = va_arg(ap, FILE*);
	fmt = va_arg(ap, char*);
	/*}*/
#else
	(FILE *F, const char *fmt, ...)
{
	va_list ap;
	int rv;
	Finfo f;
	char buf[Bsize];

	va_start(ap, fmt);
#endif
	f.u.cf = F;
	f.ob0 = buf;
	f.obe1 = buf + Bsize - 1;
#ifdef MESS
	if (stdout_or_err(F)) {
#ifdef _windows_
		if (fileno(stdout) == stdout_fileno_ASL) {
			rv = x_sprintf(f.obe1, Wput, &f, fmt, ap);
			mwrite(buf, f.lastlen);
			}
		else
#endif
#ifdef PF_BUF
		if (F == stderr_ASL) {
			rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
			pf_put(buf, f.lastlen);
			}
		else
#endif
			{
			rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
			fputs(buf, F);
			}
		}
	else
#endif /*MESS*/
		{
#ifdef PF_BUF
		if (F == stderr_ASL) {
			rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
			pf_put(buf, f.lastlen);
			}
	else
#endif
			{
			rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
			fputs(buf, F);
			}
		}
	va_end(ap);
	return rv;
	}

 int
Vsprintf
#ifdef KR_headers
	(s, fmt, ap) char *s, *fmt; va_list ap;
#else
	(char *s, const char *fmt, va_list ap)
#endif
{
	Finfo f;
	return x_sprintf(f.ob0 = s, Sput, &f, fmt, ap);
	}

 int
Vfprintf
#ifdef KR_headers
	(F, fmt, ap) FILE *F; char *fmt; va_list ap;
#else
	(FILE *F, const char *fmt, va_list ap)
#endif
{
	char buf[Bsize];
	int rv;
	Finfo f;

	f.u.cf = F;
	f.ob0 = buf;
	f.obe1 = buf + Bsize - 1;
#ifdef MESS
	if (stdout_or_err(F)) {
#ifdef _windows_
		if (fileno(stdout) == stdout_fileno_ASL) {
			rv = x_sprintf(f.obe1, Wput, &f, fmt, ap);
			mwrite(buf, f.lastlen);
			}
		else
#endif
#ifdef PF_BUF
		if (F == stderr_ASL) {
			rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
			pf_put(buf, f.lastlen);
			}
		else
#endif
			{
			rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
			fputs(buf, F);
			}
		}
	else
#endif /*MESS*/
		{
#ifdef PF_BUF
		if (F == stderr_ASL) {
			rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
			pf_put(buf, f.lastlen);
			}
		else
#endif
			{
			rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
			fputs(buf, F);
			}
		}
	va_end(ap);
	return rv;
	}

 void
Perror
#ifdef KR_headers
	(s) char *s;
#else
	(const char *s)
#endif
{
	if (s && *s)
		fprintf(Stderr, "%s: ", s);
	fprintf(Stderr, "%s\n", strerror(errno));
	}
#endif /* USE_FILE_OUTPUT */


 static char *
Snput
#ifdef KR_headers
	(f, rvp) Finfo *f; int *rvp;
#else
	(Finfo *f, int *rvp)
#endif
{
	char *s, *s0;
	size_t L;

	*rvp += Bsize;
	s0 = f->ob0;
	s = f->u.sf;
	if ((L = f->obe1 - s) > Bsize) {
		L = Bsize;
		goto copy;
		}
	if (L > 0) {
 copy:
		memcpy(s, s0, L);
		f->u.sf = s + L;
		}
	return s0;
	}


 int
Vsnprintf
#ifdef KR_headers
	(s, n, fmt, ap) char *s; size_t n; char *fmt; va_list ap;
#else
	(char *s, size_t n, const char *fmt, va_list ap)
#endif
{
	Finfo f;
	char buf[Bsize];
	int L, rv;

	if (n <= 0 || !s) {
		n = 1;
		s = buf;
		}
	f.u.sf = s;
	f.ob0 = buf;
	f.obe1 = s + n - 1;
	rv = x_sprintf(buf + Bsize, Snput, &f, fmt, ap);
	if (f.lastlen > (L = f.obe1 - f.u.sf))
		f.lastlen = L;
	if (f.lastlen > 0) {
		memcpy(f.u.sf, buf, f.lastlen);
		f.u.sf += f.lastlen;
		}
	*f.u.sf = 0;
	return rv;
	}
 int
Snprintf
#ifdef KR_headers
	(va_alist)
 va_dcl
{
	char *s, *fmt;
	int rv;
	size_t n;
	va_list ap;

	va_start(ap);
	s = va_arg(ap, char*);
	n = va_arg(ap, size_t);
	fmt = va_arg(ap, char*);
	/*}*/
#else
	(char *s, size_t n, const char *fmt, ...)
{
	int rv;
	va_list ap;

	va_start(ap, fmt);
#endif
	rv = Vsnprintf(s, n, fmt, ap);
	va_end(ap);
	return rv;
	}


#if (EXPORT_WEAK_SNPRINTF_ALIAS)
int __cdecl snprintf(char*, size_t, const char*  , ...) __attribute__ ((weak, alias ("__mingw_snprintf")));
int __cdecl vsnprintf (char*, size_t n, const char*, __gnuc_va_list) __attribute__ ((weak, alias ("__mingw_vsnprintf")));
#endif

