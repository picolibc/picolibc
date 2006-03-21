/* This is a dummy <sys/param.h> file, not customized for any
   particular system.  If there is a param.h in libc/sys/SYSDIR/sys,
   it will override this one.  */

#ifndef _SYS_PARAM_H
# define _SYS_PARAM_H

#include <sys/config.h>

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

# define HZ (60)
# define NOFILE	(60)
# define PATHSIZE (1024)

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#ifndef BYTE_ORDER
#ifdef __IEEE_LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#endif

#endif
