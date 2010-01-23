/*
FUNCTION
<<setlocale>>, <<localeconv>>---select or query locale

INDEX
	setlocale
INDEX
	localeconv
INDEX
	_setlocale_r
INDEX
	_localeconv_r

ANSI_SYNOPSIS
	#include <locale.h>
	char *setlocale(int <[category]>, const char *<[locale]>);
	lconv *localeconv(void);

	char *_setlocale_r(void *<[reent]>,
                        int <[category]>, const char *<[locale]>);
	lconv *_localeconv_r(void *<[reent]>);

TRAD_SYNOPSIS
	#include <locale.h>
	char *setlocale(<[category]>, <[locale]>)
	int <[category]>;
	char *<[locale]>;

	lconv *localeconv();

	char *_setlocale_r(<[reent]>, <[category]>, <[locale]>)
	char *<[reent]>;
	int <[category]>;
	char *<[locale]>;

	lconv *_localeconv_r(<[reent]>);
	char *<[reent]>;

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

Additionally to the POSIX specifier, seven extensions are supported for
backward compatibility with older implementations using newlib:
<<"C-UTF-8">>, <<"C-JIS">>, <<"C-eucJP">>, <<"C-SJIS">>, <<C-KOI8-R>>,
<<C-KOI8-U>>, <<"C-ISO-8859-x">> with 1 <= x <= 15, or <<"C-CPxxx">> with
xxx in [437, 720, 737, 775, 850, 852, 855, 857, 858, 862, 866, 874, 1125,
1250, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258].

Instead of <<"C-">>, you can specify also <<"C.">>.  Both variations allow
to specify language neutral locales while using other charsets than ASCII,
for instance <<"C.UTF-8">>, which keeps all settings as in the C locale,
but uses the UTF-8 charset.

Even when using POSIX locale strings, the only charsets allowed are
<<"UTF-8">>, <<"JIS">>, <<"EUCJP">>, <<"SJIS">>, <<KOI8-R>>, <<KOI8-U>>,
<<"ISO-8859-x">> with 1 <= x <= 15, or <<"CPxxx">> with xxx in
[437, 720, 737, 775, 850, 852, 855, 857, 858, 862, 866, 874, 1125, 1250,
1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258].
Charsets are case insensitive.  For instance, <<"EUCJP">> and <<"eucJP">>
are equivalent.  <<"UTF-8">> can also be written without dash, as in
<<"UTF8">> or <<"utf8">>.  <<"EUCJP">> and <<"EUCKR"> can also contain a
dash, <<"EUC-JP">> and <<"EUC-KR">>.


(<<"">> is also accepted; if given, the settings are read from the
corresponding LC_* environment variables and $LANG according to POSIX rules.

Under Cygwin, this implementation additionally supports the charsets
<<"GBK">>, <<"eucKR">>, <<"Big5">>, and <<"TIS-620">>.

This implementation also supports a single modifier, <<"cjknarrow">>.
Any other modifier is ignored.  <<"cjknarrow">>, in conjunction with one
of the language specifiers <<"ja">>, <<"ko">>, and <<"zh">> specifies
how the functions <<wcwidth>> and <<wcswidth>> handle characters from
the "CJK Ambiguous Width" character class described in
http://www.unicode.org/unicode/reports/tr11/.  Usually these characters
have a width of 1, unless you specify one of the aforementioned
languages, in which case these characters have a width of 2.  By
specifying the <<"cjknarrow">> modifier, these characters will have a
width of one in the languages <<"ja">>, <<"ko">>, and <<"zh">> as well.

If you use <<NULL>> as the <[locale]> argument, <<setlocale>> returns a
pointer to the string representing the current locale.  The acceptable
values for <[category]> are defined in `<<locale.h>>' as macros
beginning with <<"LC_">>.

<<localeconv>> returns a pointer to a structure (also defined in
`<<locale.h>>') describing the locale-specific conventions currently
in effect.  

<<_localeconv_r>> and <<_setlocale_r>> are reentrant versions of
<<localeconv>> and <<setlocale>> respectively.  The extra argument
<[reent]> is a pointer to a reentrancy structure.

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

#include <newlib.h>
#include <errno.h>
#include <locale.h>
#include <string.h>
#include <limits.h>
#include <reent.h>
#include <stdlib.h>
#include <wchar.h>
#include "lmonetary.h"
#include "lnumeric.h"
#include "../stdlib/local.h"

#define _LC_LAST      7
#define ENCODING_LEN 31

int __EXPORT __mb_cur_max = 1;

int __nlocale_changed = 0;
int __mlocale_changed = 0;
char *_PathLocale = NULL;

static
#ifndef __CYGWIN__
_CONST
#endif
struct lconv lconv = 
{
  ".", "", "", "", "", "", "", "", "", "",
  CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
  CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
  CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
  CHAR_MAX, CHAR_MAX
};

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

/*
 * Default locale per POSIX.  Can be overridden on a per-target base.
 */
