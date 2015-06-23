/*
FUNCTION
<<itoa>>---integer to string

INDEX
	itoa

ANSI_SYNOPSIS
	#include <stdlib.h>
	char *itoa(int <[value]>, char *<[str]>, int <[base]>);
	char *__itoa(int <[value]>, char *<[str]>, int <[base]>);

DESCRIPTION
<<itoa>> converts the integer <[value]> to a null-terminated string
using the specified base, which must be between 2 and 36, inclusive.
If <[base]> is 10, <[value]> is treated as signed and the string will be 
prefixed with '-' if negative. For all other bases, <[value]> is treated as
unsigned. <[str]> should be an array long enough to contain the converted
value, which in the worst case is sizeof(int)*8+1 bytes. 

RETURNS
A pointer to the string, <[str]>, or NULL if <[base]> is invalid.

PORTABILITY
<<itoa>> is non-ANSI.

No supporting OS subroutine calls are required.
*/

#include <stdlib.h>

char *
_DEFUN (__itoa, (value, str, base),
        int value _AND
        char *str _AND 
        int base)
{
  unsigned uvalue;
  int i = 0;
  
  /* Check base is supported. */
  if ((base < 2) || (base > 36))
    { 
      str[0] = '\0';
      return NULL;
    }  
    
  /* Negative numbers are only supported for decimal.
   * Cast to unsigned to avoid overflow for maximum negative value.  */ 
  if ((base == 10) && (value < 0))
    {              
      str[i++] = '-';
      uvalue = (unsigned)-value;
    }
  else
    uvalue = (unsigned)value;
  
  __utoa (uvalue, &str[i], base);
  return str;
}

char *  
_DEFUN (itoa, (value, str, base),
        int value _AND
        char *str _AND
        int base)
{
  return __itoa (value, str, base);
}
