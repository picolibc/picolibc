/* nlsfuncs.cc: NLS helper functions

   Copyright 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <winnls.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
/* Internal headers from newlib */
#include "../locale/timelocal.h"
#include "../locale/lnumeric.h"
#include "../locale/lmonetary.h"

static char *lc_time_buf;
static char *lc_numeric_buf;
static char *lc_monetary_buf;

#define _LC(x)	&lc_##x##_ptr,lc_##x##_end-lc_##x##_ptr

#define getlocaleinfo(category,type) \
	    __getlocaleinfo(lcid,(type),_LC(category),f_wctomb,charset)
#define eval_datetimefmt(type,force) \
	    __eval_datetimefmt(lcid,(type),(force),&lc_time_ptr,\
			       lc_time_end-lc_time_ptr,f_wctomb, charset)

/* Vista and later.  Not defined in w32api yet. */
extern "C" {
WINBASEAPI LCID WINAPI LocaleNameToLCID (LPCWSTR, DWORD);
};

static char last_locale[ENCODING_LEN + 1];
static LCID last_lcid;

/* Fetch LCID from POSIX locale specifier.
   Return values:

     -1: Invalid locale
      0: C or POSIX
     >0: LCID
*/
static LCID
__get_lcid_from_locale (const char *name)
{
  char locale[ENCODING_LEN + 1];
  char *c;
  LCID lcid;

  if (!strcmp (name, last_locale))
    {
      debug_printf ("LCID=0x%04x", last_lcid);
      return last_lcid;
    }
  stpcpy (last_locale, name);
  stpcpy (locale, name);
  /* Drop charset and modifier */
  c = strchr (locale, '.');
  if (!c)
    c = strchr (locale, '@');
  if (c)
    *c = '\0';
  /* "POSIX" already converted to "C" in loadlocale. */
  if (!strcmp (locale, "C"))
    return 0;
  /* Convert to form understood by LocaleNameToLCID */
  c = strchr (locale, '_');
  if (c)
    *c = '-';
  if (wincap.has_localenames ())
    {
      wchar_t wlocale[ENCODING_LEN + 1];
      mbstowcs (wlocale, locale, ENCODING_LEN + 1);
      lcid = LocaleNameToLCID (wlocale, 0);
      last_lcid = lcid ?: (LCID) -1;
      debug_printf ("LCID=0x%04x", last_lcid);
      return last_lcid;
    }
  /* Pre-Vista we have to loop through the LCID values and see if they
     match language and TERRITORY. */
  if (c)
    *c++ = '\0';
  /* locale now points to the language, c points to the TERRITORY */
  const char *language = locale;
  const char *territory = c;
  LCID lang, sublang;
  char iso[10];

  /* In theory the lang part takes 10 bits (0x3ff), but up to Windows 2003 R2
     the highest lang value is 0x81. */
  for (lang = 1; lang <= 0x81; ++lang)
    if (GetLocaleInfo (lang, LOCALE_SISO639LANGNAME, iso, 10)
	&& !strcmp (language, iso))
      break;
  if (lang > 0x81)
    lcid = 0;
  else if (!territory)
    lcid = lang;
  else
    {
      /* In theory the sublang part takes 7 bits (0x3f), but up to
	 Windows 2003 R2 the highest sublang value is 0x14. */
      for (sublang = 1; sublang <= 0x14; ++sublang)
	{
	  lcid = (sublang << 10) | lang;
	  if (GetLocaleInfo (lcid, LOCALE_SISO3166CTRYNAME, iso, 10)
	      && !strcmp (territory, iso))
	    break;
	}
      if (sublang > 0x14)
	lcid = 0;
    }
  last_lcid = lcid ?: (LCID) -1;
  debug_printf ("LCID=0x%04x", last_lcid);
  return last_lcid;
}

/* Never returns -1, *iff* s is not NULL.  Just skips invalid chars
   instead.  s==NULL returns -1 since it's used to recognize invalid
   strings in the used charset. */
