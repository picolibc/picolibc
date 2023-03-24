/* nlsfuncs.cc: NLS helper functions

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
#include "collate.h"
#include "lc_msg.h"
#include "lc_era.h"
#include "lc_collelem.h"
#include "lc_def_codesets.h"

#define _LC(x)	&lc_##x##_ptr,lc_##x##_end-lc_##x##_ptr

#define getlocaleinfo(category,type) \
	    __getlocaleinfo(win_locale,(type),_LC(category))
#define getlocaleint(type) \
	    __getlocaleint(win_locale,(type))
#define setlocaleinfo(category,val) \
	    __setlocaleinfo(_LC(category),(val))
#define eval_datetimefmt(type,flags) \
	    __eval_datetimefmt(win_locale,(type),(flags),&lc_time_ptr,\
			       lc_time_end-lc_time_ptr)
#define charfromwchar(category,in) \
	    __charfromwchar (_##category##_locale->in,_LC(category),f_wctomb)

#define has_modifier(x)	((x)[0] && !strcmp (modifier, (x)))

/* ResolveLocaleName does not what we want.  It converts anything which
   vaguely resembles a locale into some other locale it supports.  Bad
   examples are: "en-XY" gets converted to "en-US", and worse, "ff-BF" gets
   converted to "ff-Latn-SN", even though "ff-Adlm-BF" exists!  Useless.
   To check if a locale is supported, we have to enumerate all valid
   Windows locales, and return the match, even if the locale in Windows
   requires a script. */
struct res_loc_t {
  const wchar_t *search_iso639;
  const wchar_t *search_iso3166;
  wchar_t *resolved_locale;
  int res_len;
};

static BOOL
resolve_locale_proc (LPWSTR win_locale, DWORD info, LPARAM param)
{
  res_loc_t *loc = (res_loc_t *) param;
  wchar_t *iso639, *iso639_end;
  wchar_t *iso3166;

  iso639 = win_locale;
  iso639_end = wcschr (iso639, L'-');
  if (!iso639_end)
    return TRUE;
  if (wcsncmp (loc->search_iso639, iso639, iso639_end - iso639) != 0)
    return TRUE;
  iso3166 = ++iso639_end;
  /* Territory is all upper case */
  while (!iswupper (iso3166[0]) || !iswupper (iso3166[1]))
    {
      iso3166 = wcschr (iso3166, L'-');
      if (!iso3166)
	return TRUE;
      ++iso3166;
    }
  if (wcsncmp (loc->search_iso3166, iso3166, wcslen (loc->search_iso3166)))
    return TRUE;
  wcsncat (loc->resolved_locale, win_locale, loc->res_len - 1);
  return FALSE;
}

static int
resolve_locale_name (const wchar_t *search, wchar_t *result, int rlen)
{
  res_loc_t loc;

  loc.search_iso639 = search;
  loc.search_iso3166 = wcschr (search, L'-') + 1;
  loc.resolved_locale = result;
  loc.res_len = rlen;
  result[0] = L'\0';
  EnumSystemLocalesEx (resolve_locale_proc,
		       LOCALE_WINDOWS | LOCALE_SUPPLEMENTAL,
		       (LPARAM) &loc, NULL);
  return wcslen (result);
}

/* Fetch Windows RFC 5646 locale from POSIX locale specifier.
   Return values:

     -1: Invalid locale
      0: C or POSIX
      1: valid locale
*/
static int
__get_rfc5646_from_locale (const char *name, wchar_t *win_locale)
{
  wchar_t wlocale[ENCODING_LEN + 1] = { 0 };
  wchar_t locale[ENCODING_LEN + 1];
  wchar_t *c;

  win_locale[0] = L'\0';
  mbstowcs (locale, name, ENCODING_LEN + 1);
  /* Remember modifier for later use. */
  const char *modifier = strchr (name, '@') ? : "";
  /* Drop charset and modifier */
  c = wcschr (locale, L'.');
  if (!c)
    c = wcschr (locale, L'@');
  if (c)
    *c = L'\0';
  /* "POSIX" already converted to "C" in loadlocale. */
  if (!wcscmp (locale, L"C"))
    return 0;
  c = wcschr (locale, '_');
  if (!c)
    {
      set_errno (ENOENT);
      return -1;
    }

  /* Convert to RFC 5646 syntax. */
  *c = '-';
  /* Override a few locales with a different default script as used
     on Linux.  Linux also supports no_NO which is equivalent to nb_NO,
     but Windows can resolve that nicely.  Also, "tzm" and "zgh" are
     subsumed under "ber" on Linux. */
  struct {
    const wchar_t *loc;
    const wchar_t *wloc;
  } override_locale[] = {
    { L"ber-DZ" , L"tzm-Latn-DZ" },
    { L"ber-MA" , L"zgh-Tfng-MA" },
    { L"mn-CN" , L"mn-Mong-CN"   },
    { L"mn-MN" , L"mn-Mong-MN"   },
    { L"pa-PK" , L"pa-Arab-PK"   },
    { L"sd-IN" , L"sd-Deva-IN"   },
    { L"sr-BA" , L"sr-Cyrl-BA"   },
    { L"sr-ME" , L"sr-Cyrl-ME"   },
    { L"sr-RS" , L"sr-Cyrl-RS"   },
    { L"sr-XK" , L"sr-Cyrl-XK"   },
    { L"tzm-MA", L"tzm-Tfng-MA"  },
    { NULL    , NULL	     }
  };

  for (int i = 0; override_locale[i].loc
		  && override_locale[i].loc[0] <= locale[0]; ++i)
    {
      if (!wcscmp (locale, override_locale[i].loc))
	{
	  wcscpy (wlocale, override_locale[i].wloc);
	  break;
	}
    }
  /* If resolve_locale_name returns with error, or if it returns a
     locale other than the input locale, we don't support this locale. */
  if (!wlocale[0]
      && !resolve_locale_name (locale, wlocale, ENCODING_LEN + 1))
    {
      set_errno (ENOENT);
      return -1;
    }

  /* Check for modifiers changing the script */
  const wchar_t *iso15924_script[] = { L"Latn-", L"Cyrl-", L"Deva-", L"Adlm-" };
  int idx = -1;

  if (modifier[0])
    {
      if (!strcmp (++modifier, "latin"))
	idx = 0;
      else if (!strcmp (modifier, "cyrillic"))
	idx = 1;
      else if (!strcmp (modifier, "devanagari"))
	idx = 2;
      else if (!strcmp (modifier, "adlam"))
	idx = 3;
    }
  if (idx >= 0)
    {
      wchar_t *iso3166 = wcschr (wlocale, L'-') + 1;
      wchar_t *wlp;

      /* Copy iso639 language part including dash */
      wlp = wcpncpy (win_locale, wlocale, iso3166 - wlocale);
      /* Concat new iso15924 script */
      wlp = wcpcpy (wlp, iso15924_script[idx]);
      /* Concat iso3166 territory.  Skip script, if already in the locale */
      wchar_t *skip_script = wcschr (iso3166, L'-');
      if (skip_script)
	iso3166 = skip_script + 1;
       wcpcpy (wlp, iso3166);
    }
  else
    wcpcpy (win_locale, wlocale);
  return 1;
}

/* Never returns -1.  Just skips invalid chars instead.  Only if return_invalid
   is set, s==NULL returns -1 since then it's used to recognize invalid strings
   in the used charset. */