#ifndef DEFAULT_LOCALE
#define DEFAULT_LOCALE	"C"
#endif
/*
 * This variable can be changed by any outside mechanism.  This allows,
 * for instance, to load the default locale from a file.
 */
char __default_locale[ENCODING_LEN + 1] = DEFAULT_LOCALE;

/*
 * Current locales for each category
 */
static char current_categories[_LC_LAST][ENCODING_LEN + 1] = {
    "C",
    "C",
    "C",
    "C",
    "C",
    "C",
    "C",
};

/*
 * The locales we are going to try and load
 */
static char new_categories[_LC_LAST][ENCODING_LEN + 1];
static char saved_categories[_LC_LAST][ENCODING_LEN + 1];

static char current_locale_string[_LC_LAST * (ENCODING_LEN + 1/*"/"*/ + 1)];
static char *currentlocale(void);
static char *loadlocale(struct _reent *, int);
static const char *__get_locale_env(struct _reent *, int);

#endif

#if 0 /*def __CYGWIN__  TODO: temporarily(?) disable C == UTF-8 */
static char lc_ctype_charset[ENCODING_LEN + 1] = "UTF-8";
static char lc_message_charset[ENCODING_LEN + 1] = "UTF-8";
#else
static char lc_ctype_charset[ENCODING_LEN + 1] = "ASCII";
static char lc_message_charset[ENCODING_LEN + 1] = "ASCII";
#endif
static int lc_ctype_cjk_lang = 0;

