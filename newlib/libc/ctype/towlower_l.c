/*
Copyright (c) 2016 Corinna Vinschen <corinna@vinschen.de> 
Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling
 */
/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#define _DEFAULT_SOURCE
#include <wctype.h>
#include "local.h"

wint_t
towlower_l (wint_t c, locale_t locale)
{
#ifdef __MB_CAPABLE
  const struct caseconv_entry * cce = __caseconv_lookup(c, locale);

  if (cce)
    switch (cce->mode)
      {
      case TOLO:
	return c + cce->delta;
      case TOBOTH:
	return c + 1;
      case TO1:
	switch (cce->delta)
	  {
	  case EVENCAP:
	    if (!(c & 1))
	      return c + 1;
	    break;
	  case ODDCAP:
	    if (c & 1)
	      return c + 1;
	    break;
	  default:
	    break;
	  }
	default:
	  break;
      }

  return c;
#else
  (void) locale;
  return towlower(c);
#endif
}