static size_t
lc_wcstombs (wctomb_p f_wctomb, char *s, const wchar_t *pwcs, size_t n,
	     bool return_invalid = false)
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
	  bytes = f_wctomb (_REENT, buf, *pwcs++, &state);
	  if (bytes != (size_t) -1)
	    num_bytes += bytes;
	  else if (return_invalid)
	    return (size_t) -1;
	}
      return num_bytes;
    }
  while (n > 0)
    {
      bytes = f_wctomb (_REENT, buf, *pwcs, &state);
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
lc_mbstowcs (mbtowc_p f_mbtowc, wchar_t *pwcs, const char *s, size_t n)
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
      bytes = f_mbtowc (_REENT, pwcs, t, 6 /* fake, always enough */, &state);
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

static int
locale_cmp (const void *a, const void *b)
{
  char **la = (char **) a;
  char **lb = (char **) b;
  return strcmp (*la, *lb);
}

/* Helper function to workaround reallocs which move blocks even if they shrink.
   Cygwin's realloc is not doing this, but tcsh's, for instance.  All lc_foo
   structures consist entirely of pointers so they are practically pointer
   arrays.  What we do here is just treat the lc_foo pointers as char ** and
   rebase all char * pointers within, up to the given size of the structure. */
static void
rebase_locale_buf (const void *ptrv, const void *ptrvend, const char *newbase,
		   const char *oldbase, const char *oldend)
{
  const char **ptrsend = (const char **) ptrvend;
  for (const char **ptrs = (const char **) ptrv; ptrs < ptrsend; ++ptrs)
    if (*ptrs >= oldbase && *ptrs < oldend)
      *ptrs += newbase - oldbase;
}

static wchar_t *
__getlocaleinfo (wchar_t *loc, LCTYPE type, char **ptr, size_t size)
{
  size_t num;
  wchar_t *ret;

  if ((uintptr_t) *ptr % 1)
    ++*ptr;
  ret = (wchar_t *) *ptr;
  num = GetLocaleInfoEx (loc, type, ret, size / sizeof (wchar_t));
  *ptr = (char *) (ret + num);
  return ret;
}

static wchar_t *
__setlocaleinfo (char **ptr, size_t size, wchar_t val)
{
  wchar_t *ret;

  if ((uintptr_t) *ptr % 1)
    ++*ptr;
  ret = (wchar_t *) *ptr;
  ret[0] = val;
  ret[1] = L'\0';
  *ptr = (char *) (ret + 2);
  return ret;
}

static char *
__charfromwchar (const wchar_t *in, char **ptr, size_t size, wctomb_p f_wctomb)
{
  size_t num;
  char *ret;

  num = lc_wcstombs (f_wctomb, ret = *ptr, in, size);
  *ptr += num + 1;
  return ret;
}

static UINT
__getlocaleint (wchar_t *loc, LCTYPE type)
{
  UINT val;
  return GetLocaleInfoEx (loc, type | LOCALE_RETURN_NUMBER, (PWCHAR) &val,
			 sizeof val) ? val : 0;
}

enum dt_flags {
  DT_DEFAULT	= 0x00,
  DT_AMPM	= 0x01,	/* Enforce 12 hour time format. */
  DT_ABBREV	= 0x02,	/* Enforce abbreviated month and day names. */
};

static wchar_t *
__eval_datetimefmt (wchar_t *loc, LCTYPE type, dt_flags flags, char **ptr,
		    size_t size)
{
  wchar_t buf[80];
  wchar_t fc;
  size_t idx;
  const wchar_t *day_str = L"edaA";
  const wchar_t *mon_str = L"mmbB";
  const wchar_t *year_str = L"yyyY";
  const wchar_t *hour12_str = L"lI";
  const wchar_t *hour24_str = L"kH";
  const wchar_t *t_str;

  if ((uintptr_t) *ptr % 1)
    ++*ptr;
  wchar_t *ret = (wchar_t *) *ptr;
  wchar_t *p = (wchar_t *) *ptr;
  GetLocaleInfoEx (loc, type, buf, 80);
  for (wchar_t *fmt = buf; *fmt; ++fmt)
    switch (fc = *fmt)
      {
      case L'\'':
	if (fmt[1] == L'\'')
	  *p++ = L'\'';
	else
	  while (fmt[1] && *++fmt != L'\'')
	    *p++ = *fmt;
	break;
      case L'd':
      case L'M':
      case L'y':
	t_str = (fc == L'd' ? day_str : fc == L'M' ? mon_str : year_str);
	for (idx = 0; fmt[1] == fc; ++idx, ++fmt);
	if (idx > 3)
	  idx = 3;
	if ((flags & DT_ABBREV) && fc != L'y' && idx == 3)
	  idx = 2;
	*p++ = L'%';
	*p++ = t_str[idx];
	break;
      case L'g':
	/* TODO */
	break;
      case L'h':
      case L'H':
	t_str = (fc == L'h' || (flags & DT_AMPM) ? hour12_str : hour24_str);
	idx = 0;
	if (fmt[1] == fc)
	  {
	    ++fmt;
	    idx = 1;
	  }
	*p++ = L'%';
	*p++ = t_str[idx];
	break;
      case L'm':
      case L's':
      case L't':
	if (fmt[1] == fc)
	  ++fmt;
	*p++ = L'%';
	*p++ = (fc == L'm' ? L'M' : fc == L's' ? L'S' : L'p');
	break;
      case L'\t':
      case L'\n':
      case L'%':
	*p++ = L'%';
	*p++ = fc;
	break;
      default:
	*p++ = *fmt;
	break;
      }
  *p++ = L'\0';
  *ptr = (char *) p;
  return ret;
}

/* Convert Windows grouping format into POSIX grouping format. */
static char *
conv_grouping (wchar_t *loc, LCTYPE type, char **lc_ptr)
{
  wchar_t buf[10]; /* Per MSDN max size of LOCALE_SGROUPING element incl. NUL */
  bool repeat = false;
  char *ptr = *lc_ptr;
  char *ret = ptr;

  GetLocaleInfoEx (loc, type, buf, 10);
  /* Convert Windows grouping format into POSIX grouping format. Note that
     only ASCII chars are used in the grouping format. */
  for (wchar_t *c = buf; *c; ++c)
    {
      if (*c < L'0' || *c > L'9')
	continue;
      char val = *c - L'0';
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
__set_lc_time_from_win (const char *name,
			const struct lc_time_T *_C_time_locale,
			struct lc_time_T *_time_locale,
			char **lc_time_buf, wctomb_p f_wctomb,
			const char *charset)
{
  wchar_t win_locale[ENCODING_LEN + 1];
  int ret = __get_rfc5646_from_locale (name, win_locale);
  if (ret < 0)
    return ret;
  if (!ret && !strcmp (charset, "ASCII"))
    return 0;

# define MAX_TIME_BUFFER_SIZE	4096

  char *new_lc_time_buf = (char *) malloc (MAX_TIME_BUFFER_SIZE);
  const char *lc_time_end = new_lc_time_buf + MAX_TIME_BUFFER_SIZE;

  if (!new_lc_time_buf)
    return -1;
  char *lc_time_ptr = new_lc_time_buf;

  /* C.foo is just a copy of "C" with fixed charset. */
  if (!ret)
    memcpy (_time_locale, _C_time_locale, sizeof (struct lc_time_T));
  /* codeset */
  _time_locale->codeset = lc_time_ptr;
  lc_time_ptr = stpcpy (lc_time_ptr, charset) + 1;

  if (ret)
    {
      char locale[ENCODING_LEN + 1];
      strcpy (locale, name);
      /* Removes the charset from the locale and attach the modifier to the
	 language_TERRITORY part. */
      char *c = strchr (locale, '.');
      if (c)
	{
	  *c = '\0';
	  char *c2 = strchr (c + 1, '@');
	  /* Ignore @cjknarrow modifier since it's a very personal thing between
	     Cygwin and newlib... */
	  if (c2 && strcmp (c2, "@cjknarrow"))
	    memmove (c, c2, strlen (c2) + 1);
	}
      /* Now search in the alphabetically order lc_era array for the
	 locale. */
      lc_era_t locale_key = { locale, NULL, NULL, NULL, NULL, NULL ,
				      NULL, NULL, NULL, NULL, NULL };
      lc_era_t *era = (lc_era_t *) bsearch ((void *) &locale_key,
					    (void *) lc_era,
					    sizeof lc_era / sizeof *lc_era,
					    sizeof *lc_era, locale_cmp);

      /* mon */
      /* Windows has a bug in "ja-JP" and "ko-KR" (but not in "ko-KP").
         In these locales, strings returned for LOCALE_SABBREVMONTHNAME*
	 are missing the suffix representing a month.

	 A Japanese article describing the problem was
	 https://msdn.microsoft.com/ja-jp/library/cc422084.aspx, which is
	 only available via
	 https://web.archive.org/web/20110922195821/https://msdn.microsoft.com/ja-jp/library/cc422084.aspx
	 these days.  Testing indicates that this problem is still present
	 in Windows 11.

	 The workaround is to use LOCALE_SMONTHNAME* in these locales,
	 even for the abbreviated month name. */
      const LCTYPE mon_base = !wcscmp (win_locale, L"ja-JP")
			      || !wcscmp (win_locale, L"ko-KR")
			      ? LOCALE_SMONTHNAME1 : LOCALE_SABBREVMONTHNAME1;
      for (int i = 0; i < 12; ++i)
	{
	  _time_locale->wmon[i] = getlocaleinfo (time, mon_base + i);
	  _time_locale->mon[i] = charfromwchar (time, wmon[i]);
	}
      /* month and alt_month */
      for (int i = 0; i < 12; ++i)
	{
	  _time_locale->wmonth[i] = getlocaleinfo (time,
						   LOCALE_SMONTHNAME1 + i);
	  _time_locale->month[i] = _time_locale->alt_month[i]
				 = charfromwchar (time, wmonth[i]);
	}
      /* wday */
      _time_locale->wwday[0] = getlocaleinfo (time, LOCALE_SABBREVDAYNAME7);
      _time_locale->wday[0] = charfromwchar (time, wwday[0]);
      for (int i = 0; i < 6; ++i)
	{
	  _time_locale->wwday[i + 1] = getlocaleinfo (time,
						      LOCALE_SABBREVDAYNAME1 + i);
	  _time_locale->wday[i + 1] = charfromwchar (time, wwday[i + 1]);
	}
      /* weekday */
      _time_locale->wweekday[0] = getlocaleinfo (time, LOCALE_SDAYNAME7);
      _time_locale->weekday[0] = charfromwchar (time, wweekday[0]);
      for (int i = 0; i < 6; ++i)
	{
	  _time_locale->wweekday[i + 1] = getlocaleinfo (time,
							 LOCALE_SDAYNAME1 + i);
	  _time_locale->weekday[i + 1] = charfromwchar (time, wweekday[i + 1]);
	}
      size_t len;
      /* X_fmt */
      if (era && *era->t_fmt)
	{
	  _time_locale->wX_fmt = (const wchar_t *) lc_time_ptr;
	  lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->wX_fmt,
					  era->t_fmt) + 1);
	}
      else
	_time_locale->wX_fmt = eval_datetimefmt (LOCALE_STIMEFORMAT, DT_DEFAULT);
      _time_locale->X_fmt = charfromwchar (time, wX_fmt);
      /* x_fmt */
      if (era && *era->d_fmt)
	{
	  _time_locale->wx_fmt = (const wchar_t *) lc_time_ptr;
	  lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->wx_fmt,
					  era->d_fmt) + 1);
	}
      else
	_time_locale->wx_fmt = eval_datetimefmt (LOCALE_SSHORTDATE, DT_DEFAULT);
      _time_locale->x_fmt = charfromwchar (time, wx_fmt);
      /* c_fmt */
      if (era && *era->d_t_fmt)
	{
	  _time_locale->wc_fmt = (const wchar_t *) lc_time_ptr;
	  lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->wc_fmt,
					  era->d_t_fmt) + 1);
	}
      else
	{
	  _time_locale->wc_fmt = eval_datetimefmt (LOCALE_SLONGDATE, DT_ABBREV);
	  ((wchar_t *) lc_time_ptr)[-1] = L' ';
	  eval_datetimefmt (LOCALE_STIMEFORMAT, DT_DEFAULT);
	}
      _time_locale->c_fmt = charfromwchar (time, wc_fmt);
      /* AM/PM */
      _time_locale->wam_pm[0] = getlocaleinfo (time, LOCALE_S1159);
      _time_locale->wam_pm[1] = getlocaleinfo (time, LOCALE_S2359);
      _time_locale->am_pm[0] = charfromwchar (time, wam_pm[0]);
      _time_locale->am_pm[1] = charfromwchar (time, wam_pm[1]);
      /* date_fmt */
      if (era && *era->date_fmt)
	{
	  _time_locale->wdate_fmt = (const wchar_t *) lc_time_ptr;
	  lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->wdate_fmt,
					  era->date_fmt) + 1);
	}
      else
	_time_locale->wdate_fmt = _time_locale->wc_fmt;
      _time_locale->date_fmt = charfromwchar (time, wdate_fmt);
      /* md */
      {
	wchar_t buf[80];
	GetLocaleInfoEx (win_locale, LOCALE_IDATE, buf, 80);
	_time_locale->md_order = (const char *) lc_time_ptr;
	lc_time_ptr = stpcpy (lc_time_ptr, *buf == L'1' ? "dm" : "md") + 1;
      }
      /* ampm_fmt */
      if (era)
	{
	  _time_locale->wampm_fmt = (const wchar_t *) lc_time_ptr;
	  lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->wampm_fmt,
					  era->t_fmt_ampm) + 1);
	}
      else
	_time_locale->wampm_fmt = eval_datetimefmt (LOCALE_STIMEFORMAT, DT_AMPM);
      _time_locale->ampm_fmt = charfromwchar (time, wampm_fmt);

      if (era)
	{
	  /* Evaluate string length in target charset.  Characters invalid in the
	     target charset are simply ignored, as on Linux. */
	  len = 0;
	  len += lc_wcstombs (f_wctomb, NULL, era->era, 0) + 1;
	  len += lc_wcstombs (f_wctomb, NULL, era->era_d_fmt, 0) + 1;
	  len += lc_wcstombs (f_wctomb, NULL, era->era_d_t_fmt, 0) + 1;
	  len += lc_wcstombs (f_wctomb, NULL, era->era_t_fmt, 0) + 1;
	  len += lc_wcstombs (f_wctomb, NULL, era->alt_digits, 0) + 1;
	  len += (wcslen (era->era) + 1) * sizeof (wchar_t);
	  len += (wcslen (era->era_d_fmt) + 1) * sizeof (wchar_t);
	  len += (wcslen (era->era_d_t_fmt) + 1) * sizeof (wchar_t);
	  len += (wcslen (era->era_t_fmt) + 1) * sizeof (wchar_t);
	  len += (wcslen (era->alt_digits) + 1) * sizeof (wchar_t);

	  /* Make sure data fits into the buffer */
	  if (lc_time_ptr + len > lc_time_end)
	    {
	      len = lc_time_ptr + len - new_lc_time_buf;
	      char *tmp = (char *) realloc (new_lc_time_buf, len);
	      if (!tmp)
		era = NULL;
	      else
		{
		  if (tmp != new_lc_time_buf)
		    rebase_locale_buf (_time_locale, _time_locale + 1, tmp,
				       new_lc_time_buf, lc_time_ptr);
		  lc_time_ptr = tmp + (lc_time_ptr - new_lc_time_buf);
		  new_lc_time_buf = tmp;
		  lc_time_end = new_lc_time_buf + len;
		}
	    }
	  /* Copy over */
	  if (era)
	    {
	      /* era */
	      _time_locale->wera = (const wchar_t *) lc_time_ptr;
	      lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->wera,
					      era->era) + 1);
	      _time_locale->era = charfromwchar (time, wera);
	      /* era_d_fmt */
	      _time_locale->wera_d_fmt = (const wchar_t *) lc_time_ptr;
	      lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->wera_d_fmt,
					      era->era_d_fmt) + 1);
	      _time_locale->era_d_fmt = charfromwchar (time, wera_d_fmt);
	      /* era_d_t_fmt */
	      _time_locale->wera_d_t_fmt = (const wchar_t *) lc_time_ptr;
	      lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->wera_d_t_fmt,
					      era->era_d_t_fmt) + 1);
	      _time_locale->era_d_t_fmt = charfromwchar (time, wera_d_t_fmt);
	      /* era_t_fmt */
	      _time_locale->wera_t_fmt = (const wchar_t *) lc_time_ptr;
	      lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->wera_t_fmt,
					      era->era_t_fmt) + 1);
	      _time_locale->era_t_fmt = charfromwchar (time, wera_t_fmt);
	      /* alt_digits */
	      _time_locale->walt_digits = (const wchar_t *) lc_time_ptr;
	      lc_time_ptr = (char *) (wcpcpy ((wchar_t *) _time_locale->walt_digits,
					      era->alt_digits) + 1);
	      _time_locale->alt_digits = charfromwchar (time, walt_digits);
	    }
	}
      if (!era)
	{
	  _time_locale->wera =
	  _time_locale->wera_d_fmt =
	  _time_locale->wera_d_t_fmt =
	  _time_locale->wera_t_fmt =
	  _time_locale->walt_digits = (const wchar_t *) lc_time_ptr;
	  _time_locale->era =
	  _time_locale->era_d_fmt =
	  _time_locale->era_d_t_fmt =
	  _time_locale->era_t_fmt =
	  _time_locale->alt_digits = (const char *) lc_time_ptr;
	  /* Twice, to make sure wide char strings are correctly terminated. */
	  *lc_time_ptr++ = '\0';
	  *lc_time_ptr++ = '\0';
	}
    }

  char *tmp = (char *) realloc (new_lc_time_buf, lc_time_ptr - new_lc_time_buf);
  if (!tmp)
    {
      free (new_lc_time_buf);
      return -1;
    }
  if (tmp != new_lc_time_buf)
    rebase_locale_buf (_time_locale, _time_locale + 1, tmp,
		       new_lc_time_buf, lc_time_ptr);
  *lc_time_buf = tmp;
  return 1;
}