char *
_DEFUN(_setlocale_r, (p, category, locale),
       struct _reent *p _AND
       int category _AND
       _CONST char *locale)
{
#ifndef _MB_CAPABLE
  if (locale)
    { 
      if (strcmp (locale, "POSIX") && strcmp (locale, "C")
	  && strcmp (locale, ""))
        return NULL;
    }
  return "C";
#else
  int i, j, len, saverr;
  const char *env, *r;

  if (category < LC_ALL || category >= _LC_LAST)
    {
      p->_errno = EINVAL;
      return NULL;
    }

  if (locale == NULL)
    return category != LC_ALL ? current_categories[category] : currentlocale();

  /*
   * Default to the current locale for everything.
   */
  for (i = 1; i < _LC_LAST; ++i)
    strcpy (new_categories[i], current_categories[i]);

  /*
   * Now go fill up new_categories from the locale argument
   */
  if (!*locale)
    {
      if (category == LC_ALL)
	{
	  for (i = 1; i < _LC_LAST; ++i)
	    {
	      env = __get_locale_env (p, i);
	      if (strlen (env) > ENCODING_LEN)
		{
		  p->_errno = EINVAL;
		  return NULL;
		}
	      strcpy (new_categories[i], env);
	    }
	}
      else
	{
	  env = __get_locale_env (p, category);
	  if (strlen (env) > ENCODING_LEN)
	    {
	      p->_errno = EINVAL;
	      return NULL;
	    }
	  strcpy (new_categories[category], env);
	}
    }
  else if (category != LC_ALL)
    {
      if (strlen (locale) > ENCODING_LEN)
	{
	  p->_errno = EINVAL;
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
	      p->_errno = EINVAL;
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
	      p->_errno = EINVAL;
	      return NULL;  /* Hmm, just slashes... */
	    }
	  do
	    {
	      if (i == _LC_LAST)
		break;  /* Too many slashes... */
	      if ((len = r - locale) > ENCODING_LEN)
		{
		  p->_errno = EINVAL;
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
    return loadlocale (p, category);

  for (i = 1; i < _LC_LAST; ++i)
    {
      strcpy (saved_categories[i], current_categories[i]);
      if (loadlocale (p, i) == NULL)
	{
	  saverr = p->_errno;
	  for (j = 1; j < i; j++)
	    {
	      strcpy (new_categories[j], saved_categories[j]);
	      if (loadlocale (p, j) == NULL)
		{
		  strcpy (new_categories[j], "C");
		  loadlocale (p, j);
		}
	    }
	  p->_errno = saverr;
	  return NULL;
	}
    }
  return currentlocale ();
#endif
}

#ifdef _MB_CAPABLE
static char *
currentlocale()
{
        int i;

        (void)strcpy(current_locale_string, current_categories[1]);

        for (i = 2; i < _LC_LAST; ++i)
                if (strcmp(current_categories[1], current_categories[i])) {
                        for (i = 2; i < _LC_LAST; ++i) {
                                (void)strcat(current_locale_string, "/");
                                (void)strcat(current_locale_string,
                                             current_categories[i]);
                        }
                        break;
                }
        return (current_locale_string);
}
#endif

#ifdef _MB_CAPABLE
#ifdef __CYGWIN__
extern void __set_charset_from_locale (const char *locale, char *charset);
extern int __collate_load_locale (const char *, void *, const char *);
#endif /* __CYGWIN__ */

extern void __set_ctype (const char *charset);

static char *
loadlocale(struct _reent *p, int category)
{
  /* At this point a full-featured system would just load the locale
     specific data from the locale files.
     What we do here for now is to check the incoming string for correctness.
     The string must be in one of the allowed locale strings, either
     one in POSIX-style, or one in the old newlib style to maintain
     backward compatibility.  If the local string is correct, the charset
     is extracted and stored in lc_ctype_charset or lc_message_charset
     dependent on the cateogry. */
  char *locale = new_categories[category];
  char charset[ENCODING_LEN + 1];
  unsigned long val;
  char *end;
  int mbc_max;
  int (*l_wctomb) (struct _reent *, char *, wchar_t, const char *, mbstate_t *);
  int (*l_mbtowc) (struct _reent *, wchar_t *, const char *, size_t,
		   const char *, mbstate_t *);
#ifdef _MB_CAPABLE
  int cjknarrow = 0;
#endif
#ifdef __CYGWIN__
  int ret = 0;
#endif
  
  /* "POSIX" is translated to "C", as on Linux. */
  if (!strcmp (locale, "POSIX"))
    strcpy (locale, "C");
  if (!strcmp (locale, "C"))				/* Default "C" locale */
#if 0 /*def __CYGWIN__  TODO: temporarily(?) disable C == UTF-8 */
    strcpy (charset, "UTF-8");
#else
    strcpy (charset, "ASCII");
#endif
  else if (locale[0] == 'C'
	   && (locale[1] == '-'		/* Old newlib style */
	       || locale[1] == '.'))	/* Extension for the C locale to allow
					   specifying different charsets while
					   sticking to the C locale in terms
					   of sort order, etc.  Proposed in
					   the Debian project. */
    strcpy (charset, locale + 2);
  else							/* POSIX style */
    {
      char *c = locale;

      /* Don't use ctype macros here, they might be localized. */
      /* Language */
      if (c[0] < 'a' || c[0] > 'z'
	  || c[1] < 'a' || c[1] > 'z')
	return NULL;
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
	    return NULL;
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
	__set_charset_from_locale (locale, charset);
#else
	strcpy (charset, "ISO-8859-1");
#endif
      else
	/* Invalid string */
      	return NULL;
#ifdef _MB_CAPABLE
      if (c[0] == '@')
	{
	  /* Modifier */
	  /* Only one modifier is recognized right now.  "cjknarrow" is used
	     to modify the behaviour of wcwidth() for East Asian languages.
	     For details see the comment at the end of this function. */
	  if (!strcmp (c + 1, "cjknarrow"))
	    cjknarrow = 1;
	}
#endif
    }
  /* We only support this subset of charsets. */
  switch (charset[0])
    {
    case 'U':
    case 'u':
      if (strcasecmp (charset, "UTF-8") && strcasecmp (charset, "UTF8"))
	return NULL;
      strcpy (charset, "UTF-8");
      mbc_max = 6;
#ifdef _MB_CAPABLE
      l_wctomb = __utf8_wctomb;
      l_mbtowc = __utf8_mbtowc;
#endif
    break;
#ifndef __CYGWIN__
    case 'J':
    case 'j':
      if (strcasecmp (charset, "JIS"))
	return NULL;
      strcpy (charset, "JIS");
      mbc_max = 8;
#ifdef _MB_CAPABLE
      l_wctomb = __jis_wctomb;
      l_mbtowc = __jis_mbtowc;
#endif
    break;
#endif
    case 'E':
    case 'e':
      if (!strcasecmp (charset, "EUCJP") || !strcasecmp (charset, "EUC-JP"))
	{
	  strcpy (charset, "EUCJP");
	  mbc_max = 3;
#ifdef _MB_CAPABLE
	  l_wctomb = __eucjp_wctomb;
	  l_mbtowc = __eucjp_mbtowc;
#endif
	}
#ifdef __CYGWIN__
      else if (!strcasecmp (charset, "EUCKR")
	       || !strcasecmp (charset, "EUC-KR"))
	{
	  strcpy (charset, "EUCKR");
	  mbc_max = 2;
#ifdef _MB_CAPABLE
	  l_wctomb = __kr_wctomb;
	  l_mbtowc = __kr_mbtowc;
#endif
	}
#endif
      else
	return NULL;
    break;
    case 'S':
    case 's':
      if (strcasecmp (charset, "SJIS"))
	return NULL;
      strcpy (charset, "SJIS");
      mbc_max = 2;
#ifdef _MB_CAPABLE
      l_wctomb = __sjis_wctomb;
      l_mbtowc = __sjis_mbtowc;
#endif
    break;
    case 'I':
    case 'i':
      /* Must be exactly one of ISO-8859-1, [...] ISO-8859-16, except for
         ISO-8859-12. */
      if (strncasecmp (charset, "ISO-8859-", 9))
	return NULL;
      strncpy (charset, "ISO", 3);
      val = _strtol_r (p, charset + 9, &end, 10);
      if (val < 1 || val > 16 || val == 12 || *end)
	return NULL;
      mbc_max = 1;
#ifdef _MB_CAPABLE
#ifdef _MB_EXTENDED_CHARSETS_ISO
      l_wctomb = __iso_wctomb;
      l_mbtowc = __iso_mbtowc;
#else /* !_MB_EXTENDED_CHARSETS_ISO */
      l_wctomb = __ascii_wctomb;
      l_mbtowc = __ascii_mbtowc;
#endif /* _MB_EXTENDED_CHARSETS_ISO */
#endif
    break;
    case 'C':
    case 'c':
      if (charset[1] != 'P' && charset[1] != 'p')
	return NULL;
      strncpy (charset, "CP", 2);
      val = _strtol_r (p, charset + 2, &end, 10);
      if (*end)
	return NULL;
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
#ifdef _MB_CAPABLE
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
	  l_wctomb = __cp_wctomb;
	  l_mbtowc = __cp_mbtowc;
#else /* !_MB_EXTENDED_CHARSETS_WINDOWS */
	  l_wctomb = __ascii_wctomb;
	  l_mbtowc = __ascii_mbtowc;
#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */
#endif
	  break;
	default:
	  return NULL;
	}
    break;
    case 'K':
    case 'k':
      if (!strcasecmp (charset, "KOI8-R"))
	strcpy (charset, "CP20866");
      else if (!strcasecmp (charset, "KOI8-U"))
	strcpy (charset, "CP21866");
      else
	return NULL;
      mbc_max = 1;
#ifdef _MB_CAPABLE
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
      l_wctomb = __cp_wctomb;
      l_mbtowc = __cp_mbtowc;
#else /* !_MB_EXTENDED_CHARSETS_WINDOWS */
      l_wctomb = __ascii_wctomb;
      l_mbtowc = __ascii_mbtowc;
#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */
#endif
      break;
    case 'A':
    case 'a':
      if (strcasecmp (charset, "ASCII"))
	return NULL;
      strcpy (charset, "ASCII");
      mbc_max = 1;
#ifdef _MB_CAPABLE
      l_wctomb = __ascii_wctomb;
      l_mbtowc = __ascii_mbtowc;
#endif
      break;
#ifdef __CYGWIN__
    case 'G':
    case 'g':
      if (strcasecmp (charset, "GBK"))
      	return NULL;
      strcpy (charset, "GBK");
      mbc_max = 2;
#ifdef _MB_CAPABLE
      l_wctomb = __gbk_wctomb;
      l_mbtowc = __gbk_mbtowc;
#endif
      break;
    case 'B':
    case 'b':
      if (strcasecmp (charset, "BIG5"))
      	return NULL;
      strcpy (charset, "BIG5");
      mbc_max = 2;
#ifdef _MB_CAPABLE
      l_wctomb = __big5_wctomb;
      l_mbtowc = __big5_mbtowc;
#endif
      break;
    case 'T':
    case 't':
      if (strcasecmp (charset, "TIS620") && strcasecmp (charset, "TIS-620"))
      	return NULL;
      strcpy (charset, "CP874");
      mbc_max = 1;
#ifdef _MB_CAPABLE
      l_wctomb = __cp_wctomb;
      l_mbtowc = __cp_mbtowc;
#endif
      break;
#endif /* __CYGWIN__ */
    default:
      return NULL;
    }
  if (category == LC_CTYPE)
    {
      strcpy (lc_ctype_charset, charset);
      __mb_cur_max = mbc_max;
#ifdef _MB_CAPABLE
      __wctomb = l_wctomb;
      __mbtowc = l_mbtowc;
      __set_ctype (charset);
      /* Check for the language part of the locale specifier.  In case
         of "ja", "ko", or "zh", assume the use of CJK fonts, unless the
	 "@cjknarrow" modifier has been specifed.
	 The result is stored in lc_ctype_cjk_lang and tested in wcwidth()
	 to figure out the width to return (1 or 2) for the "CJK Ambiguous
	 Width" category of characters. */
      lc_ctype_cjk_lang = !cjknarrow
			  && ((strncmp (locale, "ja", 2) == 0
			      || strncmp (locale, "ko", 2) == 0
			      || strncmp (locale, "zh", 2) == 0));
#endif
    }
  else if (category == LC_MESSAGES)
    strcpy (lc_message_charset, charset);
#ifdef __CYGWIN__
  else if (category == LC_COLLATE)
    ret = __collate_load_locale (locale, (void *) l_mbtowc, charset);
  else if (category == LC_MONETARY)
    ret = __monetary_load_locale (locale, (void *) l_wctomb, charset);
  else if (category == LC_NUMERIC)
    ret = __numeric_load_locale (locale, (void *) l_wctomb, charset);
  else if (category == LC_TIME)
    ret = __time_load_locale (locale, (void *) l_wctomb, charset);
  if (ret)
    return NULL;
#endif
  return strcpy(current_categories[category], new_categories[category]);
}

static const char *
__get_locale_env(struct _reent *p, int category)
{
  const char *env;

  /* 1. check LC_ALL. */
  env = _getenv_r (p, categories[0]);

  /* 2. check LC_* */
  if (env == NULL || !*env)
    env = _getenv_r (p, categories[category]);

  /* 3. check LANG */
  if (env == NULL || !*env)
    env = _getenv_r (p, "LANG");

  /* 4. if none is set, fall to default locale */
  if (env == NULL || !*env)
    env = __default_locale;

  return env;
}
#endif

char *
_DEFUN_VOID(__locale_charset)
{
  return lc_ctype_charset;
}

char *
_DEFUN_VOID(__locale_msgcharset)
{
  return lc_message_charset;
}

int
_DEFUN_VOID(__locale_cjk_lang)
{
  return lc_ctype_cjk_lang;
}

struct lconv *
_DEFUN(_localeconv_r, (data), 
      struct _reent *data)
{
#ifdef __CYGWIN__
  if (__nlocale_changed)
    {
      struct lc_numeric_T *n = __get_current_numeric_locale ();
      lconv.decimal_point = n->decimal_point;
      lconv.thousands_sep = n->thousands_sep;
      lconv.grouping = n->grouping;
      __nlocale_changed = 0;
    }
  if (__mlocale_changed)
    {
      struct lc_monetary_T *m = __get_current_monetary_locale ();
      lconv.int_curr_symbol = m->int_curr_symbol;
      lconv.currency_symbol = m->currency_symbol;
      lconv.mon_decimal_point = m->mon_decimal_point;
      lconv.mon_thousands_sep = m->mon_thousands_sep;
      lconv.mon_grouping = m->mon_grouping;
      lconv.positive_sign = m->positive_sign;
      lconv.negative_sign = m->negative_sign;
      lconv.int_frac_digits = m->int_frac_digits[0];
      lconv.frac_digits = m->frac_digits[0];
      lconv.p_cs_precedes = m->p_cs_precedes[0];
      lconv.p_sep_by_space = m->p_sep_by_space[0];
      lconv.n_cs_precedes = m->n_cs_precedes[0];
      lconv.n_sep_by_space = m->n_sep_by_space[0];
      lconv.p_sign_posn = m->p_sign_posn[0];
      lconv.n_sign_posn = m->n_sign_posn[0];
      lconv.int_n_cs_precedes = m->n_cs_precedes[0];
      lconv.int_n_sep_by_space = m->n_sep_by_space[0];
      lconv.int_n_sign_posn = m->n_sign_posn[0];
      lconv.int_p_cs_precedes = m->p_cs_precedes[0];
      lconv.int_p_sep_by_space = m->p_sep_by_space[0];
      lconv.int_p_sign_posn = m->p_sign_posn[0];
      __mlocale_changed = 0;
    }
#endif
  return (struct lconv *) &lconv;
}

#ifndef _REENT_ONLY

#ifndef __CYGWIN__
char *
_DEFUN(setlocale, (category, locale),
       int category _AND
       _CONST char *locale)
{
  return _setlocale_r (_REENT, category, locale);
}
#endif

struct lconv *
_DEFUN_VOID(localeconv)
{
  return _localeconv_r (_REENT);
}

#endif
