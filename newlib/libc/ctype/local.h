/* Modified (m) 2017 Thomas Wolff: fixed locale/wchar handling */

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

/* internal functions to translate between JP and Unicode */
/* note this is not applicable to Cygwin, where wchar_t is always Unicode,
   and should not be applicable to most other platforms either;
   * platforms for which wchar_t is not Unicode should be explicitly listed
   * the transformation should be applied to all non-Unicode locales
     (also Chinese, Korean, and even 8-bit locales such as *.CP1252)
   * for towupper and towlower, the result must be back-transformed
     into the respective locale encoding; currently NOT IMPLEMENTED
*/
#ifdef __CYGWIN__
/* Under Cygwin, wchar_t (or its extension wint_t) is Unicode */
#define _jp2uc(c) (c)
#define _jp2uc_l(c, l) (c)
#define _uc2jp_l(c, l) (c)
#else
wint_t _jp2uc (wint_t);
wint_t _jp2uc_l (wint_t, struct __locale_t *);
wint_t _uc2jp_l (wint_t, struct __locale_t *);
#endif
