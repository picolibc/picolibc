/* libc/sys/linux/sys/ioctl.h - ioctl prototype */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

#include <bits/ioctls.h>

int ioctl(int fd,int request,...);

#endif
