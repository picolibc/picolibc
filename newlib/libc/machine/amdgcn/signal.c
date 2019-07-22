#include <signal.h>
#include <errno.h>

_sig_func_ptr
signal (int sig,
	_sig_func_ptr func)
{
  errno = EINVAL;
  return NULL;
}
