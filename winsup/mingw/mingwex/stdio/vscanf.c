//  By aaronwl 2003-01-28 for mingw-msvcrt
//  Public domain: all copyrights disclaimed, absolutely no warranties

#include <stdarg.h>
#include <stdio.h>

int vscanf(const char * __restrict__ format, va_list arg) {
  return vfscanf(stdin, format, arg);
}
