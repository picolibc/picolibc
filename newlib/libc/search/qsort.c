/*
FUNCTION
<<qsort>>---sort an array

INDEX
	qsort

ANSI_SYNOPSIS
	#include <stdlib.h>
	void qsort(void *<[base]>, size_t <[nmemb]>, size_t <[size]>,
		   int (*<[compar]>)(const void *, const void *) );

TRAD_SYNOPSIS
	#include <stdlib.h>
	qsort(<[base]>, <[nmemb]>, <[size]>, <[compar]> )
	char *<[base]>;
	size_t <[nmemb]>;
	size_t <[size]>;
	int (*<[compar]>)();

DESCRIPTION
<<qsort>> sorts an array (beginning at <[base]>) of <[nmemb]> objects.
<[size]> describes the size of each element of the array.

You must supply a pointer to a comparison function, using the argument
shown as <[compar]>.  (This permits sorting objects of unknown
properties.)  Define the comparison function to accept two arguments,
each a pointer to an element of the array starting at <[base]>.  The
result of <<(*<[compar]>)>> must be negative if the first argument is
less than the second, zero if the two arguments match, and positive if
the first argument is greater than the second (where ``less than'' and
``greater than'' refer to whatever arbitrary ordering is appropriate).

The array is sorted in place; that is, when <<qsort>> returns, the
array elements beginning at <[base]> have been reordered.

RETURNS
<<qsort>> does not return a result.

PORTABILITY
<<qsort>> is required by ANSI (without specifying the sorting algorithm).
*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <_ansi.h>
#include <sys/cdefs.h>
#include <stdlib.h>

#ifndef __GNUC__
#define inline
#endif

#if defined(I_AM_QSORT_R)
typedef int		 cmp_t(void *, const void *, const void *);
#elif defined(I_AM_GNU_QSORT_R)
typedef int		 cmp_t(const void *, const void *, void *);
#else
typedef int		 cmp_t(const void *, const void *);
#endif
static inline char	*med3 _PARAMS((char *, char *, char *, cmp_t *, void *));
static inline void	 swapfunc _PARAMS((char *, char *, int, int));

#define min(a, b)	(a) < (b) ? a : b

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#define swapcode(TYPE, parmi, parmj, n) { 		\
	long i = (n) / sizeof (TYPE); 			\
	TYPE *pi = (TYPE *) (parmi); 		\
	TYPE *pj = (TYPE *) (parmj); 		\
	do { 						\
		TYPE	t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
        } while (--i > 0);				\
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || \
	es % sizeof(long) ? 2 : es == sizeof(long)? 0 : 1;

static inline void
_DEFUN(swapfunc, (a, b, n, swaptype),
	char *a _AND
	char *b _AND
	int n _AND
	int swaptype)
{
	if(swaptype <= 1)
		swapcode(long, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define swap(a, b)					\
	if (swaptype == 0) {				\
		long t = *(long *)(a);			\
		*(long *)(a) = *(long *)(b);		\
		*(long *)(b) = t;			\
	} else						\
		swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) 	if ((n) > 0) swapfunc(a, b, n, swaptype)

#if defined(I_AM_QSORT_R)
#define	CMP(t, x, y) (cmp((t), (x), (y)))
#elif defined(I_AM_GNU_QSORT_R)
#define	CMP(t, x, y) (cmp((x), (y), (t)))
#else
#define	CMP(t, x, y) (cmp((x), (y)))
#endif

static inline char *
_DEFUN(med3, (a, b, c, cmp, thunk),
	char *a _AND
	char *b _AND
	char *c _AND
	cmp_t *cmp _AND
	void *thunk
#if !defined(I_AM_QSORT_R) && !defined(I_AM_GNU_QSORT_R)
__unused
#endif
)
{
	return CMP(thunk, a, b) < 0 ?
	       (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a ))
              :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c ));
}

#if defined(I_AM_QSORT_R)
void
_DEFUN(__bsd_qsort_r, (a, n, es, thunk, cmp),
	void *a _AND
	size_t n _AND
	size_t es _AND
	void *thunk _AND
	cmp_t *cmp)
#elif defined(I_AM_GNU_QSORT_R)
void
_DEFUN(qsort_r, (a, n, es, cmp, thunk),
	void *a _AND
	size_t n _AND
	size_t es _AND
	cmp_t *cmp _AND
	void *thunk)
#else
#define thunk NULL
void
_DEFUN(qsort, (a, n, es, cmp),
	void *a _AND
	size_t n _AND
	size_t es _AND
	cmp_t *cmp)
#endif
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	size_t d, r;
	int cmp_result;
	int swaptype, swap_cnt;

loop:	SWAPINIT(a, es);
	swap_cnt = 0;
	if (n < 7) {
		for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
			for (pl = pm; pl > (char *) a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}
	pm = (char *) a + (n / 2) * es;
	if (n > 7) {
		pl = a;
		pn = (char *) a + (n - 1) * es;
		if (n > 40) {
			d = (n / 8) * es;
			pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3(pm - d, pm, pm + d, cmp, thunk);
			pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3(pl, pm, pn, cmp, thunk);
	}
	swap(a, pm);
	pa = pb = (char *) a + es;

	pc = pd = (char *) a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swap_cnt = 1;
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swap_cnt = 1;
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		swap_cnt = 1;
		pb += es;
		pc -= es;
	}
	if (swap_cnt == 0) {  /* Switch to insertion sort */
		for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
			for (pl = pm; pl > (char *) a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}

	pn = (char *) a + n * es;
	r = min(pa - (char *)a, pb - pa);
	vecswap(a, pb - r, r);
	r = min(pd - pc, pn - pd - es);
	vecswap(pb, pn - r, r);
	if ((r = pb - pa) > es)
#if defined(I_AM_QSORT_R)
		__bsd_qsort_r(a, r / es, es, thunk, cmp);
#elif defined(I_AM_GNU_QSORT_R)
		qsort_r(a, r / es, es, cmp, thunk);
#else
		qsort(a, r / es, es, cmp);
#endif
	if ((r = pd - pc) > es) {
		/* Iterate rather than recurse to save stack space */
		a = pn - r;
		n = r / es;
		goto loop;
	}
/*		qsort(pn - r, r / es, es, cmp);*/
}
