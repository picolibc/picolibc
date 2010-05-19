#include <locale.h>
#include <string.h>
#include <stdlib.h>

static inline
unsigned int get_codepage (void)
{
  /* locale :: "lang[_country[.code_page]]" | ".code_page"  */
  char * cp_string;
  if ((cp_string = strchr (setlocale(LC_CTYPE, NULL), '.')))
    return  ((unsigned) atoi (cp_string + 1));
  return 0;
}