static size_t
lc_wcstombs (wctomb_p f_wctomb, const char *charset,
	     char *s, const wchar_t *pwcs, size_t n)
{
  char *ptr = s;
  size_t max = n;
  char buf[8];
  size_t i, bytes, num_to_copy;
  mbstate_t state;

  memset (&state, 0, sizeof state);
  if (s == NULL)
    {
      size_t num_bytes = 0;
      while (*pwcs != 0)
	{
	  bytes = f_wctomb (_REENT, buf, *pwcs++, charset, &state);
	  if (bytes == (size_t) -1)
	    return (size_t) -1;
	  num_bytes += bytes;
	}
      return num_bytes;
    }
  while (n > 0)
    {
      bytes = f_wctomb (_REENT, buf, *pwcs, charset, &state);
      if (bytes == (size_t) -1)
	{
	  memset (&state, 0, sizeof state);
	  ++pwcs;
	  continue;
	}
      num_to_copy = (n > bytes ? bytes : n);
      for (i = 0; i < num_to_copy; ++i)
	*ptr++ = buf[i];

      if (*pwcs == 0x00)
	return ptr - s - (n >= bytes);
      ++pwcs;
      n -= num_to_copy;
    }
  return max;
}

/* Never returns -1.  Invalid sequences are translated to replacement
   wide-chars. */
static size_t
lc_mbstowcs (mbtowc_p f_mbtowc, const char *charset,
	     wchar_t *pwcs, const char *s, size_t n)
{
  size_t ret = 0;
  char *t = (char *) s;
  size_t bytes;
  mbstate_t state;

  memset (&state, 0, sizeof state);
  if (!pwcs)
    n = 1;
  while (n > 0)
    {
      bytes = f_mbtowc (_REENT, pwcs, t, MB_CUR_MAX, charset, &state);
      if (bytes == (size_t) -1)
        {
          state.__count = 0;
          bytes = 1;
	  if (pwcs)
	    *pwcs = L' ';
        }
      else if (bytes == 0)
        break;
      t += bytes;
      ++ret;
      if (pwcs)
	{
	  ++pwcs;
	  --n;
	}
    }
  return ret;
}

static char *
__getlocaleinfo (LCID lcid, LCTYPE type, char **ptr, size_t size,
		 wctomb_p f_wctomb, const char *charset)
{
  wchar_t wbuf[80];
  size_t num;
  char *ret;

  GetLocaleInfoW (lcid, type, wbuf, 80);
  num = lc_wcstombs (f_wctomb, charset, ret = *ptr, wbuf, size);
  *ptr += num + 1;
  return ret;
}

static UINT
getlocaleint (LCID lcid, LCTYPE type)
{
  UINT val;
  return GetLocaleInfoW (lcid, type | LOCALE_RETURN_NUMBER, (PWCHAR) &val,
			 sizeof val) ? val : 0;
}

