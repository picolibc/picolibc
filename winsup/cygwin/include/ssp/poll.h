#ifndef _SSP_POLL_H_
#define _SSP_POLL_H_

#include <ssp/ssp.h>

#if __SSP_FORTIFY_LEVEL > 0
__BEGIN_DECLS

__ssp_decl(int, poll, (struct pollfd *__fds, nfds_t __nfds, int __timeout))
{
  __ssp_check (__fds, __nfds * sizeof(*__fds), __ssp_bos);
  return __ssp_real_poll (__fds, __nfds, __timeout);
}

#if __GNU_VISIBLE
__ssp_decl(int, ppoll, (struct pollfd *__fds, nfds_t __nfds, const struct timespec *__timeout_ts, const sigset_t *__sigmask))
{
  __ssp_check (__fds, __nfds * sizeof(*__fds), __ssp_bos);
  return __ssp_real_ppoll (__fds, __nfds, __timeout_ts, __sigmask);
}
#endif

__END_DECLS

#endif /* __SSP_FORTIFY_LEVEL > 0 */
#endif /* _SSP_POLL_H_ */
