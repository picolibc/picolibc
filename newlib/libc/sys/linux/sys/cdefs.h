/* libc/sys/linux/sys/cdefs.h - Helper macros for K&R vs. ANSI C compat. */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_CDEFS_H
#define _SYS_CDEFS_H

/*
 * Note: the goal here is not compatibility to K&R C. Since we know that we
 * have GCC which understands ANSI C perfectly well, we make use of this.
 */

#define __P(args)	args
#define __PMT(args)	args
#define __const		const
#define __signed	signed
#define __volatile	volatile
#define __DOTS    	, ...
#define __THROW

#define __ptr_t void *
#define __long_double_t  long double

#define __attribute_malloc__
#define __attribute_pure__
#define __attribute_format_strfmon__(a,b)
#define __flexarr      [0]

#ifdef  __cplusplus
# define __BEGIN_DECLS  extern "C" {
# define __END_DECLS    }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

#ifndef __BOUNDED_POINTERS__
# define __bounded      /* nothing */
# define __unbounded    /* nothing */
# define __ptrvalue     /* nothing */
#endif

#endif /* _SYS_CDEFS_H */