/* Called from newlib's setlocale() via __ctype_load_locale() if category
   is LC_CTYPE.  Returns LC_CTYPE values fetched from Windows locale data
   in the structure pointed to by _ctype_locale.  This is subsequently
   accessed by functions like nl_langinfo, localeconv, printf, etc. */
extern "C" int
__set_lc_ctype_from_win (const char *name,
			 const struct lc_ctype_T *_C_ctype_locale,
			 struct lc_ctype_T *_ctype_locale,
			 char **lc_ctype_buf, wctomb_p f_wctomb,
			 const char *charset, int mb_cur_max)
{
  wchar_t win_locale[ENCODING_LEN + 1];
  int ret = __get_rfc5646_from_locale (name, win_locale);
  if (ret < 0)
    return ret;
  if (!ret && !strcmp (charset, "ASCII"))
    return 0;

# define MAX_CTYPE_BUFFER_SIZE	256

  char *new_lc_ctype_buf = (char *) malloc (MAX_CTYPE_BUFFER_SIZE);

  if (!new_lc_ctype_buf)
    return -1;
  char *lc_ctype_ptr = new_lc_ctype_buf;
  /* C.foo is just a copy of "C" with fixed charset. */
  if (!ret)
    memcpy (_ctype_locale, _C_ctype_locale, sizeof (struct lc_ctype_T));
  /* codeset */
  _ctype_locale->codeset = lc_ctype_ptr;
  lc_ctype_ptr = stpcpy (lc_ctype_ptr, charset) + 1;
  /* mb_cur_max */
  _ctype_locale->mb_cur_max = lc_ctype_ptr;
  *lc_ctype_ptr++ = mb_cur_max;
  *lc_ctype_ptr++ = '\0';
  if (ret)
    {
      /* outdigits and woutdigits */
      wchar_t digits[11];
      GetLocaleInfoEx (win_locale, LOCALE_SNATIVEDIGITS, digits, 11);
      for (int i = 0; i <= 9; ++i)
	{
	  mbstate_t state;

	  /* Make sure the wchar_t's are always 2 byte aligned. */
	  if ((uintptr_t) lc_ctype_ptr % 2)
	    ++lc_ctype_ptr;
	  wchar_t *woutdig = (wchar_t *) lc_ctype_ptr;
	  _ctype_locale->woutdigits[i] = (const wchar_t *) woutdig;
	  *woutdig++ = digits[i];
	  *woutdig++ = L'\0';
	  lc_ctype_ptr = (char *) woutdig;
	  _ctype_locale->outdigits[i] = lc_ctype_ptr;
	  memset (&state, 0, sizeof state);
	  lc_ctype_ptr += f_wctomb (_REENT, lc_ctype_ptr, digits[i], &state);
	  *lc_ctype_ptr++ = '\0';
	}
    }

  char *tmp = (char *) realloc (new_lc_ctype_buf,
				lc_ctype_ptr - new_lc_ctype_buf);
  if (!tmp)
    {
      free (new_lc_ctype_buf);
      return -1;
    }
  if (tmp != new_lc_ctype_buf)
    rebase_locale_buf (_ctype_locale, _ctype_locale + 1, tmp,
		       new_lc_ctype_buf, lc_ctype_ptr);
  *lc_ctype_buf = tmp;
  return 1;
}

