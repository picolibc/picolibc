#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include "_ansi.h"
#include <sys/signal.h>

_BEGIN_STD_C

typedef int	sig_atomic_t;		/* Atomic entity type (ANSI) */

#if defined(__STDC__) || defined(__cplusplus)
#define SIG_DFL ((void (*)(int))0)	/* Default action */
#define SIG_IGN ((void (*)(int))1)	/* Ignore action */
#define SIG_ERR ((void (*)(int))-1)	/* Error return */
#else
#define SIG_DFL ((void (*)())0)		/* Default action */
#define SIG_IGN ((void (*)())1)		/* Ignore action */
#define SIG_ERR ((void (*)())-1)	/* Error return */
#endif

typedef void (*_sig_func_ptr) (int);

struct _reent;

_sig_func_ptr _EXFUN(_signal_r, (struct _reent *, int, _sig_func_ptr));
int	_EXFUN(_raise_r, (struct _reent *, int));

#ifndef _REENT_ONLY
_sig_func_ptr _EXFUN(signal, (int, _sig_func_ptr));
int	_EXFUN(raise, (int));
#endif

_END_STD_C

#endif /* _SIGNAL_H_ */
