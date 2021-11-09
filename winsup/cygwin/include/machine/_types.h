/* Cygwin's machine/_types.h */

#ifndef _MACHINE__TYPES_H
#define _MACHINE__TYPES_H

#include <machine/_default_types.h>

#if defined (__INSIDE_CYGWIN__) || defined (_LIBC)
typedef __int32_t __blkcnt32_t;
typedef __int16_t  __dev16_t;
typedef __uint16_t __uid16_t;
typedef __uint16_t __gid16_t;
typedef __uint32_t __ino32_t;
#endif

#define __machine_blkcnt_t_defined
typedef __int64_t __blkcnt_t;

#define __machine_blksize_t_defined
typedef __int32_t __blksize_t;

#define __machine_dev_t_defined
typedef __uint32_t __dev_t;

#define __machine_fsblkcnt_t_defined
/* Keep as is.  32 bit on i386, 64 bit on x86_64. */
typedef unsigned long __fsblkcnt_t;

#define __machine_fsfilcnt_t_defined
/* Keep as is.  32 bit on i386, 64 bit on x86_64. */
typedef unsigned long __fsfilcnt_t;

#define __machine_uid_t_defined
typedef __uint32_t __uid_t;

#define __machine_gid_t_defined
typedef __uint32_t __gid_t;

#define __machine_ino_t_defined
typedef __uint64_t __ino_t;

#define __machine_key_t_defined
typedef long long __key_t;

#define __machine_sa_family_t_defined
typedef __uint16_t __sa_family_t;

/* Not unsigned for backward compatibility.  */
#define __machine_socklen_t_defined
typedef int __socklen_t;

#endif /* _MACHINE__TYPES_H */
