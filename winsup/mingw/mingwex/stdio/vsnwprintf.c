#include <stdarg.h>
#include <wchar.h>

int  vsnwprintf(wchar_t *buffer,  size_t n, const wchar_t * format, va_list argptr)
  { return _vsnwprintf( buffer, n, format, argptr );}
