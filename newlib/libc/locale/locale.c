/*
FUNCTION
<<setlocale>>, <<localeconv>>---select or query locale

INDEX
	setlocale
INDEX
	localeconv

SYNOPSIS
	#include <locale.h>
	char *setlocale(int <[category]>, const char *<[locale]>);
	lconv *localeconv(void);

DESCRIPTION
<<setlocale>> is the facility defined by ANSI C to condition the
execution environment for international collating and formatting
information; <<localeconv>> reports on the settings of the current
locale.

This is a minimal implementation, supporting only the required <<"POSIX">>
and <<"C">> values for <[locale]>; strings representing other locales are not
honored unless _MB_CAPABLE is defined.

If _MB_CAPABLE is defined, POSIX locale strings are allowed, following
the form

  language[_TERRITORY][.charset][@@modifier]

<<"language">> is a two character string per ISO 639, or, if not available
for a given language, a three character string per ISO 639-3.
<<"TERRITORY">> is a country code per ISO 3166.  For <<"charset">> and
<<"modifier">> see below.

Additionally to the POSIX specifier, the following extension is supported
for backward compatibility with older implementations using newlib:
<<"C-charset">>.
Instead of <<"C-">>, you can also specify <<"C.">>.  Both variations allow
to specify language neutral locales while using other charsets than ASCII,
for instance <<"C.UTF-8">>, which keeps all settings as in the C locale,
but uses the UTF-8 charset.

The following charsets are recognized:
<<"UTF-8">>, <<"JIS">>, <<"EUCJP">>, <<"SJIS">>, <<"KOI8-R">>, <<"KOI8-U">>,
<<"KOI8-T">>, <<"GEORGIAN-PS">>, <<"PT154">>, <<"TIS-620">>, <<"ISO-8859-x">>
with 1 <= x <= 16, or <<"CPxxx">> with xxx in [437, 720, 737, 775, 850, 852,
855, 857, 858, 862, 866, 874, 932, 1125, 1250, 1251, 1252, 1253, 1254, 1255,
1256, 1257, 1258].

Charsets are case insensitive.  For instance, <<"EUCJP">> and <<"eucJP">>
are equivalent.  Charset names with dashes can also be written without
dashes, as in <<"UTF8">>, <<"iso88591">> or <<"koi8r">>.  <<"EUCJP">> and
<<"EUCKR">> are also recognized with dash, <<"EUC-JP">> and <<"EUC-KR">>.

Full support for all of the above charsets requires that newlib has been
build with multibyte support and support for all ISO and Windows Codepage.
Otherwise all singlebyte charsets are simply mapped to ASCII.  Right now,
only newlib for Cygwin is built with full charset support by default.
Under Cygwin, this implementation additionally supports the charsets
<<"GB18030">>, <<"GBK">>, <<"GB2312">>, <<"eucCN">>, <<"eucKR">>, and
<<"Big5">>.  Cygwin does not support <<"JIS">>.

Cygwin additionally supports locales from the file
/usr/share/locale/locale.alias.

(<<"">> is also accepted; if given, the settings are read from the
corresponding LC_* environment variables and $LANG according to POSIX rules.)

This implementation also supports the modifiers <<"cjknarrow">> and
<<"cjkwide">>, which affect how the functions <<wcwidth>> and <<wcswidth>>
handle characters from the "CJK Ambiguous Width" category of characters
described at http://www.unicode.org/reports/tr11/#Ambiguous.
These characters have a width of 1 for singlebyte charsets and UTF-8,
and a width of 2 for multibyte charsets other than UTF-8. Specifying
<<"cjknarrow">> or <<"cjkwide">> forces a width of 1 or 2, respectively.

This implementation also supports the modifier <<"cjksingle">>
to enforce single-width character properties.

If you use <<NULL>> as the <[locale]> argument, <<setlocale>> returns a
pointer to the string representing the current locale.  The acceptable
values for <[category]> are defined in `<<locale.h>>' as macros
beginning with <<"LC_">>.

<<localeconv>> returns a pointer to a structure (also defined in
`<<locale.h>>') describing the locale-specific conventions currently
in effect.  

RETURNS
A successful call to <<setlocale>> returns a pointer to a string
associated with the specified category for the new locale.  The string
returned by <<setlocale>> is such that a subsequent call using that
string will restore that category (or all categories in case of LC_ALL),
to that state.  The application shall not modify the string returned
which may be overwritten by a subsequent call to <<setlocale>>.
On error, <<setlocale>> returns <<NULL>>.

<<localeconv>> returns a pointer to a structure of type <<lconv>>,
which describes the formatting and collating conventions in effect (in
this implementation, always those of the C locale).

PORTABILITY
ANSI C requires <<setlocale>>, but the only locale required across all
implementations is the C locale.

NOTES
There is no ISO-8859-12 codepage.  It's also refused by this implementation.

No supporting OS subroutines are required.
*/

