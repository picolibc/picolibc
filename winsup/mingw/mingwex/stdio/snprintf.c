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