/* Called from newlib's setlocale() via __numeric_load_locale() if category
   is LC_NUMERIC.  Returns LC_NUMERIC values fetched from Windows locale data
   in the structure pointed to by _numeric_locale.  This is subsequently
   accessed by functions like nl_langinfo, localeconv, printf, etc. */
extern "C" int
__set_lc_numeric_from_win (const char *name,
			   const struct lc_numeric_T *_C_numeric_locale,
			   struct lc_numeric_T *_numeric_locale,
			   char **lc_numeric_buf, wctomb_p f_wctomb,
			   const char *charset)
{
  wchar_t win_locale[ENCODING_LEN + 1];
  int ret = __get_rfc5646_from_locale (name, win_locale);
  if (ret < 0)
    return ret;
  if (!ret && !strcmp (charset, "ASCII"))
    return 0;

# define MAX_NUMERIC_BUFFER_SIZE	256

  char *new_lc_numeric_buf = (char *) malloc (MAX_NUMERIC_BUFFER_SIZE);
  const char *lc_numeric_end = new_lc_numeric_buf + MAX_NUMERIC_BUFFER_SIZE;

  if (!new_lc_numeric_buf)
    return -1;
  char *lc_numeric_ptr = new_lc_numeric_buf;
  /* C.foo is just a copy of "C" with fixed charset. */
  if (!ret)
    memcpy (_numeric_locale, _C_numeric_locale, sizeof (struct lc_numeric_T));
  else
    {
      /* decimal_point and thousands_sep */
      /* fa_IR.  Windows decimal_point is slash, correct is dot */
      if (!wcscmp (win_locale, L"fa-IR"))
	{
	  _numeric_locale->wdecimal_point = setlocaleinfo (numeric, L'.');
	  _numeric_locale->wthousands_sep = setlocaleinfo (numeric, L',');
	}
      /* ps_AF.  Windows decimal_point is dot, thousands_sep is comma,
		 correct are arabic separators. */
      else if (!wcscmp (win_locale, L"ps-AF"))
	{
	  _numeric_locale->wdecimal_point = setlocaleinfo (numeric, 0x066b);
	  _numeric_locale->wthousands_sep = setlocaleinfo (numeric, 0x066c);
	}
      else
	{
	  _numeric_locale->wdecimal_point = getlocaleinfo (numeric,
							   LOCALE_SDECIMAL);
	  _numeric_locale->wthousands_sep = getlocaleinfo (numeric,
							   LOCALE_STHOUSAND);
	}
      _numeric_locale->decimal_point = charfromwchar (numeric, wdecimal_point);
      _numeric_locale->thousands_sep = charfromwchar (numeric, wthousands_sep);
      /* grouping */
      _numeric_locale->grouping = conv_grouping (win_locale, LOCALE_SGROUPING,
						 &lc_numeric_ptr);
    }
  /* codeset */
  _numeric_locale->codeset = lc_numeric_ptr;
  lc_numeric_ptr = stpcpy (lc_numeric_ptr, charset) + 1;

  char *tmp = (char *) realloc (new_lc_numeric_buf,
				lc_numeric_ptr - new_lc_numeric_buf);
  if (!tmp)
    {
      free (new_lc_numeric_buf);
      return -1;
    }
  if (tmp != new_lc_numeric_buf)
    rebase_locale_buf (_numeric_locale, _numeric_locale + 1, tmp,
		       new_lc_numeric_buf, lc_numeric_ptr);
  *lc_numeric_buf = tmp;
  return 1;
}

/* Called from newlib's setlocale() via __monetary_load_locale() if category
   is LC_MONETARY.  Returns LC_MONETARY values fetched from Windows locale data
   in the structure pointed to by _monetary_locale.  This is subsequently
   accessed by functions like nl_langinfo, localeconv, printf, etc. */
