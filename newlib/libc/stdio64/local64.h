/*
 * Information local to this implementation of stdio64,
 * in particular, macros and private variables.
 */

#include "local.h"

#ifdef __LARGE64_FILES
extern _fpos64_t _EXFUN(__sseek64,(void *, _fpos64_t, int));
extern _fpos64_t _EXFUN(__sseek64_r,(struct _reent *, void *, _fpos64_t, int));
extern _fpos64_t _EXFUN(__sseek64_error,(void *, _fpos64_t, int));
extern _READ_WRITE_RETURN_TYPE _EXFUN(__swrite64,(void *, char const *, int));
extern _READ_WRITE_RETURN_TYPE _EXFUN(__swrite64_r,(struct _reent *, void *, 
                                      char const *, int));
#endif

