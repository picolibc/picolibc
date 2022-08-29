/* cygmalloc.h: cygwin DLL malloc stuff

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __cplusplus
extern "C" {
#endif

void dlfree (void *p);
void *dlmalloc (size_t size);
void *dlrealloc (void *p, size_t size);
void *dlcalloc (size_t nmemb, size_t size);
void *dlmemalign (size_t alignment, size_t bytes);
void *dlvalloc (size_t bytes);
size_t dlmalloc_usable_size (void *p);
int dlmalloc_trim (size_t);
int dlmallopt (int p, int v);
void dlmalloc_stats ();

#define MALLOC_ALIGNMENT ((size_t)16U)

#if defined (DLMALLOC_VERSION)	/* Building malloc.cc */

extern "C" void __set_ENOMEM ();
# define MALLOC_FAILURE_ACTION	__set_ENOMEM ()
# define USE_DL_PREFIX 1

#elif defined (__INSIDE_CYGWIN__)

# define __malloc_lock() AcquireSRWLockExclusive (&mallock)
# define __malloc_unlock() ReleaseSRWLockExclusive (&mallock)
extern SRWLOCK NO_COPY mallock;
void malloc_init ();

#endif

#ifdef __cplusplus
}
#endif
