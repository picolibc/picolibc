#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>

#ifdef HAVE_STDINT_H
#  include <stdint.h>		/* uintptr_t */
#else
#  define uintptr_t size_t
#endif

#define NELEM(a) (sizeof(a) / sizeof(a[0]))

static int
is_aligned (void *p, size_t alignment, size_t offset)
{
  return !((((uintptr_t) p) + offset) & (alignment - 1));
}

#define MAX_SIZE (1 << sizeof(unsigned char))
#define NP 1000
#define NTST 100000U

#define ERRMSG fprintf(stderr, "Iteration %u, align = %u, offset = %u, size = %u (oldsize = %u), errno = %d (%s)\n", i, align[j % NELEM(align)], offset[j % NELEM(offset)], size[j], oldsize, errno, strerror(errno))

int
main (void)
{
  unsigned char *p[NP];
  size_t size[NP];
  size_t align[] = { 2, 4, 8, 16, 32, 64 };
  size_t offset[20];
  unsigned i;

  srand (time (NULL));

  for (i = 0; i < NELEM (p); ++i)
    {
      p[i] = 0;
      size[i] = 0;
    }

  for (i = 0; i < NELEM (offset); ++i)
    offset[i] = rand () % 512;

  for (i = 0; i < NTST; ++i)
    {
      size_t oldsize;
      unsigned j, k;
      j = rand () % NELEM (p);
      oldsize = size[j];
      p[j] = __mingw_aligned_offset_realloc (p[j],
					     size[j] = rand () % MAX_SIZE,
					     align[j % NELEM (align)],
					     offset[j % NELEM (offset)]);

      if (size[j] && !p[j])
	{
	  fprintf (stderr, "Returned NULL!\n");
	  ERRMSG;
	  return EXIT_FAILURE;
	}
      if (size[j] && !is_aligned (p[j],
				  align[j % NELEM (align)],
				  offset[j % NELEM (offset)]))
	{
	  fprintf (stderr, "Misaligned block!\n");
	  ERRMSG;
	  return EXIT_FAILURE;
	}
      if (oldsize > size[j])
	oldsize = size[j];
      for (k = 0; k < oldsize; ++k)
	if (p[j][k] != k)
	  {
	    fprintf (stderr, "Miscopied block!\n");
	    ERRMSG;
	    return EXIT_FAILURE;
	  }
      for (k = 0; k < size[j]; ++k)
	p[j][k] = k;
    }

  for (i = 0; i < NELEM (p); ++i)
    __mingw_aligned_free (p[i]);

  return EXIT_SUCCESS;
}
