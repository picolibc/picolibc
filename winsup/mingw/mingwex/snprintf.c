#include <stdarg.h>
#include <stdio.h>

int snprintf(char* buffer, size_t n, const char* format, ...)
{
  int retval;
  va_list argptr;
         
  va_start( argptr, format );
  retval = _vsnprintf( buffer, n, format, argptr );
  va_end( argptr );
  return retval;
}

int vsnprintf (char* s, size_t n, const char* format, va_list arg)
  { return _vsnprintf ( s, n, format, arg); }
