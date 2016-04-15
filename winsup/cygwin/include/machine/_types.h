/* Cygwin's machine/_types.h */

#ifndef _MACHINE__TYPES_H
#define _MACHINE__TYPES_H

#include <machine/_default_types.h>

#if defined (__INSIDE_CYGWIN__) || defined (_COMPILING_NEWLIB)
typedef __int16_t  __dev16_t;
typedef __uint16_t __uid16_t;
typedef __uint16_t __gid16_t;
#endif

#define __machine_dev_t_defined
typedef __uint32_t __dev_t;

#define __machine_uid_t_defined
typedef __uint32_t __uid_t;

#define __machine_gid_t_defined
typedef __uint32_t __gid_t;

#define __machine_key_t_defined
typedef long long __key_t;

#endif /* _MACHINE__TYPES_H */
