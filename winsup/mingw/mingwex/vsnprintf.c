#include <stdarg.h>
#include <stdio.h>

int vsnprintf (char* s, size_t n, const char* format, va_list arg)
  { return _vsnprintf ( s, n, format, arg); }