static char *
__eval_datetimefmt (LCID lcid, LCTYPE type, int force, char **ptr,
		    size_t size, wctomb_p f_wctomb, const char *charset)
{
  wchar_t buf[80];
  wchar_t fc;
  size_t num;
  mbstate_t mb;
  size_t idx;
  const char *day_str = "edaA";
  const char *mon_str = "mmbB";
  const char *year_str = "yyyY";
  const char *hour12_str = "lI";
  const char *hour24_str = "kH";
  const char *t_str;
  char *ret = *ptr;
  char *p = *ptr;

  GetLocaleInfoW (lcid, type, buf, 80);
  memset (&mb, 0, sizeof mb);
  for (wchar_t *fmt = buf; *fmt; ++fmt)
    switch (fc = *fmt)
      {
      case L'\'':
	if (fmt[1] == L'\'')
	  *p++ = '\'';
	else
	  while (fmt[1] && *++fmt != L'\'')
	    {
	      num = f_wctomb (_REENT, p, *fmt, charset, &mb);
	      if (num == (size_t) -1)
		memset (&mb, 0, sizeof mb);
	      else
		p += num;
	    }
	break;
      case L'd':
      case L'M':
      case L'y':
	t_str = (fc == L'd' ? day_str : fc == L'M' ? mon_str : year_str);
	if (fc == L'y')
	  force = 0;
	for (idx = 1; fmt[1] == fc; ++idx, ++fmt);
	if (--idx > 3)
	  idx = 3;
	if (force && idx == 3)
	  idx = 2;
	*p++ = '%';
	*p++ = t_str[idx];
	break;
      case L'g':
	break;
      case L'h':
      case L'H':
	t_str = (fc == L'h' || force ? hour12_str : hour24_str);
	idx = 0;
	if (fmt[1] == fc)
	  {
	    ++fmt;
	    idx = 1;
	  }
	*p++ = '%';
	*p++ = t_str[idx];
	break;
      case L'm':
      case L's':
      case L't':
	if (fmt[1] == fc)
	  ++fmt;
	*p++ = '%';
	*p++ = (fc == L'm' ? 'M' : fc == L's' ? 'S' : 'p');
	break;
      case L'\t':
      case L'\n':
      case L'%':
	*p++ = '%';
	*p++ = (char) fc;
	break;
      default:
	num = f_wctomb (_REENT, p, *fmt, charset, &mb);
	if (num == (size_t) -1)
	  memset (&mb, 0, sizeof mb);
	else
	  p += num;
	break;
      }
  *p++ = '\0';
  *ptr = p;
  return ret;
}

/* Convert Windows grouping format into POSIX grouping format. */
static char *
conv_grouping (LCID lcid, LCTYPE type, char **lc_ptr)
{
  char buf[10]; /* Per MSDN max size of LOCALE_SGROUPING element incl. NUL */
  bool repeat = false;
  char *ptr = *lc_ptr;
  char *ret = ptr;

  GetLocaleInfoA (lcid, type, buf, 10);
  /* Convert Windows grouping format into POSIX grouping format. */
  for (char *c = buf; *c; ++c)
    {
      if (*c < '0' || *c > '9')
      	continue;
      char val = *c - '0';
      if (!val)
      	{
	  repeat = true;
	  break;
	}
      *ptr++ = val;
    }
  if (!repeat)
    *ptr++ = CHAR_MAX;
  *ptr++ = '\0';
  *lc_ptr = ptr;
  return ret;
}

/* Called from newlib's setlocale() via __time_load_locale() if category
   is LC_TIME.  Returns LC_TIME values fetched from Windows locale data
   in the structure pointed to by _time_locale.  This is subsequently
   accessed by functions like nl_langinfo, strftime, strptime. */
