/*
Copyright (c) 2002 Red Hat Incorporated.
All rights reserved.
Modified (m) 2017 Thomas Wolff to refer to generated Unicode data tables.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

The name of Red Hat Incorporated may not be used to endorse
or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS   
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
