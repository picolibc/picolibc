/*
 * cygwin/rdevio.h header file for Cygwin.
 *
 * Written by C. Vinschen.
 */

#ifndef _CYGWIN_RDEVIO_H
#define _CYGWIN_RDEVIO_H

/* structure for RDIOCDOP - raw device operation */
struct rdop {
	short		rd_op;
	unsigned long	rd_parm;
};

/* Raw device operations */
#define RDSETBLK	1	/* set buffer for driver */

/* structure for RDIOCGET - get raw device */
struct rdget {
	unsigned long	bufsiz;
};

/*
 * ioctl commands
*/
#define RDIOCDOP	_IOW('r', 128, struct rdop)
#define RDIOCGET	_IOR('r', 129, struct rdget)

#endif /* _CYGWIN_RDEVIO_H */
