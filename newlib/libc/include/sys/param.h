/* This is a dummy <sys/param.h> file, not customized for any
   particular system.  If there is a param.h in libc/sys/SYSDIR/sys,
   it will override this one.  */

#ifndef _SYS_PARAM_H
# define _SYS_PARAM_H

# define HZ (60)
# define NOFILE	(60)
# define PATHSIZE (1024)

#ifdef __i386__
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#endif
