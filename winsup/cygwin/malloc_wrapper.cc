/* malloc.cc for WIN32.

   Copyright 1996, 1997, 1998 Cygnus Solutions.

   Written by Steve Chamberlain of Cygnus Support
   sac@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <assert.h>
#include "cygheap.h"
#include "heap.h"
#include "sync.h"
#include "perprocess.h"

/* we provide these stubs to call into a user's
   provided malloc if there is one - otherwise
   functions we provide - like strdup will cause
   problems if malloced on our heap and free'd on theirs.
*/

static int export_malloc_called = 0;
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
/* Call though the application pointer,
   which either points to export_malloc, or the application's
   own version. */

void *
malloc (size_t size)
{
  void *res;
  res = user_data->malloc (size);
  return res;
}

void
free (void *p)
{
  user_data->free (p);
}

void *
realloc (void *p, size_t size)
{
  void *res;
  res = user_data->realloc (p, size);
  return res;
}

void *
calloc (size_t nmemb, size_t size)
{
  void *res;
  res = user_data->calloc (nmemb, size);
  return res;
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

extern "C" char *
_strdup_r (struct _reent *, const char *s)
{
  return strdup (s);
}
#endif

/* These routines are used by the application if it
   doesn't provide its own malloc. */

extern "C"
void
export_free (void *p)
{
  malloc_printf ("(%p), called by %x", p, ((int *)&p)[-1]);
  if (use_internal_malloc)
    _free_r (_impure_ptr, p);
  else
    user_data->free (p);
}

extern "C"
void *
export_malloc (int size)
{
  void *res;
  export_malloc_called = 1;
  if (use_internal_malloc)
    res = _malloc_r (_impure_ptr, size);
  else
    res = user_data->malloc (size);
  malloc_printf ("(%d) = %x, called by %x", size, res, ((int *)&size)[-1]);
  return res;
}

extern "C"
void *
export_realloc (void *p, int size)
{
  void *res;
  if (use_internal_malloc)
    res = _realloc_r (_impure_ptr, p, size);
  else
    res = user_data->realloc (p, size);
  malloc_printf ("(%x, %d) = %x, called by %x", p, size, res, ((int *)&p)[-1]);
  return res;
}

extern "C"
void *
export_calloc (size_t nmemb, size_t size)
{
  void *res;
  if (use_internal_malloc)
    res = _calloc_r (_impure_ptr, nmemb, size);
  else
    res = user_data->calloc (nmemb, size);
  malloc_printf ("(%d, %d) = %x, called by %x", nmemb, size, res, ((int *)&nmemb)[-1]);
  return res;
}

/* We use a critical section to lock access to the malloc data
   structures.  This permits malloc to be called from different
   threads.  Note that it does not make malloc reentrant, and it does
   not permit a signal handler to call malloc.  The malloc code in
   newlib will call __malloc_lock and __malloc_unlock at appropriate
   times.  */

static NO_COPY muto *mprotect = NULL;

void
malloc_init ()
{
  mprotect = new_muto (FALSE, "mprotect");
  /* Check if mallock is provided by application. If so, redirect all
     calls to export_malloc/free/realloc to application provided. This may
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

extern "C"
void
__malloc_lock (struct _reent *)
{
  mprotect->acquire ();
}

extern "C"
void
__malloc_unlock (struct _reent *)
{
  mprotect->release ();
}