extern "C" int
__set_lc_time_from_win (const char *name, struct lc_time_T *_time_locale,
			wctomb_p f_wctomb, const char *charset)
{
  LCID lcid = __get_lcid_from_locale (name);
  if (!lcid || lcid == (LCID) -1)
    return lcid;

  char *new_lc_time_buf = (char *) malloc (4096);
  const char *lc_time_end = new_lc_time_buf + 4096;

  if (!new_lc_time_buf)
    return -1;
  char *lc_time_ptr = new_lc_time_buf;
  /* mon */
  for (int i = 0; i < 12; ++i)
    _time_locale->mon[i] = getlocaleinfo (time, LOCALE_SABBREVMONTHNAME1 + i);
  /* month and alt_month */
  for (int i = 0; i < 12; ++i)
    _time_locale->month[i] = _time_locale->alt_month[i]
			   = getlocaleinfo (time, LOCALE_SMONTHNAME1 + i);
  /* wday */
  _time_locale->wday[0] = getlocaleinfo (time, LOCALE_SABBREVDAYNAME7);
  for (int i = 0; i < 6; ++i)
    _time_locale->wday[i + 1] = getlocaleinfo (time,
					       LOCALE_SABBREVDAYNAME1 + i);
  /* weekday */
  _time_locale->weekday[0] = getlocaleinfo (time, LOCALE_SDAYNAME7);
  for (int i = 0; i < 6; ++i)
    _time_locale->weekday[i + 1] = getlocaleinfo (time, LOCALE_SDAYNAME1 + i);
  /* X_fmt */
  _time_locale->X_fmt = eval_datetimefmt (LOCALE_STIMEFORMAT, 0);
  /* x_fmt */
  _time_locale->x_fmt = eval_datetimefmt (LOCALE_SSHORTDATE, 0);
  /* c_fmt */
  _time_locale->c_fmt = eval_datetimefmt (LOCALE_SLONGDATE, 1);
  --lc_time_ptr;
  *lc_time_ptr++ = ' ';
  eval_datetimefmt (LOCALE_STIMEFORMAT, 0);
  /* AM/PM */
  _time_locale->am_pm[0] = getlocaleinfo (time, LOCALE_S1159);
  _time_locale->am_pm[1] = getlocaleinfo (time, LOCALE_S2359);
  /* date_fmt */
  _time_locale->date_fmt = eval_datetimefmt (LOCALE_SLONGDATE, 1);
  --lc_time_ptr;
  *lc_time_ptr++ = ' ';
  eval_datetimefmt (LOCALE_STIMEFORMAT, 0);
  --lc_time_ptr;
  lc_time_ptr = stpcpy (lc_time_ptr, " %Z") + 1;
  /* md */
  {
    wchar_t buf[80];
    GetLocaleInfoW (lcid, LOCALE_IDATE, buf, 80);
    lc_time_ptr = stpcpy (lc_time_ptr, *buf == L'1' ? "dm" : "md") + 1;
  }
  /* ampm_fmt */
  _time_locale->ampm_fmt = eval_datetimefmt (LOCALE_STIMEFORMAT, 1);

  char *tmp = (char *) realloc (new_lc_time_buf, lc_time_ptr - new_lc_time_buf);
  if (!tmp)
    {
      free (new_lc_time_buf);
      return -1;
    }
  if (lc_time_buf)
    free (lc_time_buf);
  lc_time_buf = tmp;
  return 1;
}

/* Called from newlib's setlocale() via __numeric_load_locale() if category
   is LC_NUMERIC.  Returns LC_NUMERIC values fetched from Windows locale data
   in the structure pointed to by _numeric_locale.  This is subsequently
   accessed by functions like nl_langinfo, localeconv, printf, etc. */
extern "C" int
__set_lc_numeric_from_win (const char *name,
			   struct lc_numeric_T *_numeric_locale,
			   wctomb_p f_wctomb, const char *charset)
{
  LCID lcid = __get_lcid_from_locale (name);
  if (!lcid || lcid == (LCID) -1)
    return lcid;

  char *new_lc_numeric_buf = (char *) malloc (48);
  const char *lc_numeric_end = new_lc_numeric_buf + 48;

  if (!new_lc_numeric_buf)
    return -1;
  char *lc_numeric_ptr = new_lc_numeric_buf;
  /* decimal_point */
  _numeric_locale->decimal_point = getlocaleinfo (numeric,
						  LOCALE_SDECIMAL);
  /* thousands_sep */
  _numeric_locale->thousands_sep = getlocaleinfo (numeric,
						  LOCALE_STHOUSAND);
  /* grouping */
  _numeric_locale->grouping = conv_grouping (lcid, LOCALE_SGROUPING,
					     &lc_numeric_ptr);

  char *tmp = (char *) realloc (new_lc_numeric_buf,
  				lc_numeric_ptr - new_lc_numeric_buf);
  if (!tmp)
    {
      free (new_lc_numeric_buf);
      return -1;
    }
  if (lc_numeric_buf)
    free (lc_numeric_buf);
  lc_numeric_buf = tmp;
  return 1;
}

