/* This is a dummy <sys/param.h> file, not customized for any
   particular system.  If there is a param.h in libc/sys/SYSDIR/sys,
   it will override this one.  */

#ifndef _SYS_PARAM_H
# define _SYS_PARAM_H

#include <sys/config.h>

# define HZ (60)
# define NOFILE	(60)
# define PATHSIZE (1024)

#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234

#ifdef __IEEE_LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#ifdef __IEEE_BIG_ENDIAN
#define BYTE_ORDER BIG_ENDIAN
#endif

#ifdef __i386__
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif

#endif
