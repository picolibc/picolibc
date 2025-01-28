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

#include <stdint.h>
#include <wctype.h>
#include <wchar.h>
#include <string.h>
#include "setlocale.h"

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

#define CLASS_none 0
#define CLASS_alnum     (1 << 0)
#define CLASS_alpha     (1 << 1)
#define CLASS_blank     (1 << 2)
#define CLASS_cntrl     (1 << 3)
#define CLASS_digit     (1 << 4)
#define CLASS_graph     (1 << 5)
#define CLASS_print     (1 << 6)
#define CLASS_punct     (1 << 7)
#define CLASS_space     (1 << 8)
#define CLASS_case      (1 << 9)
#define CLASS_lower     (1 << 10)
#define CLASS_upper     (1 << 11)
#define CLASS_xdigit    (1 << 12)

uint16_t
__ctype_table_lookup(wint_t ic, struct __locale_t *locale);

/* Japanese encoding types supported */
#define JP_JIS		1
#define JP_SJIS		2
#define JP_EUCJP	3

wint_t
__jp2uc (wint_t c, int type);

wint_t
__uc2jp (wint_t c, int type);

/*
   struct caseconv_entry describes the case conversion behaviour
   of a range of Unicode characters.
   It was designed to be compact for a minimal table size.
   The range is first...first + diff.
   Conversion behaviour for a character c in the respective range:
     mode == TOLO	towlower (c) = c + delta
     mode == TOUP	towupper (c) = c + delta
     mode == TOBOTH	(titling case characters)
			towlower (c) = c + 1
			towupper (c) = c - 1
     mode == TO1	capital/small letters are alternating
	delta == EVENCAP	even codes are capital
	delta == ODDCAP		odd codes are capital
			(this correlates with an even/odd first range value
			as of Unicode 10.0 but we do not rely on this)
   As of Unicode 10.0, the following field lengths are sufficient
	first: 17 bits
	diff: 8 bits
	delta: 17 bits
	mode: 2 bits
   The reserve of 4 bits (to limit the struct to 6 bytes)
   is currently added to the 'first' field;
   should a future Unicode version make it necessary to expand the others,
   the 'first' field could be reduced as needed, or larger ranges could
   be split up (reduce limit max=255 e.g. to max=127 or max=63 in 
   script mkcaseconv, check increasing table size).
 */
enum {TO1, TOLO, TOUP, TOBOTH};
enum {EVENCAP, ODDCAP};

struct caseconv_entry
{
  uint_least32_t first: 21;
  uint_least32_t diff: 8;
  uint_least32_t mode: 2;
#ifdef __MSP430__
  /*
   * MSP430 has 20-bit integers which the compiler attempts to use and
   * fails. Waste some memory to fix that.
   */
  int_least32_t delta;
#else
  int_least32_t delta: 17;
#endif
}
#ifdef _HAVE_BITFIELDS_IN_PACKED_STRUCTS
__attribute__((packed))
#endif
;

const struct caseconv_entry *
__caseconv_lookup(wint_t c, struct __locale_t *locale);
