#include <wchar.h>

float wcstof( const wchar_t *nptr, wchar_t **endptr)
{
  return (wcstod(nptr, endptr));
}
