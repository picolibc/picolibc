/*
FUNCTION
<<qsort>>---sort an array

INDEX
	qsort

SYNOPSIS
	#include <stdlib.h>
	void qsort(void *<[base]>, size_t <[nmemb]>, size_t <[size]>,
		   int (*<[compar]>)(const void *, const void *) );

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
#include <stdint.h>

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
static inline char	*med3 (char *, char *, char *, cmp_t *, void *);
static inline void	 swapfunc (char *, char *, size_t, int);

#define min(a, b)	(a) < (b) ? a : b

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#define swapcode(TYPE, parmi, parmj, n) { 		\
	size_t i = (n) / sizeof (TYPE); 			\
	TYPE *pi = (TYPE *) (parmi); 		\
	TYPE *pj = (TYPE *) (parmj); 		\
	do { 						\
		TYPE	t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
        } while (--i > 0);				\
}

#define SWAPINIT(a, es) swaptype = ((uintptr_t)(a) % sizeof(long)) ||   \
	((es) % sizeof(long)) ? 2 : ((es) == sizeof(long)) ? 0 : 1

static inline void
swapfunc (char *a,
	char *b,
	size_t n,
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
med3 (char *a,
	char *b,
	char *c,
	cmp_t *cmp,
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

/*
 * Classical function call recursion wastes a lot of stack space. Each
 * recursion level requires a full stack frame comprising all local variables
 * and additional space as dictated by the processor calling convention.
 *
 * This implementation instead stores the variables that are unique for each
 * recursion level in a parameter stack array, and uses iteration to emulate
 * recursion. Function call recursion is not used until the array is full.
 *
 * To ensure the stack consumption isn't worsened by this design, the size of
 * the parameter stack array is chosen to be similar to the stack frame
 * excluding the array. Each function call recursion level can handle this
 * number of iterative recursion levels.
 */
#define PARAMETER_STACK_LEVELS 8u

#if defined(I_AM_QSORT_R)
void
__bsd_qsort_r (void *a,
	size_t n,
	size_t es,
	void *thunk,
	cmp_t *cmp)
#elif defined(I_AM_GNU_QSORT_R)
void
qsort_r (void *a,
	size_t n,
	size_t es,
	cmp_t *cmp,
	void *thunk)
#else
#define thunk NULL
void
qsort (void *a,
	size_t n,
	size_t es,
	cmp_t *cmp)
#endif
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	size_t d, r;
	int cmp_result;
	int swaptype, swap_cnt;
	size_t recursion_level = 0;
	struct { void *a; size_t n; } parameter_stack[PARAMETER_STACK_LEVELS];

	SWAPINIT(a, es);
loop:	swap_cnt = 0;
	if (n < 7) {
		/* Short arrays are insertion sorted. */
		for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
			for (pl = pm; pl > (char *) a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		goto pop;
	}

	/* Select a pivot element, move it to the left. */
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

	/*
	 * Sort the array relative the pivot in four ranges as follows:
	 * { elems == pivot, elems < pivot, elems > pivot, elems == pivot }
	 */
	pa = pb = (char *) a + es;
	pc = pd = (char *) a + (n - 1) * es;
	for (;;) {
		/* Scan left to right stopping at first element > pivot. */
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			/* Move elements == pivot to the left (to pa) */
			if (cmp_result == 0) {
				swap_cnt = 1;
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		/* Scan right to left stopping at first element < pivot. */
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			/* Move elements == pivot to the right (to pd) */
			if (cmp_result == 0) {
				swap_cnt = 1;
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		/* The scan has found two elements to swap with each other. */
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
		goto pop;
	}

	/*
	 * Rearrange the array in three parts sorted like this:
	 * { elements < pivot, elements == pivot, elements > pivot }
	 */
	pn = (char *) a + n * es;
	r = min(pa - (char *)a, pb - pa);
	vecswap(a, pb - r, r);
	r = min((size_t) (pd - pc), (size_t) (pn - pd) - es);
	vecswap(pb, pn - r, r);
	d = pb - pa; /* d = Size of left part. */
	r = pd - pc; /* r = Size of right part. */
	pn -= r;     /* pn = Base of right part. */

	/*
	 * Check which of the left and right parts are larger.
	 * Set (a, n)  to (base, size) of the larger part.
	 * Set (pa, r) to (base, size) of the smaller part.
	 */
	if (r > d) { /* Right part is the larger part */
		pa = a;
		a = pn;
		n = r;
		r = d;
	}
	else { /* Left part is the larger part, or both are equal. */
		pa = pn;
		n = d;
	}

	/*
	 * The left and right parts each need further sorting if they
	 * contain two elements or more. If both need sorting we use
	 * recursion to sort the smaller part and save the larger part
	 * to be sorted by iteration after the recursion.
	 * Using recursion only for the smaller part guarantees a
	 * recursion depth that is bounded to be less than (log2(n)).
	 */
	if (r > es) {  /* Smaller part > 1 element. Both parts need sorting. */
		if (recursion_level < PARAMETER_STACK_LEVELS) {
			/*
			 * The smaller part needs to be recursively sorted
			 * before the larger part is sorted. To avoid function
			 * call recursion the parameters for the larger part
			 * are pushed on the parameter_stack array. The smaller
			 * part is sorted using iteration and the larger part
			 * will be sorted when the parameter_stack is popped
			 * after the smaller part has been sorted.
			 */
			parameter_stack[recursion_level].a = a;
			parameter_stack[recursion_level].n = n / es;
			recursion_level++;
			a = pa;
			n = r / es;
			goto loop;
		}
		else {
			/*
			 * The parameter_stack array is full. The smaller part
			 * is sorted using function call recursion. The larger
			 * part will be sorted after the function call returns.
			 */
#if defined(I_AM_QSORT_R)
			__bsd_qsort_r(pa, r / es, es, thunk, cmp);
#elif defined(I_AM_GNU_QSORT_R)
			qsort_r(pa, r / es, es, cmp, thunk);
#else
			qsort(pa, r / es, es, cmp);
#endif
		}
	}
	if (n > es) {  /* The larger part needs sorting. Iterate to sort.  */
		n = n / es;
		goto loop;
	}
	/* Both left and right parts are one element or less - level done. */
pop:
	if (recursion_level != 0) {
		recursion_level--;
		a = parameter_stack[recursion_level].a;
		n = parameter_stack[recursion_level].n;
		goto loop;
	}
}