/* Called from newlib's setlocale() via __monetary_load_locale() if category
   is LC_MONETARY.  Returns LC_MONETARY values fetched from Windows locale data
   in the structure pointed to by _monetary_locale.  This is subsequently
   accessed by functions like nl_langinfo, localeconv, printf, etc. */
extern "C" int
__set_lc_monetary_from_win (const char *name,
			    struct lc_monetary_T *_monetary_locale,
			    wctomb_p f_wctomb, const char *charset)
{
  LCID lcid = __get_lcid_from_locale (name);
  if (!lcid || lcid == (LCID) -1)
    return lcid;

  char *new_lc_monetary_buf = (char *) malloc (256);
  const char *lc_monetary_end = new_lc_monetary_buf + 256;

  if (!new_lc_monetary_buf)
    return -1;
  char *lc_monetary_ptr = new_lc_monetary_buf;
  /* int_curr_symbol */
  _monetary_locale->int_curr_symbol = getlocaleinfo (monetary,
						     LOCALE_SINTLSYMBOL);
  /* No spacing char means space. */
  if (!_monetary_locale->int_curr_symbol[3])
    {
      lc_monetary_ptr[-1] = ' ';
      *lc_monetary_ptr++ = '\0';
    }
  /* currency_symbol */
  {
    /* As on Linux:  If the currency_symbol can't be represented in the
       given charset, use int_curr_symbol. */
    wchar_t wbuf[14];
    GetLocaleInfoW (lcid, LOCALE_SCURRENCY, wbuf, 14);
    if (lc_wcstombs (f_wctomb, charset, NULL, wbuf, 0) == (size_t) -1)
      {
	_monetary_locale->currency_symbol = lc_monetary_ptr;
	lc_monetary_ptr = stpncpy (lc_monetary_ptr,
				   _monetary_locale->int_curr_symbol, 3);
	*lc_monetary_ptr++ = '\0';
      }
    else
      _monetary_locale->currency_symbol = getlocaleinfo (monetary,
							 LOCALE_SCURRENCY);
  }
  /* mon_decimal_point */
  _monetary_locale->mon_decimal_point = getlocaleinfo (monetary,
						       LOCALE_SMONDECIMALSEP);
  /* mon_thousands_sep */
  _monetary_locale->mon_thousands_sep = getlocaleinfo (monetary,
						       LOCALE_SMONTHOUSANDSEP);
  /* mon_grouping */
  _monetary_locale->mon_grouping = conv_grouping (lcid, LOCALE_SMONGROUPING,
						  &lc_monetary_ptr);
  /* positive_sign */
  _monetary_locale->positive_sign = getlocaleinfo (monetary,
						   LOCALE_SPOSITIVESIGN);
  /* negative_sign */
  _monetary_locale->negative_sign = getlocaleinfo (monetary,
						   LOCALE_SNEGATIVESIGN);
  /* int_frac_digits */
  *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_IINTLCURRDIGITS);
  _monetary_locale->int_frac_digits = lc_monetary_ptr++;
  /* frac_digits */
  *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_ICURRDIGITS);
  _monetary_locale->frac_digits = lc_monetary_ptr++;
  /* p_cs_precedes */
  *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_IPOSSYMPRECEDES);
  _monetary_locale->p_cs_precedes = lc_monetary_ptr++;
  /* p_sep_by_space */
  *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_IPOSSEPBYSPACE);
  _monetary_locale->p_sep_by_space = lc_monetary_ptr++;
  /* n_cs_precedes */
  *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_INEGSYMPRECEDES);
  _monetary_locale->n_cs_precedes = lc_monetary_ptr++;
  /* n_sep_by_space */
  *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_INEGSEPBYSPACE);
  _monetary_locale->n_sep_by_space = lc_monetary_ptr++;
  /* p_sign_posn */
  *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_IPOSSIGNPOSN);
  _monetary_locale->p_sign_posn = lc_monetary_ptr++;
  /* p_sign_posn */
  *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_INEGSIGNPOSN);
  _monetary_locale->n_sign_posn = lc_monetary_ptr++;

  char *tmp = (char *) realloc (new_lc_monetary_buf,
  				lc_monetary_ptr - new_lc_monetary_buf);
  if (!tmp)
    {
      free (new_lc_monetary_buf);
      return -1;
    }
  if (lc_monetary_buf)
    free (lc_monetary_buf);
  lc_monetary_buf = tmp;
  return 1;
}

