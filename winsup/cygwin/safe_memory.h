/* safe_memory.h

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __SAFE_MEMORY_H__
#define __SAFE_MEMORY_H__

/*****************************************************************************/

/* Temporary hack to get around the thread-unsafe new/delete in cygwin
 * gcc 2.95.3.  This should all be binned at the first opportunity,
 * e.g. gcc 3.1 or sooner.
 *
 * The trick here is to do contruction via malloc(3) and then the
 * placement new operator, and destruction via an explicit call to the
 * destructor and then free(3).
 */

#include <stdlib.h>

inline void *operator new (size_t, void *__p) throw () { return __p; }

#define safe_new0(T) (new (malloc (sizeof (T))) T)

#ifdef NEW_MACRO_VARARGS

#define safe_new(T, ...)			\
  (new (malloc (sizeof (T))) T (__VA_ARGS__))

#else /* !NEW_MACRO_VARARGS */

#define safe_new(T, args...)			\
  (new (malloc (sizeof (T))) T (## args))

#endif /* !NEW_MACRO_VARARGS */

template <typename T> void
safe_delete (T *const object)
{
  if (object)
    {
      object->~T ();
      free (object);
    }
}

#endif /* __SAFE_MEMORY_H__ */
