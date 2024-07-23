/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
c) UNIX System Laboratories, Inc.
All or some portions of this file are derived from material licensed
to the University of California by American Telephone and Telegraph
Co. or Unix System Laboratories, Inc. and are reproduced herein with
the permission of UNIX System Laboratories, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
/* malloc.h -- header file for memory routines.  */

#ifndef _INCLUDE_MALLOC_H_
#define _INCLUDE_MALLOC_H_

#include <sys/cdefs.h>
#define __need_size_t
#include <stddef.h>

/* include any machine-specific extensions */
#include <machine/malloc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This version of struct mallinfo must match the one in
   libc/stdlib/mallocr.c.  */

struct mallinfo {
  size_t arena;    /* total space allocated from system */
  size_t ordblks;  /* number of non-inuse chunks */
  size_t smblks;   /* unused -- always zero */
  size_t hblks;    /* number of mmapped regions */
  size_t hblkhd;   /* total space in mmapped regions */
  size_t usmblks;  /* unused -- always zero */
  size_t fsmblks;  /* unused -- always zero */
  size_t uordblks; /* total allocated space */
  size_t fordblks; /* total non-inuse space */
  size_t keepcost; /* top-most, releasable (via malloc_trim) space */
};	

/* The routines.  */

void	free (void *) _NOTHROW;
void	*malloc(size_t) __malloc_like __result_use_check __alloc_size(1) _NOTHROW;
void	*calloc(size_t, size_t) __malloc_like __result_use_check
    __alloc_size2(1, 2) _NOTHROW;
void	*realloc(void *, size_t) __result_use_check __alloc_size(2) _NOTHROW;
void    *memalign (size_t __alignment, size_t __size)  __malloc_like
    __result_use_check __alloc_size(2) _NOTHROW;

struct mallinfo mallinfo (void);
void malloc_stats (void);
int mallopt (int, int);
size_t malloc_usable_size (void *);
/* These aren't too useful on an embedded system, but we define them
   anyhow.  */

void *pvalloc (size_t);
int malloc_trim (size_t);
void __malloc_lock(void);
void __malloc_unlock(void);

/* A compatibility routine for an earlier version of the allocator.  */

void mstats (char *);

/* SVID2/XPG mallopt options */

#define M_MXFAST  1    /* UNUSED in this malloc */
#define M_NLBLKS  2    /* UNUSED in this malloc */
#define M_GRAIN   3    /* UNUSED in this malloc */
#define M_KEEP    4    /* UNUSED in this malloc */

/* mallopt options that actually do something */
  
#define M_TRIM_THRESHOLD    -1
#define M_TOP_PAD           -2
#define M_MMAP_THRESHOLD    -3 
#define M_MMAP_MAX          -4

/* Some systems provide this, so do too for compatibility.  */
void cfree (void *);

#ifdef __cplusplus
}
#endif

#endif /* _INCLUDE_MALLOC_H_ */
