#include <_ansi.h>
#include <../ctype/local.h>

/* internal function to compute width of wide char. */
int _EXFUN (__wcwidth, (wint_t));

/* Defined in locale/locale.c.  Returns a value != 0 if the current
   language is assumed to use CJK fonts. */
int __locale_cjk_lang ();
