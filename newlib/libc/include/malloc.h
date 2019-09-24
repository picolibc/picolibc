/* malloc.h -- header file for memory routines.  */

#ifndef _INCLUDE_MALLOC_H_
#define _INCLUDE_MALLOC_H_

#include <_ansi.h>

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

extern void *malloc (size_t);
extern void free (void *);
extern void *realloc (void *, size_t);
extern void *calloc (size_t, size_t);
extern void *memalign (size_t, size_t);
extern struct mallinfo mallinfo (void);
extern void malloc_stats (void);
extern int mallopt (int, int);
extern size_t malloc_usable_size (void *);
/* These aren't too useful on an embedded system, but we define them
   anyhow.  */

extern void *valloc (size_t);
extern void *pvalloc (size_t);
extern int malloc_trim (size_t);
extern void __malloc_lock(void);
extern void __malloc_unlock(void);

/* A compatibility routine for an earlier version of the allocator.  */

extern void mstats (char *);

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

#ifndef __CYGWIN__
/* Some systems provide this, so do too for compatibility.  */
extern void cfree (void *);
#endif /* __CYGWIN__ */

#ifdef __cplusplus
}
#endif

#endif /* _INCLUDE_MALLOC_H_ */
