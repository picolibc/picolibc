/* libc/sys/linux/include/stdint.h - Standard integer types */

/* Written 2000 by Werner Almesberger */


#ifndef _NEWLIB_STDINT_H
#define _NEWLIB_STDINT_H

/*
 * FIXME: linux/types.h defines various types that rightfully belong into
 * stdint.h. So we have no choice but to include linux/types.h directly, even
 * if this causes name space pollution. Note: we have to go via sys/types.h
 * in order to resolve some other compatibility issues.
 */

#include <sys/types.h>

#endif