extern "C" int
__set_lc_monetary_from_win (const char *name,
			    const struct lc_monetary_T *_C_monetary_locale,
			    struct lc_monetary_T *_monetary_locale,
			    char **lc_monetary_buf, wctomb_p f_wctomb,
			    const char *charset)
{
  wchar_t win_locale[ENCODING_LEN + 1];
  int ret = __get_rfc5646_from_locale (name, win_locale);
  if (ret < 0)
    return ret;
  if (!ret && !strcmp (charset, "ASCII"))
    return 0;

# define MAX_MONETARY_BUFFER_SIZE	512

  char *new_lc_monetary_buf = (char *) malloc (MAX_MONETARY_BUFFER_SIZE);
  const char *lc_monetary_end = new_lc_monetary_buf + MAX_MONETARY_BUFFER_SIZE;

  if (!new_lc_monetary_buf)
    return -1;
  char *lc_monetary_ptr = new_lc_monetary_buf;
  /* C.foo is just a copy of "C" with fixed charset. */
  if (!ret)
    memcpy (_monetary_locale, _C_monetary_locale, sizeof (struct lc_monetary_T));
  else
    {
      /* int_curr_symbol */
      _monetary_locale->wint_curr_symbol = getlocaleinfo (monetary,
							  LOCALE_SINTLSYMBOL);
      /* No spacing char means space. */
      if (!_monetary_locale->wint_curr_symbol[3])
	{
	  wchar_t *wc = (wchar_t *) _monetary_locale->wint_curr_symbol + 3;
	  *wc++ = L' ';
	  *wc++ = L'\0';
	  lc_monetary_ptr = (char *) wc;
	}
      _monetary_locale->int_curr_symbol = charfromwchar (monetary,
							 wint_curr_symbol);
      /* currency_symbol */
      _monetary_locale->wcurrency_symbol = getlocaleinfo (monetary,
							  LOCALE_SCURRENCY);
      /* As on Linux:  If the currency_symbol can't be represented in the
	 given charset, use int_curr_symbol. */
      if (lc_wcstombs (f_wctomb, NULL, _monetary_locale->wcurrency_symbol,
		       0, true) == (size_t) -1)
	_monetary_locale->currency_symbol = _monetary_locale->int_curr_symbol;
      else
	_monetary_locale->currency_symbol = charfromwchar (monetary,
							   wcurrency_symbol);
      /* mon_decimal_point and mon_thousands_sep */
      /* fa_IR or ps_AF.  Windows mon_decimal_point is slash and comma,
			  mon_thousands_sep is comma and dot, correct
			  are arabic separators. */
      if (!wcscmp (win_locale, L"fa-IR")
	  || !wcscmp (win_locale, L"ps-AF"))
	{
	  _monetary_locale->wmon_decimal_point = setlocaleinfo (monetary,
								0x066b);
	  _monetary_locale->wmon_thousands_sep = setlocaleinfo (monetary,
								0x066c);
	}
      else
	{
	  _monetary_locale->wmon_decimal_point = getlocaleinfo (monetary,
							LOCALE_SMONDECIMALSEP);
	  _monetary_locale->wmon_thousands_sep = getlocaleinfo (monetary,
							LOCALE_SMONTHOUSANDSEP);
	}
      _monetary_locale->mon_decimal_point = charfromwchar (monetary,
							   wmon_decimal_point);
      _monetary_locale->mon_thousands_sep = charfromwchar (monetary,
							   wmon_thousands_sep);
      /* mon_grouping */
      _monetary_locale->mon_grouping = conv_grouping (win_locale,
						      LOCALE_SMONGROUPING,
						      &lc_monetary_ptr);
      /* positive_sign */
      _monetary_locale->wpositive_sign = getlocaleinfo (monetary,
							LOCALE_SPOSITIVESIGN);
      _monetary_locale->positive_sign = charfromwchar (monetary, wpositive_sign);
      /* negative_sign */
      _monetary_locale->wnegative_sign = getlocaleinfo (monetary,
							LOCALE_SNEGATIVESIGN);
      _monetary_locale->negative_sign = charfromwchar (monetary, wnegative_sign);
      /* int_frac_digits */
      *lc_monetary_ptr = (char) getlocaleint (LOCALE_IINTLCURRDIGITS);
      _monetary_locale->int_frac_digits = lc_monetary_ptr++;
      /* frac_digits */
      *lc_monetary_ptr = (char) getlocaleint (LOCALE_ICURRDIGITS);
      _monetary_locale->frac_digits = lc_monetary_ptr++;
      /* p_cs_precedes and int_p_cs_precedes */
      *lc_monetary_ptr = (char) getlocaleint (LOCALE_IPOSSYMPRECEDES);
      _monetary_locale->p_cs_precedes
	    = _monetary_locale->int_p_cs_precedes = lc_monetary_ptr++;
      /* p_sep_by_space and int_p_sep_by_space */
      *lc_monetary_ptr = (char) getlocaleint (LOCALE_IPOSSEPBYSPACE);
      _monetary_locale->p_sep_by_space
	    = _monetary_locale->int_p_sep_by_space = lc_monetary_ptr++;
      /* n_cs_precedes and int_n_cs_precedes */
      *lc_monetary_ptr = (char) getlocaleint (LOCALE_INEGSYMPRECEDES);
      _monetary_locale->n_cs_precedes
	    = _monetary_locale->int_n_cs_precedes = lc_monetary_ptr++;
      /* n_sep_by_space and int_n_sep_by_space */
      *lc_monetary_ptr = (char) getlocaleint (LOCALE_INEGSEPBYSPACE);
      _monetary_locale->n_sep_by_space
	    = _monetary_locale->int_n_sep_by_space = lc_monetary_ptr++;
      /* p_sign_posn and int_p_sign_posn */
      *lc_monetary_ptr = (char) getlocaleint (LOCALE_IPOSSIGNPOSN);
      _monetary_locale->p_sign_posn
	    = _monetary_locale->int_p_sign_posn = lc_monetary_ptr++;
      /* n_sign_posn and int_n_sign_posn */
      *lc_monetary_ptr = (char) getlocaleint (LOCALE_INEGSIGNPOSN);
      _monetary_locale->n_sign_posn
	    = _monetary_locale->int_n_sign_posn = lc_monetary_ptr++;
    }
  /* codeset */
  _monetary_locale->codeset = lc_monetary_ptr;
  lc_monetary_ptr = stpcpy (lc_monetary_ptr, charset) + 1;

  char *tmp = (char *) realloc (new_lc_monetary_buf,
				lc_monetary_ptr - new_lc_monetary_buf);
  if (!tmp)
    {
      free (new_lc_monetary_buf);
      return -1;
    }
  if (tmp != new_lc_monetary_buf)
    rebase_locale_buf (_monetary_locale, _monetary_locale + 1, tmp,
		       new_lc_monetary_buf, lc_monetary_ptr);
  *lc_monetary_buf = tmp;
  return 1;
}

