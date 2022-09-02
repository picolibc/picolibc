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

#define _REENT_CHECK_RAND48(ptr)	/* nothing */
#define _REENT_CHECK_MP(ptr)		/* nothing */
#define _REENT_CHECK_TM(ptr)		/* nothing */
#define _REENT_CHECK_ASCTIME_BUF(ptr)	/* nothing */
#define _REENT_CHECK_EMERGENCY(ptr)	/* nothing */
#define _REENT_CHECK_MISC(ptr)		/* nothing */
#define _REENT_CHECK_SIGNAL_BUF(ptr)	/* nothing */

extern NEWLIB_THREAD_LOCAL void (*_tls_cleanup)(struct _reent *);
#define _REENT_CLEANUP(_ptr) (_tls_cleanup)
extern NEWLIB_THREAD_LOCAL char _tls_emergency;
#define _REENT_EMERGENCY(_ptr) (_tls_emergency)
extern NEWLIB_THREAD_LOCAL int _tls_getdate_err;
#define _REENT_GETDATE_ERR_P(_ptr) (&_tls_getdate_err)
extern NEWLIB_THREAD_LOCAL int _tls_inc;
#define _REENT_INC(_ptr) (_tls_inc)
extern NEWLIB_THREAD_LOCAL char _tls_l64a_buf[8];
#define _REENT_L64A_BUF(_ptr) (_tls_l64a_buf)
extern NEWLIB_THREAD_LOCAL _mbstate_t _tls_mblen_state;
#define _REENT_MBLEN_STATE(_ptr) (_tls_mblen_state)
extern NEWLIB_THREAD_LOCAL _mbstate_t _tls_mbrlen_state;
#define _REENT_MBRLEN_STATE(_ptr) (_tls_mbrlen_state)
extern NEWLIB_THREAD_LOCAL _mbstate_t _tls_mbrtowc_state;
#define _REENT_MBRTOWC_STATE(_ptr) (_tls_mbrtowc_state)
extern NEWLIB_THREAD_LOCAL _mbstate_t _tls_mbsrtowcs_state;
#define _REENT_MBSRTOWCS_STATE(_ptr) (_tls_mbsrtowcs_state)
extern NEWLIB_THREAD_LOCAL _mbstate_t _tls_mbtowc_state;
#define _REENT_MBTOWC_STATE(_ptr) (_tls_mbtowc_state)
extern NEWLIB_THREAD_LOCAL struct _Bigint **_tls_mp_freelist;
#define _REENT_MP_FREELIST(_ptr) (_tls_mp_freelist)
extern NEWLIB_THREAD_LOCAL struct _Bigint *_tls_mp_p5s;
#define _REENT_MP_P5S(_ptr) (_tls_mp_p5s)
extern NEWLIB_THREAD_LOCAL struct _Bigint *_tls_mp_result;
#define _REENT_MP_RESULT(_ptr) (_tls_mp_result)
extern NEWLIB_THREAD_LOCAL int _tls_mp_result_k;
#define _REENT_MP_RESULT_K(_ptr) (_tls_mp_result_k)
extern NEWLIB_THREAD_LOCAL char *_tls_strtok_last;
#define _REENT_STRTOK_LAST(_ptr) (_tls_strtok_last)
extern NEWLIB_THREAD_LOCAL struct __tm _tls_localtime_buf;
#define _REENT_TM(_ptr) (&_tls_localtime_buf)
extern NEWLIB_THREAD_LOCAL _mbstate_t _tls_wctomb_state;
#define _REENT_WCTOMB_STATE(_ptr) (_tls_wctomb_state)
extern NEWLIB_THREAD_LOCAL _mbstate_t _tls_wcrtomb_state;
#define _REENT_WCRTOMB_STATE(_ptr) (_tls_wcrtomb_state)
extern NEWLIB_THREAD_LOCAL _mbstate_t _tls_wcsrtombs_state;
#define _REENT_WCSRTOMBS_STATE(_ptr) (_tls_wcsrtombs_state)

void _reclaim_reent (struct _reent *);

#ifdef __cplusplus
}
#endif
#endif /* _SYS_REENT_H_ */
