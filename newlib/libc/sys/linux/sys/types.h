/* libc/sys/linux/sys/types.h - The usual type zoo */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

/* Newlib has it's own time_t and clock_t definitions in 
 * libc/include/sys/types.h.  Repeat those here and
 * skip the kernel's definitions. */

#include <sys/config.h>
#include <machine/types.h>

#if !defined(__time_t_defined) && !defined(_TIME_T)
#define _TIME_T
#define __time_t_defined
typedef _TIME_T_ time_t;
#endif

#if !defined(__clock_t_defined) && !defined(_CLOCK_T)
#define _CLOCK_T
#define __clock_t_defined
typedef _CLOCK_T_ clock_t;
#endif

typedef unsigned int __socklen_t;
typedef unsigned int __useconds_t;

typedef __pid_t pid_t;
typedef __off_t off_t;
typedef __loff_t loff_t;
typedef __uint32_t uintptr_t;
typedef __int32_t intptr_t;

#ifndef __u_char_defined
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
#ifdef __GNUC__
__extension__ typedef long long quad_t;
__extension__ typedef unsigned long long u_quad_t;
#else
typedef struct
  {
    long int __val[2];
  } quad_t;
typedef struct
  {
    unsigned long __val[2];
  } u_quad_t;
#endif
typedef struct
  {
    int __val[2];
  } fsid_t;
#define __u_char_defined
#endif

#ifndef __daddr_t_defined
typedef int daddr_t;
typedef char *caddr_t;
# define __daddr_t_defined
#endif

typedef int clockid_t;

/* Time Value Specification Structures, P1003.1b-1993, p. 261 */

#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC
struct timespec {
  time_t  tv_sec;   /* Seconds */
  long    tv_nsec;  /* Nanoseconds */
};
#endif /* !_STRUCT_TIMESPEC */

#include <linux/types.h>
#define __mode_t_defined

#endif
