/* sys/ioctl.h */

#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

#include <sys/cdefs.h>

/* /dev/windows ioctls */

#define WINDOWS_POST	0	/* Set write() behavior to PostMessage() */
#define WINDOWS_SEND	1	/* Set write() behavior to SendMessage() */
#define WINDOWS_HWND	2	/* Set hWnd for read() calls */

__BEGIN_DECLS

int ioctl (int __fd, int __cmd, void *);

__END_DECLS

#endif