static LCID collate_lcid = 0;
static mbtowc_p collate_mbtowc = __ascii_mbtowc;
static char collate_charset[ENCODING_LEN + 1] = "ASCII";

/* Called from newlib's setlocale() if category is LC_COLLATE.  Stores
   LC_COLLATE locale information.  This is subsequently accessed by the
   below functions strcoll, strxfrm, wcscoll, wcsxfrm. */
extern "C" int
__collate_load_locale (const char *name, mbtowc_p f_mbtowc, const char *charset)
{
  LCID lcid = __get_lcid_from_locale (name);
  if (lcid == (LCID) -1)
    return -1;
  collate_lcid = lcid;
  collate_mbtowc = f_mbtowc;
  stpcpy (collate_charset, charset);
  return 0;
}

/* We use the Windows functions for locale-specific string comparison and
   transformation.  The advantage is that we don't need any files with
   collation information. */
extern "C" int
wcscoll (const wchar_t *ws1, const wchar_t *ws2)
{
  int ret;

  if (!collate_lcid)
    return wcscmp (ws1, ws2);
  ret = CompareStringW (collate_lcid, 0, ws1, -1, ws2, -1);
  if (!ret)
    set_errno (EINVAL);
  return ret - CSTR_EQUAL;
}

extern "C" int
strcoll (const char *s1, const char *s2)
{
  size_t n1, n2;
  wchar_t *ws1, *ws2;
  tmp_pathbuf tp;
  int ret;

  if (!collate_lcid)
    return strcmp (s1, s2);
  /* The ANSI version of CompareString uses the default charset of the lcid,
     so we must use the Unicode version. */
  n1 = lc_mbstowcs (collate_mbtowc, collate_charset, NULL, s1, 0) + 1;
  ws1 = (n1 > NT_MAX_PATH ? (wchar_t *) malloc (n1 * sizeof (wchar_t))
			  : tp.w_get ());
  lc_mbstowcs (collate_mbtowc, collate_charset, ws1, s1, n1);
  n2 = lc_mbstowcs (collate_mbtowc, collate_charset, NULL, s2, 0) + 1;
  ws2 = (n2 > NT_MAX_PATH ? (wchar_t *) malloc (n2 * sizeof (wchar_t))
			  : tp.w_get ());
  lc_mbstowcs (collate_mbtowc, collate_charset, ws2, s2, n2);
  ret = CompareStringW (collate_lcid, 0, ws1, -1, ws2, -1);
  if (n1 > NT_MAX_PATH)
    free (ws1);
  if (n2 > NT_MAX_PATH)
    free (ws2);
  if (!ret)
    set_errno (EINVAL);
  return ret - CSTR_EQUAL;
}

extern "C" size_t
wcsxfrm (wchar_t *ws1, const wchar_t *ws2, size_t wsn)
{
  size_t ret;

  if (!collate_lcid)
    return wcslcpy (ws1, ws2, wsn);
  ret = LCMapStringW (collate_lcid, LCMAP_SORTKEY | LCMAP_BYTEREV,
		      ws2, -1, ws1, wsn * sizeof (wchar_t));
  /* LCMapStringW returns byte count including the terminating NUL character,
     wcsxfrm is supposed to return length in wchar_t excluding the NUL.
     Since the array is only single byte NUL-terminated we must make sure
     the result is wchar_t-NUL terminated. */
  if (ret)
    {
      ret = (ret + 1) / sizeof (wchar_t);
      if (ret >= wsn)
	return wsn;
      ws1[ret] = L'\0';
      return ret;
    }
  if (GetLastError () != ERROR_INSUFFICIENT_BUFFER)
    set_errno (EINVAL);
  return wsn;
}

