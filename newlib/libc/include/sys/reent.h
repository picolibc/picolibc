/* This header file provides the reentrancy.  */

/* WARNING: All identifiers here must begin with an underscore.  This file is
   included by stdio.h and others and we therefore must only use identifiers
   in the namespace allotted to us.  */

#ifndef _SYS_REENT_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _SYS_REENT_H_

#include <_ansi.h>
#include <stddef.h>
#include <sys/_types.h>

#define _NULL 0

struct _reent;

#define _REENT NULL
#define _GLOBAL_REENT NULL

/*
 * Since _REENT is defined as NULL, this macro ensures that calls to
 * CHECK_INIT() do not automatically fail.
 */
#define _REENT_IS_NULL(_ptr) 0

#define _REENT_CHECK_MP(ptr)		/* nothing */

extern NEWLIB_THREAD_LOCAL void (*_tls_cleanup)(struct _reent *);
#define _REENT_CLEANUP(_ptr) (_tls_cleanup)
extern NEWLIB_THREAD_LOCAL struct _Bigint **_tls_mp_freelist;
#define _REENT_MP_FREELIST(_ptr) (_tls_mp_freelist)
extern NEWLIB_THREAD_LOCAL struct _Bigint *_tls_mp_p5s;
#define _REENT_MP_P5S(_ptr) (_tls_mp_p5s)
extern NEWLIB_THREAD_LOCAL struct _Bigint *_tls_mp_result;
#define _REENT_MP_RESULT(_ptr) (_tls_mp_result)
extern NEWLIB_THREAD_LOCAL int _tls_mp_result_k;
#define _REENT_MP_RESULT_K(_ptr) (_tls_mp_result_k)

void _reclaim_reent (struct _reent *);

#ifdef __cplusplus
}
#endif
#endif /* _SYS_REENT_H_ */
