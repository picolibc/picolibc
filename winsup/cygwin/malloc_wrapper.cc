/* malloc_wrapper.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

   Originally written by Steve Chamberlain of Cygnus Support
   sac@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <assert.h>
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include <errno.h>
#include "cygerrno.h"
#include "cygheap.h"
#include "heap.h"
#include "sync.h"
#include "perprocess.h"
#include "cygmalloc.h"

/* we provide these stubs to call into a user's
   provided malloc if there is one - otherwise
   functions we provide - like strdup will cause
   problems if malloced on our heap and free'd on theirs.
*/

static int export_malloc_called;
static int use_internal_malloc = 1;

#undef in
#undef out
#define in(x)
#define out(x)

#ifdef MALLOC_DEBUG
extern "C" void * _sbrk (size_t incr_arg);

#if 0
extern "C" void *
_sbrk_r (struct _reent *, size_t incr_arg)
{
  return _sbrk (incr_arg);
}
#endif

extern "C" void *
_malloc_r (struct _reent *, size_t size)
{
  export_malloc_called = 1;
  return malloc (size);
}
#undef malloc

extern "C" void *
_calloc_r (struct _reent *, size_t nmemb, size_t size)
{
  export_malloc_called = 1;
  return calloc (nmemb, size);
}
#undef calloc

extern "C" void
_free_r (struct _reent *, void *p)
{
  export_malloc_called = 1;
  assert (!incygheap (p));
  assert (inheap (p));
  free (p);
}
#undef free

extern "C" void *
_realloc_r (struct _reent *, void *p, size_t size)
{
  export_malloc_called = 1;
  assert (!incygheap (p));
  assert (inheap (p));
  return realloc (p, size);
}
#undef realloc

extern "C" char *
strdup_dbg (const char *s, const char *file, int line)
{
  char *p;
  export_malloc_called = 1;
  if ((p = (char *) malloc_dbg (strlen (s) + 1, file, line)) != NULL)
      strcpy (p, s);
  return p;
}

#undef strdup
extern "C" char *
strdup (const char *s)
{
  return strdup_dbg (s, __FILE__, __LINE__);
}
#else
#endif
/* These routines are used by the application if it
   doesn't provide its own malloc. */

extern "C" void
free (void *p)
{
  malloc_printf ("(%p), called by %x", p, ((int *)&p)[-1]);
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
  malloc_printf ("(%d) = %x, called by %x", size, res, ((int *)&size)[-1]);
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
  malloc_printf ("(%x, %d) = %x, called by %x", p, size, res, ((int *)&p)[-1]);
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
  malloc_printf ("(%d, %d) = %x, called by %x", nmemb, size, res, ((int *)&nmemb)[-1]);
  return res;
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

  return;
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

NO_COPY muto *mallock = NULL;

void
malloc_init ()
{
  new_muto (mallock);
  /* Check if mallock is provided by application. If so, redirect all
     calls to malloc/free/realloc to application provided. This may
     happen if some other dll calls cygwin's malloc, but main code provides
     its own malloc */
  if (!user_data->forkee)
    {
#ifdef MALLOC_DEBUG
      _free_r (NULL, _malloc_r (NULL, 16));
#else
      free (malloc (16));
#endif
      if (!export_malloc_called)
	use_internal_malloc = 0;
    }
}