extern "C" int
__set_lc_messages_from_win (const char *name,
			    const struct lc_messages_T *_C_messages_locale,
			    struct lc_messages_T *_messages_locale,
			    char **lc_messages_buf,
			    wctomb_p f_wctomb, const char *charset)
{
  wchar_t win_locale[ENCODING_LEN + 1];
  int ret = __get_rfc5646_from_locale (name, win_locale);
  if (ret < 0)
    return ret;
  if (!ret && !strcmp (charset, "ASCII"))
    return 0;

  char locale[ENCODING_LEN + 1];
  char *c, *c2;
  lc_msg_t *msg = NULL;

  /* C.foo is just a copy of "C" with fixed charset. */
  if (!ret)
    memcpy (_messages_locale, _C_messages_locale, sizeof (struct lc_messages_T));
  else
    {
      strcpy (locale, name);
      /* Removes the charset from the locale and attach the modifer to the
	 language_TERRITORY part. */
      c = strchr (locale, '.');
      if (c)
	{
	  *c = '\0';
	  c2 = strchr (c + 1, '@');
	  /* Ignore @cjknarrow modifier since it's a very personal thing between
	     Cygwin and newlib... */
	  if (c2 && strcmp (c2, "@cjknarrow"))
	    memmove (c, c2, strlen (c2) + 1);
	}
      /* Now search in the alphabetically order lc_msg array for the
	 locale. */
      lc_msg_t locale_key = { locale, NULL, NULL, NULL, NULL };
      msg = (lc_msg_t *) bsearch ((void *) &locale_key, (void *) lc_msg,
				  sizeof lc_msg / sizeof *lc_msg,
				  sizeof *lc_msg, locale_cmp);
      if (!msg)
	return 0;
    }

  /* Evaluate string length in target charset.  Characters invalid in the
     target charset are simply ignored, as on Linux. */
  size_t len = 0;
  len += (strlen (charset) + 1);
  if (ret)
    {
      len += lc_wcstombs (f_wctomb, NULL, msg->yesexpr, 0) + 1;
      len += lc_wcstombs (f_wctomb, NULL, msg->noexpr, 0) + 1;
      len += lc_wcstombs (f_wctomb, NULL, msg->yesstr, 0) + 1;
      len += lc_wcstombs (f_wctomb, NULL, msg->nostr, 0) + 1;
      len += (wcslen (msg->yesexpr) + 1) * sizeof (wchar_t);
      len += (wcslen (msg->noexpr) + 1) * sizeof (wchar_t);
      len += (wcslen (msg->yesstr) + 1) * sizeof (wchar_t);
      len += (wcslen (msg->nostr) + 1) * sizeof (wchar_t);
      if (len % 1)
	++len;
    }
  /* Allocate. */
  char *new_lc_messages_buf = (char *) malloc (len);
  const char *lc_messages_end = new_lc_messages_buf + len;

  if (!new_lc_messages_buf)
    return -1;
  /* Copy over. */
  c = new_lc_messages_buf;
  /* codeset */
  _messages_locale->codeset = c;
  c = stpcpy (c, charset) + 1;
  if (ret)
    {
      _messages_locale->yesexpr = (const char *) c;
      len = lc_wcstombs (f_wctomb, c, msg->yesexpr, lc_messages_end - c);
      _messages_locale->noexpr = (const char *) (c += len + 1);
      len = lc_wcstombs (f_wctomb, c, msg->noexpr, lc_messages_end - c);
      _messages_locale->yesstr = (const char *) (c += len + 1);
      len = lc_wcstombs (f_wctomb, c, msg->yesstr, lc_messages_end - c);
      _messages_locale->nostr = (const char *) (c += len + 1);
      len = lc_wcstombs (f_wctomb, c, msg->nostr, lc_messages_end - c);
      c += len + 1;
      if ((uintptr_t) c % 1)
	++c;
      wchar_t *wc = (wchar_t *) c;
      _messages_locale->wyesexpr = (const wchar_t *) wc;
      wc = wcpcpy (wc, msg->yesexpr) + 1;
      _messages_locale->wnoexpr = (const wchar_t *) wc;
      wc = wcpcpy (wc, msg->noexpr) + 1;
      _messages_locale->wyesstr = (const wchar_t *) wc;
      wc = wcpcpy (wc, msg->yesstr) + 1;
      _messages_locale->wnostr = (const wchar_t *) wc;
      wcpcpy (wc, msg->nostr);
    }
  *lc_messages_buf = new_lc_messages_buf;
  return 1;
}

const struct lc_collate_T _C_collate_locale =
{
  L"",
  __ascii_mbtowc,
  "ASCII"
};

/* Called from newlib's setlocale() if category is LC_COLLATE.  Stores
   LC_COLLATE locale information.  This is subsequently accessed by the
   below functions strcoll, strxfrm, wcscoll, wcsxfrm. */
extern "C" int
__collate_load_locale (struct __locale_t *locale, const char *name,
		       void *f_mbtowc, const char *charset)
{
  char *bufp = NULL;
  struct lc_collate_T *cop = NULL;

  wchar_t win_locale[ENCODING_LEN + 1];
  int ret = __get_rfc5646_from_locale (name, win_locale);
  if (ret < 0)
    return ret;
  if (ret)
    {
      bufp = (char *) malloc (1);	/* dummy */
      if (!bufp)
	return -1;
      cop = (struct lc_collate_T *) calloc (1, sizeof (struct lc_collate_T));
      if (!cop)
	{
	  free (bufp);
	  return -1;
	}
      wcscpy (cop->win_locale, win_locale);
      cop->mbtowc = (mbtowc_p) f_mbtowc;
      stpcpy (cop->codeset, charset);
    }
  struct __lc_cats tmp = locale->lc_cat[LC_COLLATE];
  locale->lc_cat[LC_COLLATE].ptr = !win_locale[0] ? &_C_collate_locale : cop;
  locale->lc_cat[LC_COLLATE].buf = bufp;
  /* If buf is not NULL, both pointers have been alloc'ed */
  if (tmp.buf)
    {
      free ((void *) tmp.ptr);
      free (tmp.buf);
    }
  return 0;
}

/* We use the Windows functions for locale-specific string comparison and
   transformation.  The advantage is that we don't need any files with
   collation information. */

extern "C" int
wcscoll_l (const wchar_t *__restrict ws1, const wchar_t *__restrict ws2,
	   struct __locale_t *locale)
{
  int ret;
  const wchar_t *collate_locale = __get_collate_locale (locale)->win_locale;

  if (!collate_locale[0])
    return wcscmp (ws1, ws2);
  ret = CompareStringEx (collate_locale, 0, ws1, -1, ws2, -1, NULL, NULL, 0);
  if (!ret)
    set_errno (EINVAL);
  return ret - CSTR_EQUAL;
}

extern "C" int
wcscoll (const wchar_t *__restrict ws1, const wchar_t *__restrict ws2)
{
  return wcscoll_l (ws1, ws2, __get_current_locale ());
}

extern "C" int
strcoll_l (const char *__restrict s1, const char *__restrict s2,
	   struct __locale_t *locale)
{
  size_t n1, n2;
  wchar_t *ws1, *ws2;
  tmp_pathbuf tp;
  int ret;
  const wchar_t *collate_locale = __get_collate_locale (locale)->win_locale;

  if (!collate_locale[0])
    return strcmp (s1, s2);
  mbtowc_p collate_mbtowc = __get_collate_locale (locale)->mbtowc;
  n1 = lc_mbstowcs (collate_mbtowc, NULL, s1, 0) + 1;
  ws1 = (n1 > NT_MAX_PATH ? (wchar_t *) malloc (n1 * sizeof (wchar_t))
			  : tp.w_get ());
  lc_mbstowcs (collate_mbtowc, ws1, s1, n1);
  n2 = lc_mbstowcs (collate_mbtowc, NULL, s2, 0) + 1;
  ws2 = (n2 > NT_MAX_PATH ? (wchar_t *) malloc (n2 * sizeof (wchar_t))
			  : tp.w_get ());
  lc_mbstowcs (collate_mbtowc, ws2, s2, n2);
  ret = CompareStringEx (collate_locale, 0, ws1, -1, ws2, -1, NULL, NULL, 0);
  if (n1 > NT_MAX_PATH)
    free (ws1);
  if (n2 > NT_MAX_PATH)
    free (ws2);
  if (!ret)
    set_errno (EINVAL);
  return ret - CSTR_EQUAL;
}

extern "C" int
strcoll (const char *__restrict s1, const char *__restrict s2)
{
  return strcoll_l (s1, s2, __get_current_locale ());
}

/* BSD.  Used from glob.cc, fnmatch.c and regcomp.c. */
extern "C" int
__wcollate_range_cmp (wint_t c1, wint_t c2)
{
  wchar_t s1[3] = { (wchar_t) c1, L'\0', L'\0' };
  wchar_t s2[3] = { (wchar_t) c2, L'\0', L'\0' };

  /* Handle Unicode values >= 0x10000, convert to surrogate pair */
  if (c1 > 0xffff)
    {
      s1[0] = ((c1 - 0x10000) >> 10) + 0xd800;
      s1[1] = ((c1 - 0x10000) & 0x3ff) + 0xdc00;
    }
  if (c2 > 0xffff)
    {
      s2[0] = ((c2 - 0x10000) >> 10) + 0xd800;
      s2[1] = ((c2 - 0x10000) & 0x3ff) + 0xdc00;
    }
  return wcscoll (s1, s2);
}

/* Not so much BSD.  Used from glob.cc, fnmatch.c and regcomp.c.

   The args are pointers to wint_t strings.  This allows to compare
   against collating symbols. */
extern "C" int
__wscollate_range_cmp (wint_t *c1, wint_t *c2,
		       size_t c1len, size_t c2len)
{
  wchar_t s1[c1len * 2 + 1] = { 0 };	/* # of chars if all are surrogates */
  wchar_t s2[c2len * 2 + 1] = { 0 };

  /* wcscoll() ignores case in many locales. but we don't want that
     for filenames... */
  if ((iswupper (*c1) && !iswupper (*c2))
      || (iswlower (*c1) && !iswlower (*c2)))
    return *c1 - *c2;

  wcintowcs (s1, c1, c1len);
  wcintowcs (s2, c2, c2len);
  return wcscoll_l (s1, s2, __get_current_locale ());
}