extern "C" size_t
strxfrm (char *s1, const char *s2, size_t sn)
{
  size_t ret;
  size_t n2;
  wchar_t *ws2;
  tmp_pathbuf tp;

  if (!collate_lcid)
    return strlcpy (s1, s2, sn);
  /* The ANSI version of LCMapString uses the default charset of the lcid,
     so we must use the Unicode version. */
  n2 = lc_mbstowcs (collate_mbtowc, collate_charset, NULL, s2, 0) + 1;
  ws2 = (n2 > NT_MAX_PATH ? (wchar_t *) malloc (n2 * sizeof (wchar_t))
			  : tp.w_get ());
  lc_mbstowcs (collate_mbtowc, collate_charset, ws2, s2, n2);
  /* The sort key is a NUL-terminated byte string. */
  ret = LCMapStringW (collate_lcid, LCMAP_SORTKEY, ws2, -1, (PWCHAR) s1, sn);
  if (n2 > NT_MAX_PATH)
    free (ws2);
  if (ret == 0)
    {
      if (GetLastError () != ERROR_INSUFFICIENT_BUFFER)
	set_errno (EINVAL);
      return sn;
    }
  /* LCMapStringW returns byte count including the terminating NUL character.
     strxfrm is supposed to return length excluding the NUL. */
  return ret - 1;
}

/* Fetch default ANSI codepage from locale info and generate a setlocale
   compatible character set code.  Called from newlib's setlocale(), if the
   charset isn't given explicitely in the POSIX compatible locale specifier. */
extern "C" void
__set_charset_from_locale (const char *locale, char *charset)
{
  UINT cp;
  LCID lcid = __get_lcid_from_locale (locale);

  /* "C" locale, or invalid locale? */
  if (lcid == 0 || lcid == (LCID) -1)
    {
      __small_sprintf (charset, "ASCII");
      return;
    }
  if (!GetLocaleInfoW (lcid,
		       LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
		       (PWCHAR) &cp, sizeof cp))
    cp = 0;
  /* codepage to de-facto standard charset transition. */
  switch (cp)
    {
    case 874:
      __small_sprintf (charset, "CP%u", cp);
      break;
    case 932:
      strcpy (charset, "EUCJP");
      break;
    case 936:
      strcpy (charset, "GBK");
      break;
    case 949:
      strcpy (charset, "EUCKR");
      break;
    case 950:
      strcpy (charset, "BIG5");
      break;
    case 1250:
      strcpy (charset, "ISO-8859-2");
      break;
    case 1251:
      strcpy (charset, "ISO-8859-5");
      break;
    case 1252:
      strcpy (charset, "ISO-8859-1");
      break;
    case 1253:
      strcpy (charset, "ISO-8859-7");
      break;
    case 1254:
      strcpy (charset, "ISO-8859-9");
      break;
    case 1255:
      strcpy (charset, "ISO-8859-8");
      break;
    case 1256:
      strcpy (charset, "ISO-8859-6");
      break;
    case 1257:
      strcpy (charset, "ISO-8859-13");
      break;
    case 1258:
    default:
      strcpy (charset, "UTF-8");
      break;
    }
  if (cp >= 1250 && cp <= 1257)
    {
      char *c = strchr (locale, '@');
      if (c && !strcmp (c + 1, "euro"))
      	strcpy (charset, "ISO-8859-15");
    }
}

