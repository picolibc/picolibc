#ifndef _UIO_H_
#define _UIO_H_

/* For size_t */
#include <stddef.h>
/* For ssize_t */
#include <sys/types.h>

#include <sys/cdefs.h>

__BEGIN_DECLS

/*
 * Define the uio buffers used for writev, readv.
 */

struct iovec {
	caddr_t iov_base;
	int iov_len;
};

extern int readv __P ((int filedes, const struct iovec *vector, size_t count));
extern int writev __P ((int filedes, const struct iovec *vector, size_t count));

__END_DECLS

#endif /* _UIO_H_ */
