#include <stdio.h>

FILE* __cdecl
fopen64 (const char* filename, const char* mode)
{
  return fopen (filename, mode);
}
