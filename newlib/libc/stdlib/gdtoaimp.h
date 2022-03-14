/****************************************************************

The author of this software is David M. Gay.

Copyright (C) 1998-2000 by Lucent Technologies
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

/* This is a variation on dtoa.c that converts arbitary binary
   floating-point formats to and from decimal notation.  It uses
   double-precision arithmetic internally, so there are still
   various #ifdefs that adapt the calculations to the native
   double-precision arithmetic (any of IEEE, VAX D_floating,
   or IBM mainframe arithmetic).

   Please send bug reports to David M. Gay (dmg at acm dot org,
   with " at " changed at "@" and " dot " changed to ".").
 */

/* On a machine with IEEE extended-precision registers, it is
 * necessary to specify double-precision (53-bit) rounding precision
 * before invoking strtod or dtoa.  If the machine uses (the equivalent
 * of) Intel 80x87 arithmetic, the call
 *	_control87(PC_53, MCW_PC);
 * does this with many compilers.  Whether this or another call is
 * appropriate depends on the compiler; for this to work, it may be
 * necessary to #include "float.h" or another system-dependent header
 * file.
 */

/* strtod for IEEE-, VAX-, and IBM-arithmetic machines.
 *
 * This strtod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 112-126].
 *
 * Modifications:
 *
 *	1. We only require IEEE, IBM, or VAX double-precision
 *		arithmetic (not IEEE double-extended).
 *	2. We get by with floating-point arithmetic in a case that
 *		Clinger missed -- when we're computing d * 10^n
 *		for a small integer d and the integer n is not too
 *		much larger than 22 (the maximum integer k for which
 *		we can represent 10^k exactly), we may be able to
 *		compute (d*10^k) * 10^(e-k) with just one roundoff.
 *	3. Rather than a bit-at-a-time adjustment of the binary
 *		result in the hard case, we use floating-point
 *		arithmetic to determine the adjustment to within
 *		one bit; only in really hard cases do we need to
 *		compute a second residual.
 *	4. Because of 3., we don't need a large table of powers of 10
 *		for ten-to-e (just some small tables, e.g. of 10^k
 *		for 0 <= k <= 22).
 */