const size_t ce_size = sizeof collating_element / sizeof *collating_element;
const size_t ce_e_size = sizeof *collating_element;

/* Check if UTF-32 input character `test' is in the same equivalence class
   as UTF-32 character 'eqv'.
   Note that we only recognize input in Unicode normalization form C, that
   is, we expect all letters to be composed.  A single character is all we
   look at.
   To check equivalence, decompose pattern letter and input letter into
   normalization form KD and check the base character for equality.  Also,
   convert all digits to the ASCII digits 0 - 9 and compare. */
extern "C" int
is_unicode_equiv (wint_t test, wint_t eqv)
{
	wchar_t decomp_testc[24] = { 0 };
	wchar_t decomp_eqvc[24] = { 0 };
	wchar_t testc[3] = { 0 };
	wchar_t eqvc[3] = { 0 };

	/* For equivalence classes, case doesn't matter.  However, be careful.
	   Only convert chars which have a "upper" to "lower". */
	if (iswupper (eqv))
		eqv = towlower (eqv);
	if (iswupper (test))
		test = towlower (test);
	/* Convert to UTF-16 string */
	if (eqv > 0x10000) {
		eqvc[0] = ((eqv - 0x10000) >> 10) + 0xd800;
		eqvc[1] = ((eqv - 0x10000) & 0x3ff) + 0xdc00;
	} else
		eqvc[0] = eqv;
	if (test > 0x10000) {
		testc[0] = ((test - 0x10000) >> 10) + 0xd800;
		testc[1] = ((test - 0x10000) & 0x3ff) + 0xdc00;
	} else
		testc[0] = test;
	/* Convert to decomposed form */
	FoldStringW (MAP_COMPOSITE | MAP_FOLDCZONE | MAP_FOLDDIGITS,
		     eqvc, -1, decomp_eqvc, 24);
	FoldStringW (MAP_COMPOSITE | MAP_FOLDCZONE | MAP_FOLDDIGITS,
		     testc, -1, decomp_testc, 24);
	/* If they are equivalent, the base char must be the same. */
	if (decomp_eqvc[0] != decomp_testc[0])
		return 0;
	/* If it's a surrogate pair, check the second char, too */
	if (decomp_eqvc[0] >= 0xd800 && decomp_eqvc[0] <= 0xdbff &&
	    decomp_eqvc[1] != decomp_testc[1])
		return 0;
	return 1;
}

static int
comp_coll_elem (const void *key, const void *array_member)
{
  collating_element_t *ckey = (collating_element_t *) key;
  collating_element_t *carray_member = (collating_element_t *) array_member;

  int ret = wcicmp ((const wint_t *) ckey->element,
		    (const wint_t *) carray_member->element);
  /* The locale in the collating_element array never has a codeset
     attached.  So the length of the collating_element locale is
     always <= length of the key locale, and that's all we need to
     check.  Also, if the collating_element locale is empty, we're
     all set. */
  if (ret == 0 && carray_member->locale[0])
    ret = strncmp (ckey->locale, carray_member->locale,
		   strlen (carray_member->locale));
  return ret;
}

extern "C" int
is_unicode_coll_elem (const wint_t *test)
{
  collating_element_t ct = {
    (const char32_t *) test,
    __get_current_locale ()->categories[LC_COLLATE]
  };
  collating_element_t *cmatch;

  if (wcilen (test) == 1)
    return 1;
  cmatch = (collating_element_t *)
	   bsearch (&ct, collating_element, ce_size, ce_e_size, comp_coll_elem);
  return !!cmatch;
}

static int
comp_coll_elem_n (const void *key, const void *array_member)
{
  collating_element_t *ckey = (collating_element_t *) key;
  collating_element_t *carray_member = (collating_element_t *) array_member;

  int ret = wcincmp ((const wint_t *) ckey->element,
		     (const wint_t *) carray_member->element,
		     wcilen ((const wint_t *) carray_member->element));
  /* The locale in the collating_element array never has a codeset
     attached.  So the length of the collating_element locale is
     always <= length of the key locale, and that's all we need to
     check.  Also, if the collating_element locale is empty, we're
     all set. */
  if (ret == 0 && carray_member->locale[0])
    ret = strncmp (ckey->locale, carray_member->locale,
		   strlen (carray_member->locale));
  return ret;
}

/* Return the number of UTF-32 chars making up the next full character in
   inp, taking valid collation elements in the current locale into account. */
extern "C" size_t
next_unicode_char (wint_t *inp)
{
  collating_element_t ct = {
    (const char32_t *) inp,
    __get_current_locale ()->categories[LC_COLLATE]
  };
  collating_element_t *cmatch;

  if (wcilen (inp) > 1)
    {
      cmatch = (collating_element_t *)
	       bsearch (&ct, collating_element, ce_size, ce_e_size,
			comp_coll_elem_n);
      if (cmatch)
	return wcilen ((const wint_t *) cmatch->element);
    }
  return 1;
}

extern "C" size_t
wcsxfrm_l (wchar_t *__restrict ws1, const wchar_t *__restrict ws2, size_t wsn,
	   struct __locale_t *locale)
{
  size_t ret;
  const wchar_t *collate_locale = __get_collate_locale (locale)->win_locale;

  if (!collate_locale[0])
    return wcslcpy (ws1, ws2, wsn);
  /* Don't use LCMAP_SORTKEY in conjunction with LCMAP_BYTEREV.  The cchDest
     parameter is used as byte count with LCMAP_SORTKEY but as char count with
     LCMAP_BYTEREV. */
  ret = LCMapStringEx (collate_locale, LCMAP_SORTKEY, ws2, -1, ws1,
		       wsn * sizeof (wchar_t), NULL, NULL, 0);
  if (ret)
    {
      ret /= sizeof (wchar_t);
      if (wsn)
	{
	  /* Byte swap the array ourselves here. */
	  for (size_t idx = 0; idx < ret; ++idx)
	    ws1[idx] = __builtin_bswap16 (ws1[idx]);
	  /* LCMapStringW returns byte count including the terminating NUL char.
	     wcsxfrm is supposed to return length in wchar_t excluding the NUL.
	     Since the array is only single byte NUL-terminated yet, make sure
	     the result is wchar_t-NUL terminated. */
	  if (ret < wsn)
	    ws1[ret] = L'\0';
	}
      return ret;
    }
  if (GetLastError () != ERROR_INSUFFICIENT_BUFFER)
    set_errno (EINVAL);
  else
    {
      ret = LCMapStringEx (collate_locale, LCMAP_SORTKEY, ws2, -1,
			   NULL, 0, NULL, NULL, 0);
      if (ret)
	wsn = ret / sizeof (wchar_t);
    }
  return wsn;
}

extern "C" size_t
wcsxfrm (wchar_t *__restrict ws1, const wchar_t *__restrict ws2, size_t wsn)
{
  return wcsxfrm_l (ws1, ws2, wsn, __get_current_locale ());
}

extern "C" size_t
strxfrm_l (char *__restrict s1, const char *__restrict s2, size_t sn,
	   struct __locale_t *locale)
{
  size_t ret = 0;
  size_t n2;
  wchar_t *ws2;
  tmp_pathbuf tp;
  const wchar_t *collate_locale = __get_collate_locale (locale)->win_locale;

  if (!collate_locale[0])
    return strlcpy (s1, s2, sn);
  mbtowc_p collate_mbtowc = __get_collate_locale (locale)->mbtowc;
  n2 = lc_mbstowcs (collate_mbtowc, NULL, s2, 0) + 1;
  ws2 = (n2 > NT_MAX_PATH ? (wchar_t *) malloc (n2 * sizeof (wchar_t))
			  : tp.w_get ());
  if (ws2)
    {
      lc_mbstowcs (collate_mbtowc, ws2, s2, n2);
      /* The sort key is a NUL-terminated byte string. */
      ret = LCMapStringEx (collate_locale, LCMAP_SORTKEY, ws2, -1,
			  (PWCHAR) s1, sn, NULL, NULL, 0);
    }
  if (ret == 0)
    {
      ret = sn + 1;
      if (!ws2 || GetLastError () != ERROR_INSUFFICIENT_BUFFER)
	set_errno (EINVAL);
      else
	ret = LCMapStringEx (collate_locale, LCMAP_SORTKEY, ws2, -1,
			     NULL, 0, NULL, NULL, 0);
    }
  if (ws2 && n2 > NT_MAX_PATH)
    free (ws2);
  /* LCMapStringW returns byte count including the terminating NUL character.
     strxfrm is supposed to return length excluding the NUL. */
  return ret - 1;
}

