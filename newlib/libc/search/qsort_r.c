/*
FUNCTION
<<qsort_r>>---sort an array

INDEX
	qsort_r

ANSI_SYNOPSIS
	#define _BSD_SOURCE
	#include <stdlib.h>
	void qsort_r(void *<[base]>, size_t <[nmemb]>, size_t <[size]>,
		     void *<[thunk]>,
		     int (*<[compar]>)(void*, const void *, const void *));

	#define _GNU_SOURCE
	#include <stdlib.h>
	void qsort_r(void *<[base]>, size_t <[nmemb]>, size_t <[size]>,
		     int (*<[compar]>)(const void *, const void *, void *),
		     void *<[thunk]>);

TRAD_SYNOPSIS
	#include <stdlib.h>
	qsort_r(<[base]>, <[nmemb]>, <[size]>, <[compar]>, <[thumb]>)
	char *<[base]>;
	size_t <[nmemb]>;
	size_t <[size]>;
	int (*<[compar]>)();
	char *<[thumb]>;

DESCRIPTION
<<qsort_r>> sorts an array (beginning at <[base]>) of <[nmemb]> objects.
<[size]> describes the size of each element of the array.

You must supply a pointer to a comparison function, using the argument
shown as <[compar]>.  (This permits sorting objects of unknown
properties.)  There are two forms of this function, in each the
comparison function is defined to accept three arguments, but in a
different order.  Two are pointers to an element of the array starting at
<[base]>, and another being an arbitrary pointer <[thunk]>.  The
result of <<(*<[compar]>)>> must be negative if the first argument is
less than the second, zero if the two arguments match, and positive if
the first argument is greater than the second (where ``less than'' and
``greater than'' refer to whatever arbitrary ordering is appropriate).

The array is sorted in place; that is, when <<qsort_r>> returns, the
array elements beginning at <[base]> have been reordered.

RETURNS
<<qsort_r>> does not return a result.

PORTABILITY
<<qsort_r>>, in various forms, appears in both BSD and glibc.
*/

#define _GNU_SOURCE
#define I_AM_GNU_QSORT_R
#include "qsort.c"
