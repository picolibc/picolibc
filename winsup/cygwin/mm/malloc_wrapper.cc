/* malloc_wrapper.cc

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
#include <malloc.h>
extern "C" struct mallinfo dlmallinfo ();

/* we provide these stubs to call into a user's
   provided malloc if there is one - otherwise
   functions we provide - like strdup will cause
   problems if malloced on our heap and free'd on theirs.
*/

static bool use_internal = true;
static bool internal_malloc_determined;

/* Helper function to generate the correct caller address.  For external
   calls, the return address on the stack is _sigbe.  In that case the
   actual caller return address is on the cygtls stack.  Use this function
   via the macro caller_return_address. */
extern "C" void _sigbe ();

static inline void *
__caller_return_address (void *builtin_ret_addr)
{
  return builtin_ret_addr == &_sigbe
	 ? (void *) _my_tls.retaddr () : builtin_ret_addr;
}

#define caller_return_address() \
		__caller_return_address (__builtin_return_address (0))
void * __caller_return_address (void *);

/* Return an address from the import jmp table of main program.  */
static inline void *
import_address (void *imp)
{
  __try
    {
      if (*((uint16_t *) imp) == 0x25ff)
	{
	  const char *ptr = (const char *) imp;
	  const uintptr_t *jmpto = (uintptr_t *)
				   (ptr + 6 + *(int32_t *)(ptr + 2));
	  return (void *) *jmpto;
	}
    }
  __except (NO_ERROR) {}
  __endtry
  return NULL;
}

/* These routines are used by the application if it
   doesn't provide its own malloc. */

extern "C" void
free (void *p)
{
  malloc_printf ("(%p), called by %p", p, caller_return_address ());
  if (!use_internal)
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
  if (!use_internal)
    res = user_data->malloc (size);
  else
    {
      __malloc_lock ();
      res = dlmalloc (size);
      __malloc_unlock ();
    }
  malloc_printf ("(%ld) = %p, called by %p", size, res,
					     caller_return_address ());
  return res;
}

extern "C" void *
realloc (void *p, size_t size)
{
  void *res;
  if (!use_internal)
    res = user_data->realloc (p, size);
  else
    {
      __malloc_lock ();
      res = dlrealloc (p, size);
      __malloc_unlock ();
    }
  malloc_printf ("(%p, %ld) = %p, called by %p", p, size, res,
						 caller_return_address ());
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
    {
      __malloc_lock ();
      res = dlcalloc (nmemb, size);
      __malloc_unlock ();
    }
  malloc_printf ("(%ld, %ld) = %p, called by %p", nmemb, size, res,
						  caller_return_address ());
  return res;
}

extern "C" int
posix_memalign (void **memptr, size_t alignment, size_t bytes)
{
  save_errno save;

  void *res;
  if (!use_internal)
    return user_data->posix_memalign (memptr, alignment, bytes);
  if ((alignment & (alignment - 1)) != 0)
    return EINVAL;
  __malloc_lock ();
  res = dlmemalign (alignment, bytes);
  __malloc_unlock ();
  if (!res)
    return ENOMEM;
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
  if (!use_internal)
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
  if (!use_internal)
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
  if (!use_internal)
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
  if (!use_internal)
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
  if (!use_internal)
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
  if (!use_internal)
    {
      memset (&m, 0, sizeof m);
      set_errno (ENOSYS);
    }
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

/* We use a SRW lock to lock access to the malloc data structures.  This
   permits malloc to be called from different threads.  Note that it does
   not make malloc reentrant, and it does not permit a signal handler to
   call malloc.  The malloc code in newlib will call __malloc_lock and
   __malloc_unlock at appropriate times.  */

SRWLOCK NO_COPY mallock = SRWLOCK_INIT;

void
malloc_init ()
{
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
		     || import_address ((void *) user_data->malloc)
			== &_sigfe_malloc;
      malloc_printf ("using %s malloc", use_internal ? "internal" : "external");
      internal_malloc_determined = true;
    }
}

extern "C" void
__set_ENOMEM ()
{
  set_errno (ENOMEM);
}