/* Parts of this code are originally taken from FreeBSD. */
/*
 * Copyright (c) 1996 - 2002 FreeBSD Project
 * Copyright (c) 1991, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define _DEFAULT_SOURCE
#define _PICOLIBC_CTYPE_SMALL 0
#include <newlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <wchar.h>
#include "setlocale.h"
#include "../ctype/ctype_.h"
#include "../stdlib/local.h"

#ifdef __CYGWIN__ /* Has to be kept available as exported symbol for
		     backward compatibility.  Set it in setlocale, but
		     otherwise ignore it.  Applications compiled after
		     2010 don't use it anymore. */
int __EXPORT __mb_cur_max = 6;
#endif

#ifdef __HAVE_LOCALE_INFO__
NEWLIB_THREAD_LOCAL struct __locale_t *_locale = &__global_locale;
#endif

char *_PathLocale = NULL;

#ifdef _MB_CAPABLE
/*
 * Category names for getenv()
 */
static char *categories[_LC_LAST] = {
  "LC_ALL",
  "LC_COLLATE",
  "LC_CTYPE",
  "LC_MONETARY",
  "LC_NUMERIC",
  "LC_TIME",
  "LC_MESSAGES",
};
#endif /* _MB_CAPABLE */

/*
 * Default locale per POSIX.  Can be overridden on a per-target base.
 */
#ifndef DEFAULT_LOCALE
#define DEFAULT_LOCALE	"C"
#endif

#ifdef _MB_CAPABLE
/*
 * This variable can be changed by any outside mechanism.  This allows,
 * for instance, to load the default locale from a file.
 */
static const char __default_locale[ENCODING_LEN + 1] = DEFAULT_LOCALE;

const struct __locale_t __C_locale =
{
  { "C", "C", "C", "C", "C", "C", "C", },
  __ascii_wctomb,
  __ascii_mbtowc,
  0,
  DEFAULT_CTYPE_PTR,
  {
    ".", "", "", "", "", "", "", "", "", "",
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX
  },
#ifndef __HAVE_LOCALE_INFO__
  "\1",
  "ASCII",
  "ASCII",
#else /* __HAVE_LOCALE_INFO__ */
  {
    { NULL, NULL },			/* LC_ALL */
#ifdef __CYGWIN__
    { &_C_collate_locale, NULL },	/* LC_COLLATE */
#else
    { NULL, NULL },			/* LC_COLLATE */
#endif
    { &_C_ctype_locale, NULL },		/* LC_CTYPE */
    { &_C_monetary_locale, NULL },	/* LC_MONETARY */
    { &_C_numeric_locale, NULL },	/* LC_NUMERIC */
    { &_C_time_locale, NULL },		/* LC_TIME */
    { &_C_messages_locale, NULL },	/* LC_MESSAGES */
  },
#endif /* __HAVE_LOCALE_INFO__ */
};
#endif /* _MB_CAPABLE */