/*
 * #define IEEE_8087 for IEEE-arithmetic machines where the least
 *	significant byte has the lowest address.
 * #define IEEE_MC68k for IEEE-arithmetic machines where the most
 *	significant byte has the lowest address.
 * #define Long int on machines with 32-bit ints and 64-bit longs.
 * #define Sudden_Underflow for IEEE-format machines without gradual
 *	underflow (i.e., that flush to zero on underflow).
 * #define IBM for IBM mainframe-style floating-point arithmetic.
 * #define VAX for VAX-style floating-point arithmetic (D_floating).
 * #define No_leftright to omit left-right logic in fast floating-point
 *	computation of dtoa and gdtoa.  This will cause modes 4 and 5 to be
 *	treated the same as modes 2 and 3 for some inputs.
 * #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3.
 * #define RND_PRODQUOT to use rnd_prod and rnd_quot (assembly routines
 *	that use extended-precision instructions to compute rounded
 *	products and quotients) with IBM.
 * #define ROUND_BIASED for IEEE-format with biased rounding and arithmetic
 *	that rounds toward +Infinity.
 * #define ROUND_BIASED_without_Round_Up for IEEE-format with biased
 *	rounding when the underlying floating-point arithmetic uses
 *	unbiased rounding.  This prevent using ordinary floating-point
 *	arithmetic when the result could be computed with one rounding error.
 * #define Inaccurate_Divide for IEEE-format with correctly rounded
 *	products but inaccurate quotients, e.g., for Intel i860.
 * #define NO_LONG_LONG on machines that do not have a "long long"
 *	integer type (of >= 64 bits).  On such machines, you can
 *	#define Just_16 to store 16 bits per 32-bit Long when doing
 *	high-precision integer arithmetic.  Whether this speeds things
 *	up or slows things down depends on the machine and the number
 *	being converted.  If long long is available and the name is
 *	something other than "long long", #define Llong to be the name,
 *	and if "unsigned Llong" does not work as an unsigned version of
 *	Llong, #define #ULLong to be the corresponding unsigned type.
 * #define KR_headers for old-style C function headers.
 * #define Bad_float_h if your system lacks a float.h or if it does not
 *	define some or all of DBL_DIG, DBL_MAX_10_EXP, DBL_MAX_EXP,
 *	FLT_RADIX, FLT_ROUNDS, and DBL_MAX.
 * #define MALLOC your_malloc, where your_malloc(n) acts like malloc(n)
 *	if memory is available and otherwise does something you deem
 *	appropriate.  If MALLOC is undefined, malloc will be invoked
 *	directly -- and assumed always to succeed.  Similarly, if you
 *	want something other than the system's free() to be called to
 *	recycle memory acquired from MALLOC, #define FREE to be the
 *	name of the alternate routine.  (FREE or free is only called in
 *	pathological cases, e.g., in a gdtoa call after a gdtoa return in
 *	mode 3 with thousands of digits requested.)
 * #define Omit_Private_Memory to omit logic (added Jan. 1998) for making
 *	memory allocations from a private pool of memory when possible.
 *	When used, the private pool is PRIVATE_MEM bytes long:  2304 bytes,
 *	unless #defined to be a different length.  This default length
 *	suffices to get rid of MALLOC calls except for unusual cases,
 *	such as decimal-to-binary conversion of a very long string of
 *	digits.  When converting IEEE double precision values, the
 *	longest string gdtoa can return is about 751 bytes long.  For
 *	conversions by strtod of strings of 800 digits and all gdtoa
 *	conversions of IEEE doubles in single-threaded executions with
 *	8-byte pointers, PRIVATE_MEM >= 7400 appears to suffice; with
 *	4-byte pointers, PRIVATE_MEM >= 7112 appears adequate.
 * #define NO_INFNAN_CHECK if you do not wish to have INFNAN_CHECK
 *	#defined automatically on IEEE systems.  On such systems,
 *	when INFNAN_CHECK is #defined, strtod checks
 *	for Infinity and NaN (case insensitively).
 *	When INFNAN_CHECK is #defined and No_Hex_NaN is not #defined,
 *	strtodg also accepts (case insensitively) strings of the form
 *	NaN(x), where x is a string of hexadecimal digits (optionally
 *	preceded by 0x or 0X) and spaces; if there is only one string
 *	of hexadecimal digits, it is taken for the fraction bits of the
 *	resulting NaN; if there are two or more strings of hexadecimal
 *	digits, each string is assigned to the next available sequence
 *	of 32-bit words of fractions bits (starting with the most
 *	significant), right-aligned in each sequence.
 *	Unless GDTOA_NON_PEDANTIC_NANCHECK is #defined, input "NaN(...)"
 *	is consumed even when ... has the wrong form (in which case the
 *	"(...)" is consumed but ignored).
 * #define MULTIPLE_THREADS if the system offers preemptively scheduled
 *	multiple threads.  In this case, you must provide (or suitably
 *	#define) two locks, acquired by ACQUIRE_DTOA_LOCK(n) and freed
 *	by FREE_DTOA_LOCK(n) for n = 0 or 1.  (The second lock, accessed
 *	in pow5mult, ensures lazy evaluation of only one copy of high
 *	powers of 5; omitting this lock would introduce a small
 *	probability of wasting memory, but would otherwise be harmless.)
 *	You must also invoke freedtoa(s) to free the value s returned by
 *	dtoa.  You may do so whether or not MULTIPLE_THREADS is #defined.
 * #define IMPRECISE_INEXACT if you do not care about the setting of
 *	the STRTOG_Inexact bits in the special case of doing IEEE double
 *	precision conversions (which could also be done by the strtod in
 *	dtoa.c).
 * #define NO_HEX_FP to disable recognition of C9x's hexadecimal
 *	floating-point constants.
 * #define -DNO_ERRNO to suppress setting errno (in strtod.c and
 *	strtodg.c).
 * #define NO_STRING_H to use private versions of memcpy.
 *	On some K&R systems, it may also be necessary to
 *	#define DECLARE_SIZE_T in this case.
 * #define USE_LOCALE to use the current locale's decimal_point value.
 */

#ifndef GDTOAIMP_H_INCLUDED
#define GDTOAIMP_H_INCLUDED
#include "mprec.h"
#include "gdtoa.h"

#ifndef __SINGLE_THREAD__
#define MULTIPLE_THREADS
#endif

#define dtoa __dtoa
#define gdtoa __gdtoa
#define freedtoa __freedtoa

#define dtoa_result __dtoa_result_D2A
#define nrv_alloc __nrv_alloc_D2A
#define quorem __quorem_D2A
#define rshift __rshift_D2A
#define rv_alloc __rv_alloc_D2A
#define trailz __trailz_D2A

extern char *dtoa_result;
extern char *nrv_alloc ANSI((struct _reent *, char*, char **, int));
extern int quorem ANSI((Bigint*, Bigint*));
extern void rshift ANSI((Bigint*, int));
extern char *rv_alloc ANSI((struct _reent *, int));
extern int trailz ANSI((Bigint*));

#endif /* GDTOAIMP_H_INCLUDED */
