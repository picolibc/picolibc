/*  Wide char wrapper for strtold
 *  Revision history:
 *  6 Nov 2002	Initial version.
 *
 *  Contributor:   Danny Smith <dannysmith@users.sourceforege.net>
 */

 /* This routine has been placed in the public domain.*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <locale.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

extern int __asctoe64(const char * __restrict__ ss,
	       short unsigned int * __restrict__ y);


static __inline__ unsigned int get_codepage (void)
{
  char* cp;

  /*
    locale :: "lang[_country[.code_page]]" 
               | ".code_page"
  */
  if ((cp = strchr(setlocale(LC_CTYPE, NULL), '.')))
    return atoi( cp + 1);
  else
    return 0;
}

long double wcstold (const wchar_t * __restrict__ wcs, wchar_t ** __restrict__ wcse)
{
  char * cs;
  int i;
  int lenldstr;
  union
  {
    unsigned short int us[6];
    long double ld;
  } xx;

  unsigned int cp = get_codepage ();   
  
  /* Allocate enough room for (possibly) mb chars */
  cs = (char *) malloc ((wcslen(wcs)+1) * MB_CUR_MAX);

  if (cp == 0) /* C locale */
    {
      for (i = 0; (wcs[i] != 0) && wcs[i] <= 255; i++)
        cs[i] = (char) wcs[i];                                                                                                                                                                                                                                                                                                   
      cs[i]  = '\0';
    }
  else
    {
      int nbytes = -1;
      int mb_len = 0;
      /* loop through till we hit null or invalid character */
      for (i = 0; (wcs[i] != 0) && (nbytes != 0); i++)
	{
     	  nbytes = WideCharToMultiByte(cp, WC_COMPOSITECHECK | WC_SEPCHARS,
				       wcs + i, 1, cs + mb_len, MB_CUR_MAX,
				       NULL, NULL);
	  mb_len += nbytes;
	}
      cs[mb_len] = '\0';
    }
  lenldstr =  __asctoe64( cs, xx.us);
  free (cs);
  if (wcse)
    *wcse = (wchar_t*) wcs + lenldstr;
  return xx.ld;
}
