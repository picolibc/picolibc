/*
FUNCTION
<<utoa>>---unsigned integer to string

INDEX
	utoa

SYNOPSIS
	#include <stdlib.h>
	char *utoa(unsigned <[value]>, char *<[str]>, int <[base]>);
	char *__utoa(unsigned <[value]>, char *<[str]>, int <[base]>);

DESCRIPTION
<<utoa>> converts the unsigned integer [<value>] to a null-terminated string
using the specified base, which must be between 2 and 36, inclusive.
<[str]> should be an array long enough to contain the converted
value, which in the worst case is sizeof(int)*8+1 bytes. 

RETURNS
A pointer to the string, <[str]>, or NULL if <[base]> is invalid.

PORTABILITY
<<utoa>> is non-ANSI.

No supporting OS subroutine calls are required.
*/

#include <stdlib.h>

char *
__utoa (unsigned value,
        char *str,
        int base)
{
  const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  int i, j;
  unsigned remainder;
  char c;
  
  /* Check base is supported. */
  if ((base < 2) || (base > 36))
    { 
      str[0] = '\0';
      return NULL;
    }  
    
  /* Convert to string. Digits are in reverse order.  */
  i = 0;
  do 
    {
      remainder = value % base;
      str[i++] = digits[remainder];
      value = value / base;
    } while (value != 0);  
  str[i] = '\0'; 
  
  /* Reverse string.  */
  for (j = 0, i--; j < i; j++, i--)
    {
      c = str[j];
      str[j] = str[i];
      str[i] = c; 
    }       
  
  return str;
}

char *  
utoa (unsigned value,
        char *str,
        int base)
{
  return __utoa (value, str, base);
}
