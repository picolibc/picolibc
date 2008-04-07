/* Initial implementation:
   Copyright (c) 2002 Robert Drehmel
   All rights reserved.

   As long as the above copyright statement and this notice remain
   unchanged, you can do what ever you want with this file.  */

#include <stdint.h>	/* for uint8_t */
#include <string.h>	/* for memcpy () prototype */

static void *lwork (const void *, const void *, size_t *, size_t,
		    int (*) (const void *, const void *), int);

extern "C" void *
lsearch (const void *key, void *base, size_t *nelp, size_t width,
	       int (*compar) (const void *, const void *))
{
  return lwork (key, base, nelp, width, compar, 1);
}

extern "C" void *
lfind (const void *key, const void *base, size_t *nelp, size_t width,
	     int (*compar) (const void *, const void *))
{
  return lwork (key, base, nelp, width, compar, 0);
}

static void *
lwork (const void *key, const void *base, size_t *nelp, size_t width,
       int (*compar) (const void *, const void *), int addelem)
{
  uint8_t *ep, *endp;

  /*  Cast to an integer value first to avoid the warning for removing
     'const' via a cast.  */
  ep = (uint8_t *) (uintptr_t)base;
  for (endp = (uint8_t *) (ep + width * *nelp); ep < endp; ep += width)
    if (compar (key, ep) == 0)
      return ep;

  /* lfind () shall return when the key was not found. */
  if (!addelem)
    return NULL;

  /* lsearch () adds the key to the end of the table and increments
     the number of elements.  */
  memcpy (endp, key, width);
  ++*nelp;

  return endp;
}
