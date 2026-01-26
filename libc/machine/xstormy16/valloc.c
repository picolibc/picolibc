/* stdlib/valloc.c defines all these symbols in this file. */
#include <picolibc.h>

#define DEFINE_PVALLOC
#define DEFINE_VALLOC
#include "tiny-malloc.c"
