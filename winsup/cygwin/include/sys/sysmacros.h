/* sys/sysmacros.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H

#include <sys/types.h>

_ELIDABLE_INLINE int gnu_dev_major(dev_t);
_ELIDABLE_INLINE int gnu_dev_minor(dev_t);
_ELIDABLE_INLINE dev_t gnu_dev_makedev(int, int);

_ELIDABLE_INLINE int
gnu_dev_major(dev_t dev)
{
	return (int)(((dev) >> 16) & 0xffff);
}

_ELIDABLE_INLINE int
gnu_dev_minor(dev_t dev)
{
	return (int)((dev) & 0xffff);
}

_ELIDABLE_INLINE dev_t
gnu_dev_makedev(int maj, int min)
{
	return (((maj) << 16) | ((min) & 0xffff));
}

#define major(dev) gnu_dev_major(dev)
#define minor(dev) gnu_dev_minor(dev)
#define makedev(maj, min) gnu_dev_makedev(maj, min)

#endif /* _SYS_SYSMACROS_H */
