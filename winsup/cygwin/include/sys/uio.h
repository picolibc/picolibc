#ifndef _UIO_H_
#define _UIO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* For size_t */
#include <stddef.h>
/* For ssize_t */
#include <sys/types.h>

/*
 * Define the uio buffers used for writev, readv.
 */

struct iovec {
	caddr_t iov_base;
	int iov_len;
};

#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif /* _UIO_H_ */
