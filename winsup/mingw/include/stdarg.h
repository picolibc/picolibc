/* 
 * stdarg.h 
 * Forwarding header to get gcc's stdarg.h and keep std namespace clean
 */

#ifndef RC_INVOKED 

#include <_mingw.h>

#ifdef __cplusplus

namespace __ginclude
{
  /* Include gcc's stdarg.h.  */
# include_next<stdarg.h>
}

/* If invocation was from the user program, push va_list into std. */  
#if defined  (_STDARG_H)

__BEGIN_CSTD_NAMESPACE
  using __ginclude::va_list;
__END_CSTD_NAMESPACE

/* Otherwise inject __gnuc_va_list into global.  */

#else

using __ginclude::__gnuc_va_list;

#endif /* _STDARG_H */

#else /* __cplusplus */

/* Just include gcc's stdarg.h.  */
#include_next<stdarg.h>

#endif

#endif /* RC_INVOKED */ 
