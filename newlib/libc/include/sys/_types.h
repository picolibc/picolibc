/* ANSI C namespace clean utility typedefs */

/* This file defines various typedefs needed by the system calls that support
   the C library.  Basically, they're just the POSIX versions with an '_'
   prepended.  Targets shall use <machine/_types.h> to define their own
   internal types if desired.

   There are three define patterns used for type definitions.  Lets assume
   xyz_t is a user type.

   The internal type definition uses __machine_xyz_t_defined.  It is defined by
   <machine/_types.h> to disable a default definition in <sys/_types.h>. It
   must not be used in other files.

   User type definitions are guarded by __xyz_t_defined in glibc and
   _XYZ_T_DECLARED in BSD compatible systems.
*/

#ifndef	_SYS__TYPES_H
#define _SYS__TYPES_H

#include <machine/_types.h>
#include <sys/lock.h>

#ifndef __machine_off_t_defined
typedef long _off_t;
#endif

#if defined(__XMK__)
typedef signed char __pid_t;
#else
typedef int __pid_t;
#endif

#ifndef __machine_dev_t_defined
typedef short __dev_t;
#endif

#ifndef __machine_uid_t_defined
typedef unsigned short __uid_t;
#endif
#ifndef __machine_gid_t_defined
typedef unsigned short __gid_t;
#endif

#ifndef __machine_id_t_defined
typedef __uint32_t __id_t;
#endif

#ifndef __machine_off64_t_defined
__extension__ typedef long long _off64_t;
#endif

#ifdef __CYGWIN__
typedef _off64_t __off_t;
#else
typedef _off_t __off_t;
#endif

typedef _off64_t __loff_t;

#ifndef __machine_key_t_defined
typedef long __key_t;
#endif

/*
 * We need fpos_t for the following, but it doesn't have a leading "_",
 * so we use _fpos_t instead.
 */
#ifndef __machine_fpos_t_defined
typedef long _fpos_t;		/* XXX must match off_t in <sys/types.h> */
				/* (and must be `long' for now) */
#endif

#ifdef __LARGE64_FILES
#ifndef __machine_fpos64_t_defined
typedef _off64_t _fpos64_t;
#endif
#endif

#ifndef __machine_ssize_t_defined
#ifdef __SIZE_TYPE__
/* If __SIZE_TYPE__ is defined (gcc) we define ssize_t based on size_t.
   We simply change "unsigned" to "signed" for this single definition
   to make sure ssize_t and size_t only differ by their signedness. */
#define unsigned signed
typedef __SIZE_TYPE__ _ssize_t;
#undef unsigned
#else
#if defined(__INT_MAX__) && __INT_MAX__ == 2147483647
typedef int _ssize_t;
#else
typedef long _ssize_t;
#endif
#endif
#endif

#define __need_wint_t
#include <stddef.h>

#ifndef __machine_mbstate_t_defined
/* Conversion state information.  */
typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    unsigned char __wchb[4];
  } __value;		/* Value so far.  */
} _mbstate_t;
#endif

#ifndef __machine_flock_t_defined
typedef _LOCK_RECURSIVE_T _flock_t;
#endif

#ifndef __machine_iconv_t_defined
/* Iconv descriptor type */
typedef void *_iconv_t;
#endif

#define	_CLOCK_T_	unsigned long		/* clock() */
#define	_TIME_T_	long			/* time() */
#define _CLOCKID_T_ 	unsigned long
#define _TIMER_T_   	unsigned long

typedef	long		__suseconds_t;	/* microseconds (signed) */

#endif	/* _SYS__TYPES_H */
