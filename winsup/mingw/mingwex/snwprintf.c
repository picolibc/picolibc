#include <stdarg.h>
#include <wchar.h>

int  swnprintf(wchar_t* buffer, size_t n, const wchar_t* format, ...)
{
    int retval;
    va_list argptr;
          
    va_start( argptr, format );
    retval = _vswnprintf( buffer, n, format, argptr );
    va_end( argptr );
    return retval;
}

int  vswnprintf(wchar_t *buffer,  size_t n, const wchar_t * format, va_list argptr)
{ return _vswnprintf( buffer, n, format, argptr );}