extern "C" size_t
strxfrm (char *__restrict s1, const char *__restrict s2, size_t sn)
{
  return strxfrm_l (s1, s2, sn, __get_current_locale ());
}

/* Fetch default ANSI codepage from locale info and generate a setlocale
   compatible character set code.  Called from newlib's setlocale(), if the
   charset isn't given explicitely in the POSIX compatible locale specifier. */
extern "C" void
__set_charset_from_locale (const char *loc, char *charset)
{
  wchar_t win_locale[ENCODING_LEN + 1];
  char locale[ENCODING_LEN + 1];
  const char *modifier;
  char *c;
  UINT cp;

  /* Cut out explicit codeset */
  stpcpy (locale, loc);
  modifier = strchr (loc, '@');
  if ((c = strchr (locale, '.')))
    stpcpy (c, modifier ?: "");

  default_codeset_t srch_dc = { locale, NULL };
  default_codeset_t *dc = (default_codeset_t *)
	 bsearch ((void *) &srch_dc, (void *) default_codeset,
		  sizeof default_codeset / sizeof *default_codeset,
		  sizeof *default_codeset, locale_cmp);
  if (dc)
    {
      stpcpy (charset, dc->codeset);
      return;
    }

  /* "C" locale, or invalid locale? */
  if (__get_rfc5646_from_locale (locale, win_locale) <= 0)
    cp = 20127;
  else if (GetLocaleInfoEx (win_locale,
			    LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
			    (PWCHAR) &cp, sizeof cp))
    cp = 0;
  /* Translate codepage and lcid to a charset closely aligned with the default
     charsets defined in Glibc. */
  const char *cs;
  switch (cp)
    {
    case 20127:
      cs = "ASCII";
      break;
    case 874:
      cs = "CP874";
      break;
    case 932:
      cs = "EUCJP";
      break;
    case 936:
      cs = "GB2312";
      break;
    case 949:
      cs = "EUCKR";
      break;
    case 950:
      cs = "BIG5";
      break;
    case 1250:
      cs = "ISO-8859-2";
      break;
    case 1251:
      cs = "ISO-8859-5";
      break;
    case 1252:
      cs = "ISO-8859-1";
      break;
    case 1253:
      cs = "ISO-8859-7";
      break;
    case 1254:
      cs = "ISO-8859-9";
      break;
    case 1255:
      cs = "ISO-8859-8";
      break;
    case 1256:
      cs = "ISO-8859-6";
      break;
    case 1257:
      cs = "ISO-8859-13";
      break;
    case 1258:
    default:
      cs = "UTF-8";
      break;
    }
  stpcpy (charset, cs);
}

/* Called from fhandler_tty::setup_locale.  Set a codepage which reflects the
   internal charset setting.  This is *not* necessarily the Windows
   codepage connected to a locale by default, so we have to set this
   up explicitely. */
UINT
__eval_codepage_from_internal_charset ()
{
  const char *charset = __locale_charset (__get_global_locale ());
  UINT codepage = CP_UTF8; /* Default UTF8 */

  /* The internal charset names are well defined, so we can use shortcuts. */
  switch (charset[0])
    {
    case 'B': /* BIG5 */
      codepage = 950;
      break;
    case 'C': /* CPxxx */
      codepage = strtoul (charset + 2, NULL, 10);
      break;
    case 'E': /* EUCxx */
      switch (charset[3])
	{
	case 'J': /* EUCJP */
	  codepage = 20932;
	  break;
	case 'K': /* EUCKR */
	  codepage = 949;
	  break;
	case 'C': /* EUCCN */
	  codepage = 936;
	  break;
	}
      break;
    case 'G': /* GBK/GB2312/GB18030 */
      codepage = (charset[2] == '1') ? 54936 : 936;
      break;
    case 'I': /* ISO-8859-x */
      codepage = strtoul (charset + 9, NULL, 10) + 28590;
      break;
    case 'S': /* SJIS */
      codepage = 932;
      break;
    default: /* All set to UTF8 already */
      break;
    }
  return codepage;
}

/* This function is called from newlib's loadlocale if the locale identifier
   was invalid, one way or the other.  It looks for the file

     /usr/share/locale/locale.alias

   which is part of the gettext package, and if it finds the locale alias
   in that file, it replaces the locale with the correct locale string from
   that file.

   If successful, it returns a pointer to new_locale, NULL otherwise.*/
extern "C" char *
__set_locale_from_locale_alias (const char *locale, char *new_locale)
{
  wchar_t wlocale[ENCODING_LEN + 1];
  wchar_t walias[ENCODING_LEN + 1];
#define LOCALE_ALIAS_LINE_LEN 255
  char alias_buf[LOCALE_ALIAS_LINE_LEN + 1], *c;
  wchar_t *wc;
  const char *alias, *replace;
  char *ret = NULL;

  FILE *fp = fopen ("/usr/share/locale/locale.alias", "rt");
  if (!fp)
    return NULL;
  /* The incoming locale is given in the application charset, or in
     the Cygwin internal charset.  We try both. */
  if (mbstowcs (wlocale, locale, ENCODING_LEN + 1) == (size_t) -1)
    sys_mbstowcs (wlocale, ENCODING_LEN + 1, locale);
  wlocale[ENCODING_LEN] = L'\0';
  /* Ignore @cjknarrow modifier since it's a very personal thing between
     Cygwin and newlib... */
  if ((wc = wcschr (wlocale, L'@')) && !wcscmp (wc + 1, L"cjknarrow"))
    *wc = L'\0';
  while (fgets (alias_buf, LOCALE_ALIAS_LINE_LEN + 1, fp))
    {
      alias_buf[LOCALE_ALIAS_LINE_LEN] = '\0';
      c = strrchr (alias_buf, '\n');
      if (c)
	*c = '\0';
      c = alias_buf;
      c += strspn (c, " \t");
      if (!*c || *c == '#')
	continue;
      alias = c;
      c += strcspn (c, " \t");
      *c++ = '\0';
      c += strspn (c, " \t");
      if (*c == '#')
	continue;
      replace = c;
      c += strcspn (c, " \t");
      *c++ = '\0';
      if (strlen (replace) > ENCODING_LEN)
	continue;
      /* The file is latin1 encoded */
      lc_mbstowcs (__iso_mbtowc (1), walias, alias, ENCODING_LEN + 1);
      walias[ENCODING_LEN] = L'\0';
      if (!wcscmp (wlocale, walias))
	{
	  ret = strcpy (new_locale, replace);
	  break;
	}
    }
  fclose (fp);
  return ret;
}

/* Can be called via cygwin_internal (CW_INTERNAL_SETLOCALE) for application
   which really (think they) know what they are doing. */
extern "C" void
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
  char *path;
  wchar_t *w_path = NULL, *w_cwd;

  /* Don't do anything if the charset hasn't actually changed. */
  if (cygheap->locale.mbtowc == __get_global_locale ()->mbtowc)
    return;

  debug_printf ("Global charset set to %s",
		__locale_charset (__get_global_locale ()));
  /* Fetch PATH and CWD and convert to wchar_t in previous charset. */
  path = getenv ("PATH");
  if (path && *path)	/* $PATH can be potentially unset. */
    {
      w_path = tp.w_get ();
      _sys_mbstowcs (cygheap->locale.mbtowc, w_path, 32768, path);
    }
  w_cwd = tp.w_get ();
  cwdstuff::acquire_write ();
  _sys_mbstowcs (cygheap->locale.mbtowc, w_cwd, 32768,
		   cygheap->cwd.get_posix ());
  /* Set charset for internal conversion functions. */
  cygheap->locale.mbtowc = __get_global_locale ()->mbtowc;
  if (cygheap->locale.mbtowc == __ascii_mbtowc)
    cygheap->locale.mbtowc = __utf8_mbtowc;
  /* Restore CWD and PATH in new charset. */
  cygheap->cwd.reset_posix (w_cwd);
  cwdstuff::release_write ();
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
  if (ret)
    internal_setlocale ();
}
