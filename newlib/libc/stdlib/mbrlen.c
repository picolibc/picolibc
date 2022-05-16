#include <reent.h>
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifdef _REENT_THREAD_LOCAL
_Thread_local _mbstate_t _tls_mbrlen_state;
#endif

size_t
mbrlen(const char *__restrict s, size_t n, mbstate_t *__restrict ps)
{
#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      struct _reent *reent = _REENT;

      _REENT_CHECK_MISC(reent);
      ps = &(_REENT_MBRLEN_STATE(reent));
    }
#endif

  return mbrtowc(NULL, s, n, ps);
}
