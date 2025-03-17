#include <picolibc.h>

#ifdef __x86_64
#include "tls-64.S"
#else
#include "tls-32.S"
#endif
