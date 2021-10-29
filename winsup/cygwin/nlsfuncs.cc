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
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
#include "lc_msg.h"
#include "lc_era.h"

#define _LC(x)	&lc_##x##_ptr,lc_##x##_end-lc_##x##_ptr

#define getlocaleinfo(category,type) \
	    __getlocaleinfo(lcid,(type),_LC(category))
#define setlocaleinfo(category,val) \
	    __setlocaleinfo(_LC(category),(val))
#define eval_datetimefmt(type,flags) \
	    __eval_datetimefmt(lcid,(type),(flags),&lc_time_ptr,\
			       lc_time_end-lc_time_ptr)
#define charfromwchar(category,in) \
	    __charfromwchar (_##category##_locale->in,_LC(category),f_wctomb)

#define has_modifier(x)	((x)[0] && !strcmp (modifier, (x)))

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

  /* Speed up reusing the same locale as before, for instance in LC_ALL case. */
  if (!strcmp (name, last_locale))
    {
      debug_printf ("LCID=%04y", last_lcid);
      return last_lcid;
    }
  stpcpy (last_locale, name);
  stpcpy (locale, name);
  /* Store modifier for later use. */
  const char *modifier = strchr (last_locale, '@') ? : "";
  /* Drop charset and modifier */
  c = strchr (locale, '.');
  if (!c)
    c = strchr (locale, '@');
  if (c)
    *c = '\0';
  /* "POSIX" already converted to "C" in loadlocale. */
  if (!strcmp (locale, "C"))
    return last_lcid = 0;
  c = strchr (locale, '_');
  if (!c)
    return last_lcid = (LCID) -1;

  wchar_t wlocale[ENCODING_LEN + 1];

  /* Convert to RFC 4646 syntax. */
  *c = '-';
  mbstowcs (wlocale, locale, ENCODING_LEN + 1);
  lcid = LocaleNameToLCID (wlocale, 0);
  /* Bug on Windows 10: LocaleNameToLCID returns LOCALE_CUSTOM_UNSPECIFIED
     for unknown locales. */
  if (lcid == 0 || lcid == LOCALE_CUSTOM_UNSPECIFIED)
    {
      /* Unfortunately there are a couple of locales for which no form
	 without a Script part per RFC 4646 exists.
	 Linux also supports no_NO which is equivalent to nb_NO. */
      struct {
	const char    *loc;
	const wchar_t *wloc;
      } sc_only_locale[] = {
	{ "az-AZ" , L"az-Latn-AZ"  },
	{ "bs-BA" , L"bs-Latn-BA"  },
	{ "chr-US", L"chr-Cher-US"},
	{ "ff-SN" , L"ff-Latn-SN"  },
	{ "ha-NG" , L"ha-Latn-NG"  },
	{ "iu-CA" , L"iu-Latn-CA"  },
	{ "ks-IN" , L"ks-Arab-IN"  },
	{ "ku-IQ" , L"ku-Arab-IQ"  },
	{ "mn-CN" , L"mn-Mong-CN"  },
	{ "mn-MN" , L"mn-Mong-MN"  },
	{ "no-NO" , L"nb-NO"       },
	{ "pa-PK" , L"pa-Arab-PK"  },
	{ "quc-GT", L"quc-Latn-GT" },
	{ "sd-PK" , L"sd-Arab-PK"  },
	{ "sd-IN" , L"sd-Deva-IN"  },
	{ "sr-BA" , L"sr-Cyrl-BA"  },
	{ "sr-ME" , L"sr-Cyrl-ME"  },
	{ "sr-RS" , L"sr-Cyrl-RS"  },
	{ "tg-TJ" , L"tg-Cyrl-TJ"  },
	{ "tzm-DZ", L"tzm-Latn-DZ" },
	{ "tzm-MA", L"tzm-Tfng-MA" },
	{ "uz-UZ" , L"uz-Latn-UZ"  },
	{ NULL    , NULL	   }
      };
      for (int i = 0; sc_only_locale[i].loc
		      && sc_only_locale[i].loc[0] <= locale[0]; ++i)
	if (!strcmp (locale, sc_only_locale[i].loc))
	  {
	    lcid = LocaleNameToLCID (sc_only_locale[i].wloc, 0);
	    if (!strncmp (locale, "sr-", 3))
	      {
		/* "@latin" modifier for the sr_XY locales changes
		    collation behaviour so lcid should accommodate that
		    by being set to the Latin sublang. */
		if (lcid != 0 && lcid != LOCALE_CUSTOM_UNSPECIFIED
		    && has_modifier ("@latin"))
		  lcid = MAKELANGID (lcid & 0x3ff, (lcid >> 10) - 1);
	      }
	    else if (!strncmp (locale, "uz-", 3))
	      {
		/* Equivalent for "@cyrillic" modifier in uz_UZ locale */
		if (lcid != 0 && lcid != LOCALE_CUSTOM_UNSPECIFIED
		    && has_modifier ("@cyrillic"))
		  lcid = MAKELANGID (lcid & 0x3ff, (lcid >> 10) + 1);
	      }
	    break;
	  }
    }
  if (lcid && lcid != LOCALE_CUSTOM_UNSPECIFIED)
    last_lcid = lcid;
  else
    last_lcid = (LCID) -1;
  debug_printf ("LCID=%04y", last_lcid);
  return last_lcid;
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
__getlocaleinfo (LCID lcid, LCTYPE type, char **ptr, size_t size)
{
  size_t num;
  wchar_t *ret;

  if ((uintptr_t) *ptr % 1)
    ++*ptr;
  ret = (wchar_t *) *ptr;
  num = GetLocaleInfoW (lcid, type, ret, size / sizeof (wchar_t));
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
getlocaleint (LCID lcid, LCTYPE type)
{
  UINT val;
  return GetLocaleInfoW (lcid, type | LOCALE_RETURN_NUMBER, (PWCHAR) &val,
			 sizeof val) ? val : 0;
}

enum dt_flags {
  DT_DEFAULT	= 0x00,
  DT_AMPM	= 0x01,	/* Enforce 12 hour time format. */
  DT_ABBREV	= 0x02,	/* Enforce abbreviated month and day names. */
};

static wchar_t *
__eval_datetimefmt (LCID lcid, LCTYPE type, dt_flags flags, char **ptr,
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
  GetLocaleInfoW (lcid, type, buf, 80);
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
__set_lc_time_from_win (const char *name,
			const struct lc_time_T *_C_time_locale,
			struct lc_time_T *_time_locale,
			char **lc_time_buf, wctomb_p f_wctomb,
			const char *charset)
{
  LCID lcid = __get_lcid_from_locale (name);
  if (lcid == (LCID) -1)
    return lcid;
  if (!lcid && !strcmp (charset, "ASCII"))
    return 0;

# define MAX_TIME_BUFFER_SIZE	4096

  char *new_lc_time_buf = (char *) malloc (MAX_TIME_BUFFER_SIZE);
  const char *lc_time_end = new_lc_time_buf + MAX_TIME_BUFFER_SIZE;

  if (!new_lc_time_buf)
    return -1;
  char *lc_time_ptr = new_lc_time_buf;

  /* C.foo is just a copy of "C" with fixed charset. */
  if (!lcid)
    memcpy (_time_locale, _C_time_locale, sizeof (struct lc_time_T));
  /* codeset */
  _time_locale->codeset = lc_time_ptr;
  lc_time_ptr = stpcpy (lc_time_ptr, charset) + 1;

  if (lcid)
    {
      char locale[ENCODING_LEN + 1];
      strcpy (locale, name);
      /* Removes the charset from the locale and attach the modifer to the
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
      lc_era_t *era = (lc_era_t *) bsearch ((void *) &locale_key, (void *) lc_era,
					    sizeof lc_era / sizeof *lc_era,
					    sizeof *lc_era, locale_cmp);

      /* mon */
      /* Windows has a bug in Japanese and Korean locales.  In these
	 locales, strings returned for LOCALE_SABBREVMONTHNAME* are missing
	 the suffix representing a month.  Unfortunately this is not
	 documented in English.  A Japanese article describing the problem
	 is http://msdn.microsoft.com/ja-jp/library/cc422084.aspx
	 The workaround is to use LOCALE_SMONTHNAME* in these locales,
	 even for the abbreviated month name. */
      const LCTYPE mon_base =
		lcid == MAKELANGID (LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN)
		|| lcid == MAKELANGID (LANG_KOREAN, SUBLANG_KOREAN)
		? LOCALE_SMONTHNAME1 : LOCALE_SABBREVMONTHNAME1;
      for (int i = 0; i < 12; ++i)
	{
	  _time_locale->wmon[i] = getlocaleinfo (time, mon_base + i);
	  _time_locale->mon[i] = charfromwchar (time, wmon[i]);
	}
      /* month and alt_month */
      for (int i = 0; i < 12; ++i)
	{
	  _time_locale->wmonth[i] = getlocaleinfo (time, LOCALE_SMONTHNAME1 + i);
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
	GetLocaleInfoW (lcid, LOCALE_IDATE, buf, 80);
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
  LCID lcid = __get_lcid_from_locale (name);
  if (lcid == (LCID) -1)
    return lcid;
  if (!lcid && !strcmp (charset, "ASCII"))
    return 0;

# define MAX_CTYPE_BUFFER_SIZE	256

  char *new_lc_ctype_buf = (char *) malloc (MAX_CTYPE_BUFFER_SIZE);

  if (!new_lc_ctype_buf)
    return -1;
  char *lc_ctype_ptr = new_lc_ctype_buf;
  /* C.foo is just a copy of "C" with fixed charset. */
  if (!lcid)
    memcpy (_ctype_locale, _C_ctype_locale, sizeof (struct lc_ctype_T));
  /* codeset */
  _ctype_locale->codeset = lc_ctype_ptr;
  lc_ctype_ptr = stpcpy (lc_ctype_ptr, charset) + 1;
  /* mb_cur_max */
  _ctype_locale->mb_cur_max = lc_ctype_ptr;
  *lc_ctype_ptr++ = mb_cur_max;
  *lc_ctype_ptr++ = '\0';
  if (lcid)
    {
      /* outdigits and woutdigits */
      wchar_t digits[11];
      GetLocaleInfoW (lcid, LOCALE_SNATIVEDIGITS, digits, 11);
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
  LCID lcid = __get_lcid_from_locale (name);
  if (lcid == (LCID) -1)
    return lcid;
  if (!lcid && !strcmp (charset, "ASCII"))
    return 0;

# define MAX_NUMERIC_BUFFER_SIZE	256

  char *new_lc_numeric_buf = (char *) malloc (MAX_NUMERIC_BUFFER_SIZE);
  const char *lc_numeric_end = new_lc_numeric_buf + MAX_NUMERIC_BUFFER_SIZE;

  if (!new_lc_numeric_buf)
    return -1;
  char *lc_numeric_ptr = new_lc_numeric_buf;
  /* C.foo is just a copy of "C" with fixed charset. */
  if (!lcid)
    memcpy (_numeric_locale, _C_numeric_locale, sizeof (struct lc_numeric_T));
  else
    {
      /* decimal_point and thousands_sep */
      if (lcid == 0x0429)	/* fa_IR.  Windows decimal_point is slash,
					   correct is dot */
	{
	  _numeric_locale->wdecimal_point = setlocaleinfo (numeric, L'.');
	  _numeric_locale->wthousands_sep = setlocaleinfo (numeric, L',');
	}
      else if (lcid == 0x0463)	/* ps_AF.  Windows decimal_point is dot,
					   thousands_sep is comma, correct are
					   arabic separators. */
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
      _numeric_locale->grouping = conv_grouping (lcid, LOCALE_SGROUPING,
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
  LCID lcid = __get_lcid_from_locale (name);
  if (lcid == (LCID) -1)
    return lcid;
  if (!lcid && !strcmp (charset, "ASCII"))
    return 0;

# define MAX_MONETARY_BUFFER_SIZE	512

  char *new_lc_monetary_buf = (char *) malloc (MAX_MONETARY_BUFFER_SIZE);
  const char *lc_monetary_end = new_lc_monetary_buf + MAX_MONETARY_BUFFER_SIZE;

  if (!new_lc_monetary_buf)
    return -1;
  char *lc_monetary_ptr = new_lc_monetary_buf;
  /* C.foo is just a copy of "C" with fixed charset. */
  if (!lcid)
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
      if (lcid == 0x0429 || lcid == 0x0463)	/* fa_IR or ps_AF.  Windows
						   mon_decimal_point is slash
						   and comma, mon_thousands_sep
						   is comma and dot, correct
						   are arabic separators. */
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
      _monetary_locale->mon_grouping = conv_grouping (lcid, LOCALE_SMONGROUPING,
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
      *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_IINTLCURRDIGITS);
      _monetary_locale->int_frac_digits = lc_monetary_ptr++;
      /* frac_digits */
      *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_ICURRDIGITS);
      _monetary_locale->frac_digits = lc_monetary_ptr++;
      /* p_cs_precedes and int_p_cs_precedes */
      *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_IPOSSYMPRECEDES);
      _monetary_locale->p_cs_precedes
	    = _monetary_locale->int_p_cs_precedes = lc_monetary_ptr++;
      /* p_sep_by_space and int_p_sep_by_space */
      *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_IPOSSEPBYSPACE);
      _monetary_locale->p_sep_by_space
	    = _monetary_locale->int_p_sep_by_space = lc_monetary_ptr++;
      /* n_cs_precedes and int_n_cs_precedes */
      *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_INEGSYMPRECEDES);
      _monetary_locale->n_cs_precedes
	    = _monetary_locale->int_n_cs_precedes = lc_monetary_ptr++;
      /* n_sep_by_space and int_n_sep_by_space */
      *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_INEGSEPBYSPACE);
      _monetary_locale->n_sep_by_space
	    = _monetary_locale->int_n_sep_by_space = lc_monetary_ptr++;
      /* p_sign_posn and int_p_sign_posn */
      *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_IPOSSIGNPOSN);
      _monetary_locale->p_sign_posn
	    = _monetary_locale->int_p_sign_posn = lc_monetary_ptr++;
      /* n_sign_posn and int_n_sign_posn */
      *lc_monetary_ptr = (char) getlocaleint (lcid, LOCALE_INEGSIGNPOSN);
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
  LCID lcid = __get_lcid_from_locale (name);
  if (lcid == (LCID) -1)
    return lcid;
  if (!lcid && !strcmp (charset, "ASCII"))
    return 0;

  char locale[ENCODING_LEN + 1];
  char *c, *c2;
  lc_msg_t *msg = NULL;

  /* C.foo is just a copy of "C" with fixed charset. */
  if (!lcid)
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
  if (lcid)
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
  if (lcid)
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
  0,
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

  LCID lcid = __get_lcid_from_locale (name);
  if (lcid == (LCID) -1)
    return -1;
  if (lcid)
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
      cop->lcid = lcid;
      cop->mbtowc = (mbtowc_p) f_mbtowc;
      stpcpy (cop->codeset, charset);
    }
  struct __lc_cats tmp = locale->lc_cat[LC_COLLATE];
  locale->lc_cat[LC_COLLATE].ptr = lcid == 0 ? &_C_collate_locale : cop;
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
  LCID collate_lcid = __get_collate_locale (locale)->lcid;

  if (!collate_lcid)
    return wcscmp (ws1, ws2);
  ret = CompareStringW (collate_lcid, 0, ws1, -1, ws2, -1);
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
  LCID collate_lcid = __get_collate_locale (locale)->lcid;

  if (!collate_lcid)
    return strcmp (s1, s2);
  /* The ANSI version of CompareString uses the default charset of the lcid,
     so we must use the Unicode version. */
  mbtowc_p collate_mbtowc = __get_collate_locale (locale)->mbtowc;
  n1 = lc_mbstowcs (collate_mbtowc, NULL, s1, 0) + 1;
  ws1 = (n1 > NT_MAX_PATH ? (wchar_t *) malloc (n1 * sizeof (wchar_t))
			  : tp.w_get ());
  lc_mbstowcs (collate_mbtowc, ws1, s1, n1);
  n2 = lc_mbstowcs (collate_mbtowc, NULL, s2, 0) + 1;
  ws2 = (n2 > NT_MAX_PATH ? (wchar_t *) malloc (n2 * sizeof (wchar_t))
			  : tp.w_get ());
  lc_mbstowcs (collate_mbtowc, ws2, s2, n2);
  ret = CompareStringW (collate_lcid, 0, ws1, -1, ws2, -1);
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

/* BSD.  Used from glob.cc, fnmatch.c and regcomp.c.  Make sure caller is
   using wide chars.  Unfortunately the definition of this functions hides
   the required input type. */
extern "C" int
__collate_range_cmp (int c1, int c2)
{
  wchar_t s1[2] = { (wchar_t) c1, L'\0' };
  wchar_t s2[2] = { (wchar_t) c2, L'\0' };
  return wcscoll (s1, s2);
}

extern "C" size_t
wcsxfrm_l (wchar_t *__restrict ws1, const wchar_t *__restrict ws2, size_t wsn,
	   struct __locale_t *locale)
{
  size_t ret;
  LCID collate_lcid = __get_collate_locale (locale)->lcid;

  if (!collate_lcid)
    return wcslcpy (ws1, ws2, wsn);
  /* Don't use LCMAP_SORTKEY in conjunction with LCMAP_BYTEREV.  The cchDest
     parameter is used as byte count with LCMAP_SORTKEY but as char count with
     LCMAP_BYTEREV. */
  ret = LCMapStringW (collate_lcid, LCMAP_SORTKEY, ws2, -1, ws1,
		      wsn * sizeof (wchar_t));
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
      ret = LCMapStringW (collate_lcid, LCMAP_SORTKEY, ws2, -1, NULL, 0);
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
  LCID collate_lcid = __get_collate_locale (locale)->lcid;

  if (!collate_lcid)
    return strlcpy (s1, s2, sn);
  /* The ANSI version of LCMapString uses the default charset of the lcid,
     so we must use the Unicode version. */
  mbtowc_p collate_mbtowc = __get_collate_locale (locale)->mbtowc;
  n2 = lc_mbstowcs (collate_mbtowc, NULL, s2, 0) + 1;
  ws2 = (n2 > NT_MAX_PATH ? (wchar_t *) malloc (n2 * sizeof (wchar_t))
			  : tp.w_get ());
  if (ws2)
    {
      lc_mbstowcs (collate_mbtowc, ws2, s2, n2);
      /* The sort key is a NUL-terminated byte string. */
      ret = LCMapStringW (collate_lcid, LCMAP_SORTKEY, ws2, -1,
			  (PWCHAR) s1, sn);
    }
  if (ret == 0)
    {
      ret = sn + 1;
      if (!ws2 || GetLastError () != ERROR_INSUFFICIENT_BUFFER)
	set_errno (EINVAL);
      else
	ret = LCMapStringW (collate_lcid, LCMAP_SORTKEY, ws2, -1, NULL, 0);
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
__set_charset_from_locale (const char *locale, char *charset)
{
  UINT cp;
  LCID lcid = __get_lcid_from_locale (locale);
  wchar_t wbuf[9];

  /* "C" locale, or invalid locale? */
  if (lcid == 0 || lcid == (LCID) -1)
    cp = 20127;
  else if (!GetLocaleInfoW (lcid,
			    LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
			    (PWCHAR) &cp, sizeof cp))
    cp = 0;
  /* Translate codepage and lcid to a charset closely aligned with the default
     charsets defined in Glibc. */
  const char *cs;
  const char *modifier = strchr (locale, '@') ?: "";
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
      if (lcid == 0x181a		/* sr_BA (Serbian Language/Bosnia
						  and Herzegovina) */
	  || lcid == 0x241a		/* sr_RS (Serbian Language/Serbia) */
	  || lcid == 0x2c1a		/* sr_ME (Serbian Language/Montenegro)*/
	  || lcid == 0x0442)		/* tk_TM (Turkmen/Turkmenistan) */
	cs = "UTF-8";
      else if (lcid == 0x041c)		/* sq_AL (Albanian/Albania) */
	cs = "ISO-8859-1";
      else
	cs = "ISO-8859-2";
      break;
    case 1251:
      if (lcid == 0x1c1a		/* sr_BA (Serbian Language/Bosnia
						  and Herzegovina) */
	  || lcid == 0x281a		/* sr_RS (Serbian Language/Serbia) */
	  || lcid == 0x301a		/* sr_ME (Serbian Language/Montenegro)*/
	  || lcid == 0x0440		/* ky_KG (Kyrgyz/Kyrgyzstan) */
	  || lcid == 0x0843		/* uz_UZ (Uzbek/Uzbekistan) */
					/* tt_RU (Tatar/Russia),
						 IQTElif alphabet */
	  || (lcid == 0x0444 && has_modifier ("@iqtelif"))
	  || lcid == 0x0450)		/* mn_MN (Mongolian/Mongolia) */
	cs = "UTF-8";
      else if (lcid == 0x0423)		/* be_BY (Belarusian/Belarus) */
	cs = has_modifier ("@latin") ? "UTF-8" : "CP1251";
      else if (lcid == 0x0402)		/* bg_BG (Bulgarian/Bulgaria) */
	cs = "CP1251";
      else if (lcid == 0x0422)		/* uk_UA (Ukrainian/Ukraine) */
	cs = "KOI8-U";
      else
	cs = "ISO-8859-5";
      break;
    case 1252:
      if (lcid == 0x0452)		/* cy_GB (Welsh/Great Britain) */
	cs = "ISO-8859-14";
      else if (lcid == 0x4009		/* en_IN (English/India) */
	       || lcid == 0x0867	/* ff_SN (Fulah/Senegal) */
	       || lcid == 0x0464	/* fil_PH (Filipino/Philippines) */
	       || lcid == 0x0462	/* fy_NL (Frisian/Netherlands) */
	       || lcid == 0x0468	/* ha_NG (Hausa/Nigeria) */
	       || lcid == 0x0475	/* haw_US (Hawaiian/United States) */
	       || lcid == 0x0470	/* ig_NG (Igbo/Nigeria) */
	       || lcid == 0x085d	/* iu_CA (Inuktitut/Canada) */
	       || lcid == 0x046c	/* nso_ZA (Northern Sotho/South Africa) */
	       || lcid == 0x0487	/* rw_RW (Kinyarwanda/Rwanda) */
	       || lcid == 0x043b	/* se_NO (Northern Saami/Norway) */
	       || lcid == 0x0832	/* tn_BW (Tswana/Botswana) */
	       || lcid == 0x0432	/* tn_ZA (Tswana/South Africa) */
	       || lcid == 0x0488	/* wo_SN (Wolof/Senegal) */
	       || lcid == 0x046a)	/* yo_NG (Yoruba/Nigeria) */
	cs = "UTF-8";
      else if (lcid == 0x042e)		/* hsb_DE (Upper Sorbian/Germany) */
	cs = "ISO-8859-2";
      else if (lcid == 0x0491		/* gd_GB (Scots Gaelic/Great Britain) */
	       || (has_modifier ("@euro")
		   && GetLocaleInfoW (lcid, LOCALE_SINTLSYMBOL, wbuf, 9)
		   && !wcsncmp (wbuf, L"EUR", 3)))
	cs = "ISO-8859-15";
      else
	cs = "ISO-8859-1";
      break;
    case 1253:
      cs = "ISO-8859-7";
      break;
    case 1254:
      if (lcid == 0x042c)		/* az_AZ (Azeri/Azerbaijan) */
	cs = "UTF-8";
      else if (lcid == 0x0443)		/* uz_UZ (Uzbek/Uzbekistan) */
	cs = "ISO-8859-1";
      else
	cs = "ISO-8859-9";
      break;
    case 1255:
      cs = "ISO-8859-8";
      break;
    case 1256:
      if (lcid == 0x0429		/* fa_IR (Persian/Iran) */
	  || lcid == 0x0846		/* pa_PK (Punjabi/Pakistan) */
	  || lcid == 0x0859		/* sd_PK (Sindhi/Pakistan) */
	  || lcid == 0x0480		/* ug_CN (Uyghur/China) */
	  || lcid == 0x0420)		/* ur_PK (Urdu/Pakistan) */
	cs = "UTF-8";
      else
	cs = "ISO-8859-6";
      break;
    case 1257:
      if (lcid == 0x0425)		/* et_EE (Estonian/Estonia) */
	cs = "ISO-8859-15";
      else
	cs = "ISO-8859-13";
      break;
    case 1258:
    default:
      if (lcid == 0x3c09		/* en_HK (English/Hong Kong) */
	  || lcid == 0x200c		/* fr_RE (French/Réunion) */
	  || lcid == 0x240c		/* fr_CD (French/Congo) */
	  || lcid == 0x280c		/* fr_SN (French/Senegal) */
	  || lcid == 0x2c0c		/* fr_CM (French/Cameroon) */
	  || lcid == 0x300c		/* fr_CI (French/Ivory Coast) */
	  || lcid == 0x340c		/* fr_ML (French/Mali) */
	  || lcid == 0x380c		/* fr_MA (French/Morocco) */
	  || lcid == 0x3c0c		/* fr_HT (French/Haiti) */
	  || lcid == 0x0477		/* so_SO (Somali/Somali) */
	  || lcid == 0x0430)		/* st_ZA (Sotho/South Africa) */
	cs = "ISO-8859-1";
      else if (lcid == 0x818)		/* ro_MD (Romanian/Moldovia) */
	cs = "ISO-8859-2";
      else if (lcid == 0x043a)		/* mt_MT (Maltese/Malta) */
	cs = "ISO-8859-3";
      else if (lcid == 0x0481)		/* mi_NZ (Maori/New Zealand) */
	cs = "ISO-8859-13";
      else if (lcid == 0x0437)		/* ka_GE (Georgian/Georgia) */
	cs = "GEORGIAN-PS";
      else if (lcid == 0x043f)		/* kk_KZ (Kazakh/Kazakhstan) */
	cs = "PT154";
      else
	cs = "UTF-8";
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
    case 'G': /* GBK/GB2312 */
      codepage = 936;
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
  cwdstuff::cwd_lock.acquire ();
  _sys_mbstowcs (cygheap->locale.mbtowc, w_cwd, 32768,
		   cygheap->cwd.get_posix ());
  /* Set charset for internal conversion functions. */
  cygheap->locale.mbtowc = __get_global_locale ()->mbtowc;
  if (cygheap->locale.mbtowc == __ascii_mbtowc)
    cygheap->locale.mbtowc = __utf8_mbtowc;
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
  if (ret)
    internal_setlocale ();
}
