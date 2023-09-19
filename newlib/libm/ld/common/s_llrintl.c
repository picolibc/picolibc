//__FBSDID("$FreeBSD: src/lib/msun/src/s_llrintl.c,v 1.1 2008/01/14 02:12:06 das Exp $");

#define dtype		long long
#define DTYPE_MIN LLONG_MIN
#define DTYPE_MAX LLONG_MAX
#define fn llrintl

#include "s_lrintl.c"
