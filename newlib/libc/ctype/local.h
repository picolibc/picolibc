/* wctrans constants */

#include <_ansi.h>
#include "../locale/setlocale.h"

/* valid values for wctrans_t */
#define WCT_TOLOWER 1
#define WCT_TOUPPER 2

/* valid values for wctype_t */
#define WC_ALNUM	1
#define WC_ALPHA	2
#define WC_BLANK	3
#define WC_CNTRL	4
#define WC_DIGIT	5
#define WC_GRAPH	6
#define WC_LOWER	7
#define WC_PRINT	8
#define WC_PUNCT	9
#define WC_SPACE	10
#define WC_UPPER	11
#define WC_XDIGIT	12

/* internal function to translate JP to Unicode */
#ifdef __CYGWIN__
/* Under Cygwin, the incoming wide character is already given in UTF due
   to the requirements of the underlying OS. */
#define _jp2uc(c) (c)
#else
wint_t _EXFUN (_jp2uc, (wint_t));
#endif
