#include <picolibc.h>

#ifdef __x86_64
#include "tcb-64.S"
#else
#include "tcb-32.S"
#endif
