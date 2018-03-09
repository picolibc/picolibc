/* Modified (m) 2017 Thomas Wolff: revise Unicode and locale/wchar handling */
#include <_ansi.h>
#include <wctype.h>
//#include <errno.h>
#include "local.h"

enum {EVENCAP, ODDCAP};
enum {TO1, TOLO, TOUP, TOBOTH};
static struct caseconv_entry {
  unsigned int first: 21;
  unsigned short diff: 8;
  unsigned char mode: 2;
  int delta: 17;
} __attribute__ ((packed))
caseconv_table [] = {
#include "caseconv.t"
};
#define first(ce)	ce.first
#define last(ce)	(ce.first + ce.diff)

/* auxiliary function for binary search in interval properties table */
static const struct caseconv_entry *
bisearch(wint_t ucs, const struct caseconv_entry *table, int max)
{
  int min = 0;
  int mid;

  if (ucs < first(table[0]) || ucs > last(table[max]))
    return 0;
  while (max >= min)
    {
      mid = (min + max) / 2;
      if (ucs > last(table[mid]))
	min = mid + 1;
      else if (ucs < first(table[mid]))
	max = mid - 1;
      else
	return &table[mid];
    }
  return 0;
}

static wint_t
toulower (wint_t c)
{
  const struct caseconv_entry * cce =
    bisearch(c, caseconv_table,
             sizeof(caseconv_table) / sizeof(*caseconv_table) - 1);
  if (cce)
    switch (cce->mode) {
      case TOLO: return c + cce->delta;
      case TOBOTH: return c + 1;
      case TO1: switch (cce->delta) {
        case EVENCAP: if (!(c & 1)) return c + 1; break;
        case ODDCAP: if (c & 1) return c + 1; break;
      }
    }
  else
    return c;
}

static wint_t
touupper (wint_t c)
{
  const struct caseconv_entry * cce =
    bisearch(c, caseconv_table,
             sizeof(caseconv_table) / sizeof(*caseconv_table) - 1);
  if (cce)
    switch (cce->mode) {
      case TOUP: return c + cce->delta;
      case TOBOTH: return c - 1;
      case TO1: switch (cce->delta) {
        case EVENCAP: if (c & 1) return c - 1; break;
        case ODDCAP: if (!(c & 1)) return c - 1; break;
      }
    }
  else
    return c;
}

wint_t
towctrans_l (wint_t c, wctrans_t w, struct __locale_t *locale)
{
  wint_t u = _jp2uc_l (c, locale);
  wint_t res;
  if (w == WCT_TOLOWER)
    res = toulower (u);
  else if (w == WCT_TOUPPER)
    res = touupper (u);
  else
    {
      // skipping the errno setting that was previously involved
      // by delegating to towctrans; it was causing trouble (cygwin crash)
      // and there is no errno specified for towctrans
      return c;
    }
  if (res != u)
    return _uc2jp_l (res, locale);
  else
    return c;
}
