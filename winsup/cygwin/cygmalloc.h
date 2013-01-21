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
void __reg1 dlfree (void *p);
void __reg1 *dlmalloc (unsigned size);
void __reg2 *dlrealloc (void *p, unsigned size);
void __reg2 *dlcalloc (size_t nmemb, size_t size);
void __reg2 *dlmemalign (size_t alignment, size_t bytes);
void __reg1 *dlvalloc (size_t bytes);
size_t __reg1 dlmalloc_usable_size (void *p);
int __reg1 dlmalloc_trim (size_t);
int __reg2 dlmallopt (int p, int v);
void dlmalloc_stats ();

#ifndef __INSIDE_CYGWIN__
extern "C" void __set_ENOMEM ();
# define MALLOC_FAILURE_ACTION	__set_ENOMEM ()
# define USE_DL_PREFIX 1
#else
# define __malloc_lock() mallock.acquire ()
# define __malloc_unlock() mallock.release ()
extern muto mallock;
#endif
#ifdef __cplusplus
}
#endif
