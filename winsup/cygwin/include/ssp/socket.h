#ifndef _SSP_SOCKET_H_
#define _SSP_SOCKET_H_

#include <ssp/ssp.h>

#if __SSP_FORTIFY_LEVEL > 0
__BEGIN_DECLS

__ssp_redirect0(ssize_t, recv, \
    (int __fd, void *__buf, size_t __len, int __flags), \
    (__fd, __buf, __len, __flags));

__ssp_redirect0(ssize_t, recvfrom, \
    (int __fd, void *__buf, size_t __len, int __flags, struct sockaddr *__from, socklen_t *__fromlen), \
    (__fd, __buf, __len, __flags, __from, __fromlen));

__END_DECLS

#endif /* __SSP_FORTIFY_LEVEL > 0 */
#endif /* _SSP_SOCKET_H_ */