static char *
check_codepage (char *ret)
{
  if (!wincap.has_always_all_codepages ())
    {
      /* Prior to Windows Vista, many codepages are not installed by
	 default, or can be deinstalled.  The following codepages require
	 that the respective conversion tables are installed into the OS.
	 So we check if they are installed and if not, setlocale should
	 fail. */
      CPINFO cpi;
      UINT cp = 0;
      if (__mbtowc == __sjis_mbtowc)
	cp = 932;
      else if (__mbtowc == __eucjp_mbtowc)
	cp = 20932;
      else if (__mbtowc == __gbk_mbtowc)
	cp = 936;
      else if (__mbtowc == __kr_mbtowc)
	cp = 949;
      else if (__mbtowc == __big5_mbtowc)
	cp = 950;
      if (cp && !GetCPInfo (cp, &cpi)
	  && GetLastError () == ERROR_INVALID_PARAMETER)
	return NULL;
    }
  return ret;
}

static void
internal_setlocale ()
{
  /* Each setlocale from the environment potentially changes the
     multibyte representation of the CWD.  Therefore we have to
     reevaluate the CWD's posix path and store in the new charset.
     Same for the PATH environment variable. */
  /* FIXME: Other buffered paths might be affected as well. */
  /* FIXME: It could be necessary to convert the entire environment,
	    not just PATH. */
  tmp_pathbuf tp;
  char *path = getenv ("PATH");
  wchar_t *w_path = NULL, *w_cwd;

  debug_printf ("Cygwin charset changed from %s to %s",
		cygheap->locale.charset, __locale_charset ());
  /* Fetch PATH and CWD and convert to wchar_t in previous charset. */
  if (path && *path)	/* $PATH can be potentially unset. */
    {
      w_path = tp.w_get ();
      sys_mbstowcs (w_path, 32768, path);
    }
  w_cwd = tp.w_get ();
  cwdstuff::cwd_lock.acquire ();
  sys_mbstowcs (w_cwd, 32768, cygheap->cwd.get_posix ());
  /* Set charset for internal conversion functions. */
  if (*__locale_charset () == 'A'/*SCII*/)
    {
      cygheap->locale.mbtowc = __utf8_mbtowc;
      cygheap->locale.wctomb = __utf8_wctomb;
    }
  else
    {
      cygheap->locale.mbtowc = __mbtowc;
      cygheap->locale.wctomb = __wctomb;
    }
  strcpy (cygheap->locale.charset, __locale_charset ());
  /* Restore CWD and PATH in new charset. */
  cygheap->cwd.reset_posix (w_cwd);
  cwdstuff::cwd_lock.release ();
  if (w_path)
    {
      char *c_path = tp.c_get ();
      sys_wcstombs (c_path, 32768, w_path);
      setenv ("PATH", c_path, 1);
    }
}

/* Called from dll_crt0_1, before fetching the command line from Windows.
   Set the internal charset according to the environment locale settings.
   Check if a required codepage is available, and only switch internal
   charset if so.
   Make sure to reset the application locale to "C" per POSIX. */
void
initial_setlocale ()
{
  char *ret = _setlocale_r (_REENT, LC_CTYPE, "");
  if (ret && check_codepage (ret)
      && strcmp (cygheap->locale.charset, __locale_charset ()) != 0)
    internal_setlocale ();
}

/* Like newlib's setlocale, but additionally check if the charset needs
   OS support and the required codepage is actually installed.  If codepage
   is not available, revert to previous locale and return NULL.  For details
   about codepage availability, see the comment in check_codepage() above. */
extern "C" char *
setlocale (int category, const char *locale)
{
  char old[(LC_MESSAGES + 1) * (ENCODING_LEN + 1/*"/"*/ + 1)];
  if (locale && !wincap.has_always_all_codepages ())
    stpcpy (old, _setlocale_r (_REENT, category, NULL));
  char *ret = _setlocale_r (_REENT, category, locale);
  if (ret && locale && !(ret = check_codepage (ret)))
    _setlocale_r (_REENT, category, old);
  return ret;
}
