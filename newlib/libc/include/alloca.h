/* libc/include/alloca.h - Allocate memory on stack */

/* Copyright (c) 2000 Werner Almesberger */
/* Rearranged for general inclusion by stdlib.h.
   2001, Corinna Vinschen <vinschen@redhat.com> */

#ifndef _NEWLIB_ALLOCA_H
#define _NEWLIB_ALLOCA_H

#include "_ansi.h"

#undef alloca

#ifdef _HAVE_BUILTIN_ALLOCA
#define alloca(size) __builtin_alloca(size)
#else
void * alloca (size_t);
#endif

#endif
