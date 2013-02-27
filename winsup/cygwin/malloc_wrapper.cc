/* malloc_wrapper.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2013 Red Hat, Inc.

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
#include "miscfuncs.h"
#include "cygmalloc.h"
#ifndef MALLOC_DEBUG
#include <malloc.h>
#endif
extern "C" struct mallinfo ptmallinfo ();

/* we provide these stubs to call into a user's
   provided malloc if there is one - otherwise
   functions we provide - like strdup will cause
   problems if malloced on our heap and free'd on theirs.
*/

static bool use_internal = true;
static bool internal_malloc_determined;

/* These routines are used by the application if it
   doesn't provide its own malloc. */

extern "C" void
free (void *p)
{
  malloc_printf ("(%p), called by %p", p, __builtin_return_address (0));
  if (!use_internal)
    user_data->free (p);
  else
    ptfree (p);
}

extern "C" void *
malloc (size_t size)
{
  void *res;
  if (!use_internal)
    res = user_data->malloc (size);
  else
    res = ptmalloc (size);
  malloc_printf ("(%d) = %x, called by %p", size, res, __builtin_return_address (0));
  return res;
}

extern "C" void *
realloc (void *p, size_t size)
{
  void *res;
  if (!use_internal)
    res = user_data->realloc (p, size);
  else
    res = ptrealloc (p, size);
  malloc_printf ("(%x, %d) = %x, called by %x", p, size, res, __builtin_return_address (0));
  return res;
}

/* BSD extension:  Same as realloc, just if it fails to allocate new memory,
   it frees the incoming pointer. */
extern "C" void *
reallocf (void *p, size_t size)
{
  void *res = realloc (p, size);
  if (!res && p)
    free (p);
  return res;
}

extern "C" void *
calloc (size_t nmemb, size_t size)
{
  void *res;
  if (!use_internal)
    res = user_data->calloc (nmemb, size);
  else
    res = ptcalloc (nmemb, size);
  malloc_printf ("(%d, %d) = %x, called by %x", nmemb, size, res, __builtin_return_address (0));
  return res;
}

extern "C" int
posix_memalign (void **memptr, size_t alignment, size_t bytes)
{
  save_errno save;

  void *res;
  if (!use_internal)
    return ENOSYS;
  if ((alignment & (alignment - 1)) != 0)
    return EINVAL;
  res = ptmemalign (alignment, bytes);
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
  if (!use_internal)
    {
      set_errno (ENOSYS);
      res = NULL;
    }
  else
    res = ptmemalign (alignment, bytes);

  return res;
}

extern "C" void *
valloc (size_t bytes)
{
  void *res;
  if (!use_internal)
    {
      set_errno (ENOSYS);
      res = NULL;
    }
  else
    res = ptvalloc (bytes);

  return res;
}

extern "C" size_t
malloc_usable_size (void *p)
{
  size_t res;
  if (!use_internal)
    {
      set_errno (ENOSYS);
      res = 0;
    }
  else
    res = ptmalloc_usable_size (p);

  return res;
}

extern "C" int
malloc_trim (size_t pad)
{
  size_t res;
  if (!use_internal)
    {
      set_errno (ENOSYS);
      res = 0;
    }
  else
    res = ptmalloc_trim (pad);

  return res;
}

extern "C" int
mallopt (int p, int v)
{
  int res;
  if (!use_internal)
    {
      set_errno (ENOSYS);
      res = 0;
    }
  else
    res = ptmallopt (p, v);

  return res;
}

extern "C" void
malloc_stats ()
{
  if (!use_internal)
    set_errno (ENOSYS);
  else
    ptmalloc_stats ();
}

extern "C" struct mallinfo
mallinfo ()
{
  struct mallinfo m;
  if (!use_internal)
    set_errno (ENOSYS);
  else
    m = ptmallinfo ();

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

void
malloc_init ()
{
#ifndef MALLOC_DEBUG
  /* Check if malloc is provided by application. If so, redirect all
     calls to malloc/free/realloc to application provided. This may
     happen if some other dll calls cygwin's malloc, but main code provides
     its own malloc */
  if (!internal_malloc_determined)
    {
      extern void *_sigfe_malloc;
      /* Decide if we are using our own version of malloc by testing the import
	 address from user_data.  */
      use_internal = user_data->malloc == malloc
		     || import_address (user_data->malloc) == &_sigfe_malloc;
      malloc_printf ("using %s malloc", use_internal ? "internal" : "external");
      internal_malloc_determined = true;
    }
#endif
}

extern "C" void
__set_ENOMEM ()
{
  set_errno (ENOMEM);
}
