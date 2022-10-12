#ifdef __x86_64
#include "memset-64.S"
#else
#include "memset-32.S"
#endif
