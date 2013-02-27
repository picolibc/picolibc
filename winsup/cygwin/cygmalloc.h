/* cygmalloc.h: cygwin DLL malloc stuff

   Copyright 2002, 2003, 2004, 2005, 2007, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef __reg1
# define __reg1 __stdcall __attribute__ ((regparm (1)))
#endif
#ifndef __reg2
# define __reg2 __stdcall __attribute__ ((regparm (2)))
#endif
#ifndef __reg3
# define __reg3 __stdcall __attribute__ ((regparm (3)))
#endif
void __reg1 ptfree (void *p);
void __reg1 *ptmalloc (size_t size);
void __reg2 *ptrealloc (void *p, size_t size);
void __reg2 *ptcalloc (size_t nmemb, size_t size);
void __reg2 *ptmemalign (size_t alignment, size_t bytes);
void __reg1 *ptvalloc (size_t bytes);
size_t __reg1 ptmalloc_usable_size (void *p);
int __reg1 ptmalloc_trim (size_t);
int __reg2 ptmallopt (int p, int v);
void ptmalloc_stats ();

#ifdef __x86_64__
#define MALLOC_ALIGNMENT ((size_t)16U)
#endif

#ifndef __INSIDE_CYGWIN__
extern "C" void __set_ENOMEM ();
void *mmap64 (void *, size_t, int, int, int, off_t);
#define mmap mmap64
# define MALLOC_FAILURE_ACTION	__set_ENOMEM ()
# define USE_DL_PREFIX 1
# define MSPACES 1
# define ONLY_MSPACES 1
#endif
#ifdef __cplusplus
}
#endif
