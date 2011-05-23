#define __NO_CTYPE_LINES
#include <ctype.h>

int __cdecl isblank (int c)
{return (_isctype(c, _BLANK) || c == '\t');}
