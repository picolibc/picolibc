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

#endif
