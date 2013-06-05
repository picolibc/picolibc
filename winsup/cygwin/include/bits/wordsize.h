/* wordsize.h - Linux compatibility */

#ifndef _WORDSIZE_H
#define _WORDSIZE_H 1
#ifdef __x86_64__
# define __WORDSIZE     64
# define __WORDSIZE_COMPAT32    1
#else
# define __WORDSIZE     32
#endif
#endif /*_WORDSIZE_H*/