struct __locale_t __global_locale =
{
  { "C", "C", DEFAULT_LOCALE, "C", "C", "C", "C", },
#ifdef __CYGWIN__
  __utf8_wctomb,
  __utf8_mbtowc,
#else
  __ascii_wctomb,
  __ascii_mbtowc,
#endif
  0,
  DEFAULT_CTYPE_PTR,
  {
    ".", "", "", "", "", "", "", "", "", "",
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX
  },
#ifndef __HAVE_LOCALE_INFO__
  "\1",
  "ASCII",
  "ASCII",
#else /* __HAVE_LOCALE_INFO__ */
  {
    { NULL, NULL },			/* LC_ALL */
#ifdef __CYGWIN__
    { &_C_collate_locale, NULL },	/* LC_COLLATE */
    { &_C_utf8_ctype_locale, NULL },	/* LC_CTYPE */
#else
    { NULL, NULL },			/* LC_COLLATE */
    { &_C_ctype_locale, NULL },		/* LC_CTYPE */
#endif
    { &_C_monetary_locale, NULL },	/* LC_MONETARY */
    { &_C_numeric_locale, NULL },	/* LC_NUMERIC */
    { &_C_time_locale, NULL },		/* LC_TIME */
    { &_C_messages_locale, NULL },	/* LC_MESSAGES */
  },
#endif /* __HAVE_LOCALE_INFO__ */
};

#ifdef _MB_CAPABLE
/* Renamed from current_locale_string to make clear this is only the
   *global* string for setlocale (LC_ALL, NULL).  There's no equivalent
   functionality for uselocale. */
static char global_locale_string[_LC_LAST * (ENCODING_LEN + 1/*"/"*/ + 1)]
	    = "C";
static char *currentlocale (void);

#endif /* _MB_CAPABLE */

char *
setlocale (
       int category,
       const char *locale)
{
  (void) category;
#ifndef _MB_CAPABLE
  if (locale)
    { 
      if (strcmp (locale, "POSIX") && strcmp (locale, "C")
	  && strcmp (locale, ""))
        return NULL;
    }
  return "C";
#else /* _MB_CAPABLE */
  static char new_categories[_LC_LAST][ENCODING_LEN + 1];
  static char saved_categories[_LC_LAST][ENCODING_LEN + 1];
  int i, j, len, saverr;
  const char *env, *r;
  char *ret;

  if (category < LC_ALL || category >= _LC_LAST)
    {
      _REENT_ERRNO(p) = EINVAL;
      return NULL;
    }

  if (locale == NULL)
    return category != LC_ALL ? __get_global_locale ()->categories[category]
			      : global_locale_string;

  /*
   * Default to the current locale for everything.
   */
  for (i = 1; i < _LC_LAST; ++i)
    strcpy (new_categories[i], __get_global_locale ()->categories[i]);

  /*
   * Now go fill up new_categories from the locale argument
   */
  if (!*locale)
    {
      if (category == LC_ALL)
	{
	  for (i = 1; i < _LC_LAST; ++i)
	    {
	      env = __get_locale_env (i);
	      if (strlen (env) > ENCODING_LEN)
		{
		  _REENT_ERRNO(p) = EINVAL;
		  return NULL;
		}
	      strcpy (new_categories[i], env);
	    }
	}
      else
	{
	  env = __get_locale_env (category);
	  if (strlen (env) > ENCODING_LEN)
	    {
	      _REENT_ERRNO(p) = EINVAL;
	      return NULL;
	    }
	  strcpy (new_categories[category], env);
	}
    }
  else if (category != LC_ALL)
    {
      if (strlen (locale) > ENCODING_LEN)
	{
	  _REENT_ERRNO(p) = EINVAL;
	  return NULL;
	}
      strcpy (new_categories[category], locale);
    }
  else
    {
      if ((r = strchr (locale, '/')) == NULL)
	{
	  if (strlen (locale) > ENCODING_LEN)
	    {
	      _REENT_ERRNO(p) = EINVAL;
	      return NULL;
	    }
	  for (i = 1; i < _LC_LAST; ++i)
	    strcpy (new_categories[i], locale);
	}
      else
	{
	  for (i = 1; r[1] == '/'; ++r)
	    ;
	  if (!r[1])
	    {
	      _REENT_ERRNO(p) = EINVAL;
	      return NULL;  /* Hmm, just slashes... */
	    }
	  do
	    {
	      if (i == _LC_LAST)
		break;  /* Too many slashes... */
	      if ((len = r - locale) > ENCODING_LEN)
		{
		  _REENT_ERRNO(p) = EINVAL;
		  return NULL;
		}
	      strlcpy (new_categories[i], locale, len + 1);
	      i++;
	      while (*r == '/')
		r++;
	      locale = r;
	      while (*r && *r != '/')
		r++;
	    }
	  while (*locale);
	  while (i < _LC_LAST)
	    {
	      strcpy (new_categories[i], new_categories[i-1]);
	      i++;
	    }
	}
    }

  if (category != LC_ALL)
    {
      ret = __loadlocale (__get_global_locale (), category,
			  new_categories[category]);
      currentlocale ();
      return ret;
    }

  for (i = 1; i < _LC_LAST; ++i)
    {
      strcpy (saved_categories[i], __get_global_locale ()->categories[i]);
      if (__loadlocale (__get_global_locale (), i, new_categories[i]) == NULL)
	{
	  saverr = _REENT_ERRNO(p);
	  for (j = 1; j < i; j++)
	    {
	      strcpy (new_categories[j], saved_categories[j]);
	      if (__loadlocale (__get_global_locale (), j, new_categories[j])
		  == NULL)
		{
		  strcpy (new_categories[j], "C");
		  __loadlocale (__get_global_locale (), j, new_categories[j]);
		}
	    }
	  _REENT_ERRNO(p) = saverr;
	  return NULL;
	}
    }
  return currentlocale ();
#endif /* _MB_CAPABLE */
}

