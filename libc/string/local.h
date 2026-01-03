/* Copyright (c) 2009 Corinna Vinschen <corinna@vinschen.de> */
#include "../ctype/local.h"

/* internal function to compute width of wide char. */
int   __wcwidth(wint_t);

/* Reentrant version of strerror.  */
char *_strerror_r(int, int, int *);

/* Nonzero if X is not aligned on a "long" boundary.
 * This macro is used to skip a few bytes to find an aligned pointer.
 * It's better to keep it as is even if _HAVE_HW_MISALIGNED_ACCESS is enabled,
 * to avoid small performance penalties (if they are not zero).  */
#define UNALIGNED_X(X) ((uintptr_t)(X) & (sizeof(unsigned long) - 1))

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED_X_Y(X, Y)                            \
    (((uintptr_t)(X) & (sizeof(unsigned long) - 1))    \
     | ((uintptr_t)(Y) & (sizeof(unsigned long) - 1)))

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLE_BLOCK_SIZE (sizeof(unsigned long))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIG_BLOCK_SIZE (sizeof(unsigned long) << 2)

/* Threshhold for punting to the little block byte copier.  */
#define TOO_SMALL_LITTLE_BLOCK(LEN) ((LEN) < LITTLE_BLOCK_SIZE)

/* Threshhold for punting to the big block byte copier.  */
#define TOO_SMALL_BIG_BLOCK(LEN) ((LEN) < BIG_BLOCK_SIZE)

/* Macros for detecting endchar.  */
#if ULONG_MAX == 4294967295UL
#define DETECT_NULL(X) (((X) - 0x01010101UL) & ~(X) & 0x80808080UL)
#elif ULONG_MAX == 18446744073709551615UL
/* Nonzero if X (a long int) contains a NULL byte.  */
#define DETECT_NULL(X) (((X) - 0x0101010101010101UL) & ~(X) & 0x8080808080808080UL)
#else
#error long int is not a 32bit or 64bit type.
#endif

/* Returns nonzero if (unsigned long)X contains the byte used to fill (unsigned long)MASK.  */
#define DETECT_CHAR(X, MASK) (DETECT_NULL(X ^ MASK))
