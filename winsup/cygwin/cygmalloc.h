/* cygmalloc.h: cygwin DLL malloc stuff

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

extern "C" void dlfree (void *p) __attribute__ ((regparm (1)));
extern "C" void *dlmalloc (unsigned size) __attribute__ ((regparm (1)));
extern "C" void *dlrealloc (void *p, unsigned size) __attribute__ ((regparm (2)));
extern "C" void *dlcalloc (size_t nmemb, size_t size) __attribute__ ((regparm (2)));
extern "C" void *dlmemalign (size_t alignment, size_t bytes) __attribute__ ((regparm (2)));
extern "C" void *dlvalloc (size_t bytes) __attribute__ ((regparm (1)));
extern "C" size_t dlmalloc_usable_size (void *p) __attribute__ ((regparm (1)));
extern "C" int dlmalloc_trim (size_t) __attribute__ ((regparm (1)));
extern "C" int dlmallopt (int p, int v) __attribute__ ((regparm (2)));
extern "C" void dlmalloc_stats ();

#ifndef __INSIDE_CYGWIN__
# define USE_DL_PREFIX 1
# define MORECORE_CANNOT_TRIM 1
#else
# define __malloc_lock() mallock->acquire ()
# define __malloc_unlock() mallock->release ()
extern muto *mallock;
#endif
