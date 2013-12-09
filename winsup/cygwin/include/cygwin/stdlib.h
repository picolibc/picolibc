/* stdlib.h

   Copyright 2005, 2006, 2007, 2008, 2009, 2011, 2013 Red Hat Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_STDLIB_H
#define _CYGWIN_STDLIB_H

#include <cygwin/wait.h>

#ifdef __cplusplus
extern "C"
{
#endif

__uint32_t arc4random(void);
void arc4random_addrandom(unsigned char *, int);
void arc4random_buf(void *, size_t);
void arc4random_stir(void);
__uint32_t arc4random_uniform(__uint32_t);

const char *getprogname (void);
void	setprogname (const char *);

#ifndef __STRICT_ANSI__
char *canonicalize_file_name (const char *);
int unsetenv (const char *);
#endif /*__STRICT_ANSI__*/
#if !defined(__STRICT_ANSI__) \
    || (defined(_XOPEN_SOURCE) \
	&& ((_XOPEN_SOURCE - 0 >= 500) || defined(_XOPEN_SOURCE_EXTENDED)))
char *initstate (unsigned seed, char *state, size_t size);
long random (void);
char *setstate (const char *state);
void srandom (unsigned);
#endif
#ifndef __STRICT_ANSI__
char *ptsname (int);
int ptsname_r(int, char *, size_t);
int getpt (void);
int grantpt (int);
int unlockpt (int);
#endif /*__STRICT_ANSI__*/

int posix_openpt (int);
int posix_memalign (void **, size_t, size_t);

#ifdef _COMPILING_NEWLIB
#define unsetenv UNUSED_unsetenv
#define _unsetenv_r UNUSED__unsetenv_r
#endif

extern _PTR memalign _PARAMS ((size_t, size_t));
extern _PTR valloc _PARAMS ((size_t));

#undef _malloc_r
#define _malloc_r(r, s) malloc (s)
#undef _free_r
#define _free_r(r, p) free (p)
#undef _realloc_r
#define _realloc_r(r, p, s) realloc (p, s)
#undef _calloc_r
#define _calloc_r(r, s1, s2) calloc (s1, s2);
#undef _memalign_r
#define _memalign_r(r, s1, s2) memalign (s1, s2);
#undef _mallinfo_r
#define _mallinfo_r(r) mallinfo ()
#undef _malloc_stats_r
#define _malloc_stats_r(r) malloc_stats ()
#undef _mallopt_r
#define _mallopt_r(i1, i2) mallopt (i1, i2)
#undef _malloc_usable_size_r
#define _malloc_usable_size_r(r, p) malloc_usable_size (p)
#undef _valloc_r
#define _valloc_r(r, s) valloc (s)
#undef _pvalloc_r
#define _pvalloc_r(r, s) pvalloc (s)
#undef _malloc_trim_r
#define _malloc_trim_r(r, s) malloc_trim (s)
#undef _mstats_r
#define _mstats_r(r, p) mstats (p)

#ifdef __cplusplus
}
#endif
#endif /*_CYGWIN_STDLIB_H*/
