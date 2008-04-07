/* malloc_wrapper.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007 Red Hat, Inc.

   Originally written by Steve Chamberlain of Cygnus Support
   sac@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "perprocess.h"
#include "cygmalloc.h"
#ifndef MALLOC_DEBUG
#include <malloc.h>
#endif
extern "C" struct mallinfo dlmallinfo ();

/* we provide these stubs to call into a user's
   provided malloc if there is one - otherwise
   functions we provide - like strdup will cause
   problems if malloced on our heap and free'd on theirs.
*/

static int export_malloc_called;
static int use_internal_malloc = 1;

/* These routines are used by the application if it
   doesn't provide its own malloc. */

extern "C" void
free (void *p)
{
  malloc_printf ("(%p), called by %p", p, __builtin_return_address (0));
  if (!use_internal_malloc)
    user_data->free (p);
  else
    {
      __malloc_lock ();
      dlfree (p);
      __malloc_unlock ();
    }
}

extern "C" void *
malloc (size_t size)
{
  void *res;
  export_malloc_called = 1;
  if (!use_internal_malloc)
    res = user_data->malloc (size);
  else
    {
      __malloc_lock ();
      res = dlmalloc (size);
      __malloc_unlock ();
    }
  malloc_printf ("(%d) = %x, called by %p", size, res, __builtin_return_address (0));
  return res;
}

extern "C" void *
realloc (void *p, size_t size)
{
  void *res;
  if (!use_internal_malloc)
    res = user_data->realloc (p, size);
  else
    {
      __malloc_lock ();
      res = dlrealloc (p, size);
      __malloc_unlock ();
    }
  malloc_printf ("(%x, %d) = %x, called by %x", p, size, res, __builtin_return_address (0));
  return res;
}

extern "C" void *
calloc (size_t nmemb, size_t size)
{
  void *res;
  if (!use_internal_malloc)
    res = user_data->calloc (nmemb, size);
  else
    {
      __malloc_lock ();
      res = dlcalloc (nmemb, size);
      __malloc_unlock ();
    }
  malloc_printf ("(%d, %d) = %x, called by %x", nmemb, size, res, __builtin_return_address (0));
  return res;
}

extern "C" int
posix_memalign (void **memptr, size_t alignment, size_t bytes)
{
  save_errno save;

  void *res;
  if (!use_internal_malloc)
    return ENOSYS;
  if ((alignment & (alignment - 1)) != 0)
    return EINVAL;
  __malloc_lock ();
  res = dlmemalign (alignment, bytes);
  __malloc_unlock ();
  if (!res)
    return ENOMEM;
  if (memptr)
    *memptr = res;
  return 0;
}

extern "C" void *
memalign (size_t alignment, size_t bytes)
{
  void *res;
  if (!use_internal_malloc)
    {
      set_errno (ENOSYS);
      res = NULL;
    }
  else
    {
      __malloc_lock ();
      res = dlmemalign (alignment, bytes);
      __malloc_unlock ();
    }

  return res;
}

extern "C" void *
valloc (size_t bytes)
{
  void *res;
  if (!use_internal_malloc)
    {
      set_errno (ENOSYS);
      res = NULL;
    }
  else
    {
      __malloc_lock ();
      res = dlvalloc (bytes);
      __malloc_unlock ();
    }

  return res;
}

extern "C" size_t
malloc_usable_size (void *p)
{
  size_t res;
  if (!use_internal_malloc)
    {
      set_errno (ENOSYS);
      res = 0;
    }
  else
    {
      __malloc_lock ();
      res = dlmalloc_usable_size (p);
      __malloc_unlock ();
    }

  return res;
}

extern "C" int
malloc_trim (size_t pad)
{
  size_t res;
  if (!use_internal_malloc)
    {
      set_errno (ENOSYS);
      res = 0;
    }
  else
    {
      __malloc_lock ();
      res = dlmalloc_trim (pad);
      __malloc_unlock ();
    }

  return res;
}

extern "C" int
mallopt (int p, int v)
{
  int res;
  if (!use_internal_malloc)
    {
      set_errno (ENOSYS);
      res = 0;
    }
  else
    {
      __malloc_lock ();
      res = dlmallopt (p, v);
      __malloc_unlock ();
    }

  return res;
}

extern "C" void
malloc_stats ()
{
  if (!use_internal_malloc)
    set_errno (ENOSYS);
  else
    {
      __malloc_lock ();
      dlmalloc_stats ();
      __malloc_unlock ();
    }
}

extern "C" struct mallinfo
mallinfo ()
{
  struct mallinfo m;
  if (!use_internal_malloc)
    set_errno (ENOSYS);
  else
    {
      __malloc_lock ();
      m = dlmallinfo ();
      __malloc_unlock ();
    }

  return m;
}

extern "C" char *
strdup (const char *s)
{
  char *p;
  size_t len = strlen (s) + 1;
  if ((p = (char *) malloc (len)) != NULL)
    memcpy (p, s, len);
  return p;
}

/* We use a critical section to lock access to the malloc data
   structures.  This permits malloc to be called from different
   threads.  Note that it does not make malloc reentrant, and it does
   not permit a signal handler to call malloc.  The malloc code in
   newlib will call __malloc_lock and __malloc_unlock at appropriate
   times.  */

muto NO_COPY mallock;

void
malloc_init ()
{
  mallock.init ("mallock");

#ifndef MALLOC_DEBUG
  /* Check if malloc is provided by application. If so, redirect all
     calls to malloc/free/realloc to application provided. This may
     happen if some other dll calls cygwin's malloc, but main code provides
     its own malloc */
  if (!in_forkee)
    {
      user_data->free (user_data->malloc (16));
      if (export_malloc_called)
	malloc_printf ("using internal malloc");
      else
	{
	  use_internal_malloc = 0;
	  malloc_printf ("using external malloc");
	}
    }
#endif
}

extern "C" void
__set_ENOMEM ()
{
  set_errno (ENOMEM);
}
