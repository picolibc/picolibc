//  By aaronwl 2003-01-28 for mingw-msvcrt.
//  Public domain: all copyrights disclaimed, absolutely no warranties.

#include <stdarg.h>
#include <wchar.h>
#include <stdio.h>

int vwscanf(const wchar_t * __restrict__ format, va_list arg) {
  return vfwscanf(stdin, format, arg);
}