#ifdef _MB_CAPABLE
static char *
currentlocale (void)
{
  int i;

  strcpy (global_locale_string, __get_global_locale ()->categories[1]);

  for (i = 2; i < _LC_LAST; ++i)
    if (strcmp (__get_global_locale ()->categories[1],
		__get_global_locale ()->categories[i]))
      {
	for (i = 2; i < _LC_LAST; ++i)
	  {
	    (void)strcat(global_locale_string, "/");
	    (void)strcat(global_locale_string,
			 __get_global_locale ()->categories[i]);
	  }
	break;
      }
  return global_locale_string;
}

extern void __set_ctype (struct __locale_t *, const char *charset);

char *
__loadlocale (struct __locale_t *loc, int category, char *new_locale)
{
  /* At this point a full-featured system would just load the locale
     specific data from the locale files.
     What we do here for now is to check the incoming string for correctness.
     The string must be in one of the allowed locale strings, either
     one in POSIX-style, or one in the old newlib style to maintain
     backward compatibility.  If the local string is correct, the charset
     is extracted and stored in ctype_codeset or message_charset
     dependent on the cateogry. */
  char *locale = NULL;
  char charset[ENCODING_LEN + 1] = {};
  long val = 0;
  char *end, *c = NULL;
  int mbc_max;
  wctomb_p l_wctomb;
  mbtowc_p l_mbtowc;
  int cjksingle = 0;
  int cjknarrow = 0;
  int cjkwide = 0;

  /* Avoid doing everything twice if nothing has changed.

     duplocale relies on this test to go wrong so the locale is actually
     duplicated when required.  Any change here has to be synced with a
     matching change in duplocale. */
  if (!strcmp (new_locale, loc->categories[category]))
    return loc->categories[category];

  int ret = 0;

#ifdef __CYGWIN__
  /* This additional code handles the case that the incoming locale string
     is not valid.  If so, it calls the function __set_locale_from_locale_alias,
     which is only available on Cygwin right now.  The function reads the
     file /usr/share/locale/locale.alias.  The file contains locale aliases
     and their replacement locale.  For instance, the alias "french" is
     translated to "fr_FR.ISO-8859-1", the alias "thai" is translated to
     "th_TH.TIS-620".  If successful, the function returns with a pointer
     to the second argument, which is a buffer in which the replacement locale
     gets stored.  Otherwise the function returns NULL. */
  char tmp_locale[ENCODING_LEN + 1];

restart:
  if (!locale)
    locale = new_locale;
  else if (locale != tmp_locale)
    {
      locale = __set_locale_from_locale_alias (locale, tmp_locale);
      if (!locale)
	return NULL;
    }
# define FAIL	goto restart
#else
  locale = new_locale;
# define FAIL	return NULL
#endif

  /* "POSIX" is translated to "C", as on Linux. */
  if (!strcmp (locale, "POSIX"))
    strcpy (locale, "C");
  if (!strcmp (locale, "C"))				/* Default "C" locale */
    strcpy (charset, "ASCII");
  else if (locale[0] == 'C'
	   && (locale[1] == '-'		/* Old newlib style */
	       || locale[1] == '.'))	/* Extension for the C locale to allow
					   specifying different charsets while
					   sticking to the C locale in terms
					   of sort order, etc.  Proposed in
					   the Debian project. */
    {
      char *chp;

      c = locale + 2;
      strcpy (charset, c);
      if ((chp = strchr (charset, '@')))
        /* Strip off modifier */
        *chp = '\0';
      c += strlen (charset);
    }
  else							/* POSIX style */
    {
      c = locale;

      /* Don't use ctype macros here, they might be localized. */
      /* Language */
      if (c[0] < 'a' || c[0] > 'z'
	  || c[1] < 'a' || c[1] > 'z')
	FAIL;
      c += 2;
      /* Allow three character Language per ISO 639-3 */
      if (c[0] >= 'a' && c[0] <= 'z')
      	++c;
      if (c[0] == '_')
        {
	  /* Territory */
	  ++c;
	  if (c[0] < 'A' || c[0] > 'Z'
	      || c[1] < 'A' || c[1] > 'Z')
	    FAIL;
	  c += 2;
	}
      if (c[0] == '.')
	{
	  /* Charset */
	  char *chp;

	  ++c;
	  strcpy (charset, c);
	  if ((chp = strchr (charset, '@')))
	    /* Strip off modifier */
	    *chp = '\0';
	  c += strlen (charset);
	}
      else if (c[0] == '\0' || c[0] == '@')
	/* End of string or just a modifier */
#ifdef __CYGWIN__
	/* The Cygwin-only function __set_charset_from_locale checks
	   for the default charset which is connected to the given locale.
	   The function uses Windows functions in turn so it can't be easily
	   adapted to other targets.  However, if any other target provides
	   equivalent functionality, preferrably using the same function name
	   it would be sufficient to change the guarding #ifdef. */
	__set_charset_from_locale (locale, charset);
#else
	strcpy (charset, "ISO-8859-1");
#endif
      else
	/* Invalid string */
      	FAIL;
    }
  if (c && c[0] == '@')
    {
      /* Modifier "cjksingle" is recognized to enforce single-width mode. */
      /* Modifiers "cjknarrow" or "cjkwide" are recognized to modify the
         behaviour of wcwidth() and wcswidth() for East Asian languages.
         For details see the comment at the end of this function. */
      if (!strcmp (c + 1, "cjksingle"))
	cjksingle = 1;
      else if (!strcmp (c + 1, "cjknarrow"))
	cjknarrow = 1;
      else if (!strcmp (c + 1, "cjkwide"))
	cjkwide = 1;
    }
  /* We only support this subset of charsets. */
  switch (charset[0])
    {
    case 'U':
    case 'u':
      if (strcasecmp (charset, "UTF-8") && strcasecmp (charset, "UTF8"))
	FAIL;
      strcpy (charset, "UTF-8");
      mbc_max = 6;
      l_wctomb = __utf8_wctomb;
      l_mbtowc = __utf8_mbtowc;
    break;
#ifndef __CYGWIN__
    /* Cygwin does not support JIS at all. */
    case 'J':
    case 'j':
      if (strcasecmp (charset, "JIS"))
	FAIL;
      strcpy (charset, "JIS");
      mbc_max = 8;
      l_wctomb = __jis_wctomb;
      l_mbtowc = __jis_mbtowc;
    break;
#endif /* !__CYGWIN__ */
    case 'E':
    case 'e':
      if (strncasecmp (charset, "EUC", 3))
	FAIL;
      c = charset + 3;
      if (*c == '-')
	++c;
      if (!strcasecmp (c, "JP"))
	{
	  strcpy (charset, "EUCJP");
	  mbc_max = 3;
	  l_wctomb = __eucjp_wctomb;
	  l_mbtowc = __eucjp_mbtowc;
	}
#ifdef __CYGWIN__
      /* Newlib does neither provide EUC-KR nor EUC-CN, and Cygwin's
	 implementation requires Windows support. */
      else if (!strcasecmp (c, "KR"))
	{
	  strcpy (charset, "EUCKR");
	  mbc_max = 2;
	  l_wctomb = __kr_wctomb;
	  l_mbtowc = __kr_mbtowc;
	}
      else if (!strcasecmp (c, "CN"))
	{
	  strcpy (charset, "EUCCN");
	  mbc_max = 2;
	  l_wctomb = __gbk_wctomb;
	  l_mbtowc = __gbk_mbtowc;
	}
#endif /* __CYGWIN__ */
      else
	FAIL;
    break;
    case 'S':
    case 's':
      if (strcasecmp (charset, "SJIS"))
	FAIL;
      strcpy (charset, "SJIS");
      mbc_max = 2;
      l_wctomb = __sjis_wctomb;
      l_mbtowc = __sjis_mbtowc;
    break;
    case 'I':
    case 'i':
      /* Must be exactly one of ISO-8859-1, [...] ISO-8859-16, except for
         ISO-8859-12.  This code also recognizes the aliases without dashes. */
      if (strncasecmp (charset, "ISO", 3))
	FAIL;
      c = charset + 3;
      if (*c == '-')
	++c;
      if (strncasecmp (c, "8859", 4))
	FAIL;
      c += 4;
      if (*c == '-')
	++c;
      val = strtol (c, &end, 10);
      if (val < 1 || val > 16 || val == 12 || *end)
	FAIL;
      strcpy (charset, "ISO-8859-");
      c = charset + 9;
      if (val > 10)
      	*c++ = '1';
      *c++ = val % 10 + '0';
      *c = '\0';
      mbc_max = 1;
#ifdef _MB_EXTENDED_CHARSETS_ISO
      l_wctomb = __iso_wctomb (val);
      l_mbtowc = __iso_mbtowc (val);
#else /* !_MB_EXTENDED_CHARSETS_ISO */
      l_wctomb = __ascii_wctomb;
      l_mbtowc = __ascii_mbtowc;
#endif /* _MB_EXTENDED_CHARSETS_ISO */
    break;
    case 'C':
    case 'c':
      if (charset[1] != 'P' && charset[1] != 'p')
	FAIL;
      memcpy (charset, "CP", 2);
      val = strtol (charset + 2, &end, 10);
      if (*end)
	FAIL;
      switch (val)
	{
	case 437:
	case 720:
	case 737:
	case 775:
	case 850:
	case 852:
	case 855:
	case 857:
	case 858:
	case 862:
	case 866:
	case 874:
	case 1125:
	case 1250:
	case 1251:
	case 1252:
	case 1253:
	case 1254:
	case 1255:
	case 1256:
	case 1257:
	case 1258:
	  mbc_max = 1;
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
	  l_wctomb = __cp_wctomb (val);
	  l_mbtowc = __cp_mbtowc (val);
#else /* !_MB_EXTENDED_CHARSETS_WINDOWS */
	  l_wctomb = __ascii_wctomb;
	  l_mbtowc = __ascii_mbtowc;
#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */
	  break;
	case 932:
	  mbc_max = 2;
	  l_wctomb = __sjis_wctomb;
	  l_mbtowc = __sjis_mbtowc;
	  break;
	default:
	  FAIL;
	}
    break;
    case 'K':
    case 'k':
      /* KOI8-R, KOI8-U, KOI8-T and the aliases without dash */
      if (strncasecmp (charset, "KOI8", 4))
	FAIL;
      c = charset + 4;
      if (*c == '-')
	++c;
      if (*c == 'R' || *c == 'r')
	{
	  val = 20866;
	  strcpy (charset, "CP20866");
	}
      else if (*c == 'U' || *c == 'u')
	{
	  val = 21866;
	  strcpy (charset, "CP21866");
	}
      else if (*c == 'T' || *c == 't')
	{
	  val = 103;
	  strcpy (charset, "CP103");
	}
      else
	FAIL;
      mbc_max = 1;
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
      l_wctomb = __cp_wctomb (val);
      l_mbtowc = __cp_mbtowc (val);
#else /* !_MB_EXTENDED_CHARSETS_WINDOWS */
      l_wctomb = __ascii_wctomb;
      l_mbtowc = __ascii_mbtowc;
#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */
      break;
    case 'A':
    case 'a':
      if (strcasecmp (charset, "ASCII"))
	FAIL;
      strcpy (charset, "ASCII");
      mbc_max = 1;
      l_wctomb = __ascii_wctomb;
      l_mbtowc = __ascii_mbtowc;
      break;
    case 'G':
    case 'g':
#ifdef __CYGWIN__
      /* Newlib does not provide GBK/GB2312 and Cygwin's implementation
	 requires Windows support. */
      if (!strcasecmp (charset, "GBK")
	  || !strcasecmp (charset, "GB2312"))
	{
	  strcpy (charset, charset[2] == '2' ? "GB2312" : "GBK");
	  mbc_max = 2;
	  l_wctomb = __gbk_wctomb;
	  l_mbtowc = __gbk_mbtowc;
	}
      else if (!strcasecmp (charset, "GB18030"))
	{
	  strcpy (charset, "GB18030");
	  mbc_max = 4;
	  l_wctomb = __gb18030_wctomb;
	  l_mbtowc = __gb18030_mbtowc;
	}
      else
#endif /* __CYGWIN__ */
      /* GEORGIAN-PS and the alias without dash */
      if (!strncasecmp (charset, "GEORGIAN", 8))
	{
	  c = charset + 8;
	  if (*c == '-')
	    ++c;
	  if (strcasecmp (c, "PS"))
	    FAIL;
	  val = 101;
	  strcpy (charset, "CP101");
	  mbc_max = 1;
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
	  l_wctomb = __cp_wctomb (val);
	  l_mbtowc = __cp_mbtowc (val);
#else /* !_MB_EXTENDED_CHARSETS_WINDOWS */
	  l_wctomb = __ascii_wctomb;
	  l_mbtowc = __ascii_mbtowc;
#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */
	}
      else
	FAIL;
      break;
    case 'P':
    case 'p':
      /* PT154 */
      if (strcasecmp (charset, "PT154"))
	FAIL;
      val = 102;
      strcpy (charset, "CP102");
      mbc_max = 1;
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
      l_wctomb = __cp_wctomb (val);
      l_mbtowc = __cp_mbtowc (val);
#else /* !_MB_EXTENDED_CHARSETS_WINDOWS */
      l_wctomb = __ascii_wctomb;
      l_mbtowc = __ascii_mbtowc;
#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */
      break;
    case 'T':
    case 't':
      if (strncasecmp (charset, "TIS", 3))
      	FAIL;
      c = charset + 3;
      if (*c == '-')
	++c;
      if (strcmp (c, "620"))
      	FAIL;
      val = 874;
      strcpy (charset, "CP874");
      mbc_max = 1;
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
      l_wctomb = __cp_wctomb (val);
      l_mbtowc = __cp_mbtowc (val);
#else /* !_MB_EXTENDED_CHARSETS_WINDOWS */
      l_wctomb = __ascii_wctomb;
      l_mbtowc = __ascii_mbtowc;
#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */
      break;
#ifdef __CYGWIN__
    /* Newlib does not provide Big5 and Cygwin's implementation
       requires Windows support. */
    case 'B':
    case 'b':
      if (strcasecmp (charset, "BIG5"))
      	FAIL;
      strcpy (charset, "BIG5");
      mbc_max = 2;
      l_wctomb = __big5_wctomb;
      l_mbtowc = __big5_mbtowc;
      break;
#endif /* __CYGWIN__ */
    default:
      FAIL;
    }
  switch (category)
    {
    case LC_CTYPE:
#ifndef __HAVE_LOCALE_INFO__
      strcpy (loc->ctype_codeset, charset);
      loc->mb_cur_max[0] = mbc_max;
#endif
#ifdef __CYGWIN__
      __mb_cur_max = mbc_max;	/* Only for backward compat */
#endif
      loc->wctomb = l_wctomb;
      loc->mbtowc = l_mbtowc;
      __set_ctype (loc, charset);
      /* Set CJK width mode (1: ambiguous-wide, 0: normal, -1: disabled). */
      /* Determine the width for the "CJK Ambiguous Width" category of
         characters. This is used in wcwidth(). Assume single width for
         single-byte charsets, and double width for multi-byte charsets
         other than UTF-8. For UTF-8, use single width.
         Single width can also be forced with the "@cjknarrow" modifier.
         Double width can also be forced with the "@cjkwide" modifier.
       */
      loc->cjk_lang = cjkwide ||
		      (!cjknarrow && mbc_max > 1 && charset[0] != 'U');
      if (cjksingle)
	loc->cjk_lang = -1;	/* Disable CJK dual-width */
#ifdef __CYGWIN__
      ret = __ctype_load_locale (loc, locale, (void *) l_wctomb, charset,
				 mbc_max);
#endif /* __CYGWIN__ */
      break;
#ifdef __CYGWIN__
  /* Right now only Cygwin supports a __messages_load_locale function at all. */
    case LC_MESSAGES:
#ifdef __HAVE_LOCALE_INFO__
      ret = __messages_load_locale (loc, locale, (void *) l_wctomb, charset);
      if (!ret)
#else
      strcpy (loc->message_codeset, charset);
#endif /* __HAVE_LOCALE_INFO__ */
      break;
#endif
#ifdef __HAVE_LOCALE_INFO__
#ifdef __CYGWIN__
  /* Right now only Cygwin supports a __collate_load_locale function at all. */
    case LC_COLLATE:
      ret = __collate_load_locale (loc, locale, (void *) l_mbtowc, charset);
      break;
  /* Right now only Cygwin supports a __monetary_load_locale function at all. */
    case LC_MONETARY:
      ret = __monetary_load_locale (loc, locale, (void *) l_wctomb, charset);
      break;
  /* Right now only Cygwin supports a __numeric_load_locale function at all. */
    case LC_NUMERIC:
      ret = __numeric_load_locale (loc, locale, (void *) l_wctomb, charset);
      break;
  /* Right now only Cygwin supports a __time_load_locale function at all. */
    case LC_TIME:
      ret = __time_load_locale (loc, locale, (void *) l_wctomb, charset);
      break;
#endif
#endif /* __HAVE_LOCALE_INFO__ */
    default:
      break;
    }
#ifdef __HAVE_LOCALE_INFO__
  if (ret)
    FAIL;
#endif /* __HAVE_LOCALE_INFO__ */
  return strcpy(loc->categories[category], new_locale);
}

const char *
__get_locale_env (int category)
{
  const char *env;

  /* 1. check LC_ALL. */
  env = getenv (categories[0]);

  /* 2. check LC_* */
  if (env == NULL || !*env)
    env = getenv (categories[category]);

  /* 3. check LANG */
  if (env == NULL || !*env)
    env = getenv ("LANG");

  /* 4. if none is set, fall to default locale */
  if (env == NULL || !*env)
    env = __default_locale;

  return env;
}
#endif /* _MB_CAPABLE */

size_t
__locale_mb_cur_max (void)
{
#ifdef __HAVE_LOCALE_INFO__
  return __get_current_ctype_locale ()->mb_cur_max[0];
#else
  return __get_current_locale ()->mb_cur_max[0];
#endif
}

#ifdef __HAVE_LOCALE_INFO__
const char *
__locale_ctype_ptr_l (struct __locale_t *locale)
{
  return locale->ctype_ptr;
}

const char *
__locale_ctype_ptr (void)
{
  return __get_current_locale ()->ctype_ptr;
}
#endif /* __HAVE_LOCALE_INFO__ */
