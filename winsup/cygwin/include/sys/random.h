/* sys/random.h header file for Cygwin.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for details. */

#ifndef _SYS_RANDOM_H
#define _SYS_RANDOM_H

#include <_ansi.h>
#include <sys/types.h>

/* getrandom flags */
#define GRND_NONBLOCK	1
#define GRND_RANDOM	2

#ifdef __cplusplus
extern "C" {
#endif

ssize_t getrandom (void *__ptr, size_t __len, unsigned int __flags);
int getentropy (void *__ptr, size_t __len);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_RANDOM_H */
