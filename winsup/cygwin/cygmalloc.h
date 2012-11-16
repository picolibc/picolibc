/* cygmalloc.h: cygwin DLL malloc stuff

   Copyright 2002, 2003, 2004, 2005, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __cplusplus
extern "C" {
#endif
void dlfree (void *p) __attribute__ ((regparm (1)));
void *dlmalloc (size_t size) __attribute__ ((regparm (1)));
void *dlrealloc (void *p, size_t size) __attribute__ ((regparm (2)));
void *dlcalloc (size_t nmemb, size_t size) __attribute__ ((regparm (2)));
void *dlmemalign (size_t alignment, size_t bytes) __attribute__ ((regparm (2)));
void *dlvalloc (size_t bytes) __attribute__ ((regparm (1)));
size_t dlmalloc_usable_size (void *p) __attribute__ ((regparm (1)));
int dlmalloc_trim (size_t) __attribute__ ((regparm (1)));
int dlmallopt (int p, int v) __attribute__ ((regparm (2)));
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
