#include <stdlib.h>

int
_DEFUN (rand_r, (seed), unsigned int *seed)
{
        return (((*seed) = (*seed) * 1103515245 + 12345) & RAND_MAX);
}
