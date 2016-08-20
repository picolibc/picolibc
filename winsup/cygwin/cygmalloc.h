/* cygmalloc.h: cygwin DLL malloc stuff

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __cplusplus
extern "C" {
#endif
#include "regparm.h"

void __reg1 dlfree (void *p);
void __reg1 *dlmalloc (size_t size);
void __reg2 *dlrealloc (void *p, size_t size);
void __reg2 *dlcalloc (size_t nmemb, size_t size);
void __reg2 *dlmemalign (size_t alignment, size_t bytes);
void __reg1 *dlvalloc (size_t bytes);
size_t __reg1 dlmalloc_usable_size (void *p);
int __reg1 dlmalloc_trim (size_t);
int __reg2 dlmallopt (int p, int v);
void dlmalloc_stats ();

#ifdef __x86_64__
#define MALLOC_ALIGNMENT ((size_t)16U)
#endif

#if defined (DLMALLOC_VERSION)	/* Building malloc.cc */

extern "C" void __set_ENOMEM ();
void *mmap64 (void *, size_t, int, int, int, off_t);
# define mmap mmap64
# define MALLOC_FAILURE_ACTION	__set_ENOMEM ()
# define USE_DL_PREFIX 1

#elif defined (__INSIDE_CYGWIN__)

# define __malloc_lock() mallock.acquire ()
# define __malloc_unlock() mallock.release ()
extern muto mallock;

#endif

#ifdef __cplusplus
}
#endif
