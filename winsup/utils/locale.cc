/*
 * Copyright (c) 2010, 2011, 2012, 2013 Corinna Vinschen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <langinfo.h>
#include <limits.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#define _WIN32_WINNT 0x0602
#define WINVER 0x0602
#include <windows.h>

#define LOCALE_ALIAS		"/usr/share/locale/locale.alias"
#define LOCALE_ALIAS_LINE_LEN	255

void
usage ()
{
  printf (
"Usage: %1$s [-amvhV]\n"
"   or: %1$s [-ck] NAME\n"
"   or: %1$s [-usfnU]\n"
"\n"
"Get locale-specific information.\n"
"\n"
"System information:\n"
"\n"
"  -a, --all-locales    List all available supported locales\n"
"  -m, --charmaps       List all available character maps\n"
"  -v, --verbose        More verbose output\n"
"\n"
"Modify output format:\n"
"\n"
"  -c, --category-name  List information about given category NAME\n"
"  -k, --keyword-name   Print information about given keyword NAME\n"
"\n"
"Default locale information:\n"
"\n"
"  -u, --user           Print locale of user's default UI language\n"
"  -s, --system         Print locale of system default UI language\n"
"  -f, --format         Print locale of user's regional format settings\n"
"                       (time, numeric & monetary)\n"
"  -n, --no-unicode     Print system default locale for non-Unicode programs\n"
"  -U, --utf            Attach \".UTF-8\" to the result\n"
"\n"
"Other options:\n"
"\n"
"  -h, --help           This text\n"
"  -V, --version        Print program version and exit\n\n",
  program_invocation_short_name);
}

void
print_version ()
{
  printf ("locale (cygwin) %d.%d.%d\n"
	  "Get locale-specific information\n"
	  "Copyright (C) 2010 - %s Red Hat, Inc.\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

struct option longopts[] = {
  {"all-locales", no_argument, NULL, 'a'},
  {"category-name", no_argument, NULL, 'c'},
  {"format", no_argument, NULL, 'f'},
  {"help", no_argument, NULL, 'h'},
  {"keyword-name", no_argument, NULL, 'k'},
  {"charmaps", no_argument, NULL, 'm'},
  {"no-unicode", no_argument, NULL, 'n'},
  {"system", no_argument, NULL, 's'},
  {"user", no_argument, NULL, 'u'},
  {"utf", no_argument, NULL, 'U'},
  {"verbose", no_argument, NULL, 'v'},
  {"version", no_argument, NULL, 'V'},
  {0, no_argument, NULL, 0}
};
const char *opts = "acfhkmnsuUvV";

int
getlocale (LCID lcid, char *name)
{
  char iso639[10];
  char iso3166[10];

  iso3166[0] = '\0';
  if (!GetLocaleInfo (lcid, LOCALE_SISO639LANGNAME, iso639, 10))
    return 0;
  GetLocaleInfo (lcid, LOCALE_SISO3166CTRYNAME, iso3166, 10);
  sprintf (name, "%s%s%s", iso639, lcid > 0x3ff ? "_" : "",
			   lcid > 0x3ff ? iso3166 : "");
  return 1;
}

typedef struct {
  const char *name;
  const wchar_t *language;
  const wchar_t *territory;
  const char *codeset;
  bool alias;
} loc_t;
loc_t *locale;
size_t loc_max;
size_t loc_num;

void
print_codeset (const char *codeset)
{
  for (; *codeset; ++codeset)
    if (*codeset != '-')
      putc (tolower ((int)(unsigned char) *codeset), stdout);
}

void
print_locale_with_codeset (int verbose, loc_t *locale, bool utf8,
			   const char *modifier)
{
  static const char *sysroot;
  char locname[32];

  if (verbose
      && (!strcmp (locale->name, "C") || !strcmp (locale->name, "POSIX")))
    return;
  if (!sysroot)
    {
      WCHAR sysbuf[PATH_MAX];
      HMODULE k32 = GetModuleHandleW (L"kernel32.dll");
      if (GetModuleFileNameW (k32, sysbuf, PATH_MAX))
	sysroot = (const char *) cygwin_create_path (CCP_WIN_W_TO_POSIX,
						     sysbuf);
      if (!sysroot)
      	sysroot = "kernel32.dll";
    }
  snprintf (locname, 32, "%s%s%s%s", locale->name, utf8 ? ".utf8" : "",
				     modifier ? "@" : "", modifier ?: "");
  if (verbose)
    fputs ("locale: ", stdout);
  if (verbose)
    {
      printf ("%-15s ", locname);
      printf ("archive: %s\n",
      locale->alias ? LOCALE_ALIAS : sysroot);
      puts ("-------------------------------------------------------------------------------");
      printf (" language | %ls\n", locale->language);
      printf ("territory | %ls\n", locale->territory);
      printf ("  codeset | %s\n\n", utf8 ? "UTF-8" : locale->codeset);
    }
  else
    printf ("%s\n", locname);
}

void
print_locale (int verbose, loc_t *locale)
{
  print_locale_with_codeset (verbose, locale, false, NULL);
  char *modifier = strchr (locale->name, '@');
  if (!locale->alias)
    {
      if (!modifier)
	print_locale_with_codeset (verbose, locale, true, NULL);
      else if (!strcmp (modifier, "@cjknarrow"))
	{
	  *modifier++ = '\0';
	  print_locale_with_codeset (verbose, locale, true, modifier);
	}
    }
}

int
compare_locales (const void *a, const void *b)
{
  const loc_t *la = (const loc_t *) a;
  const loc_t *lb = (const loc_t *) b;
  return strcmp (la->name, lb->name);
}

void
add_locale (const char *name, const wchar_t *language, const wchar_t *territory,
	    bool alias = false)
{
  char orig_locale[32];

  if (loc_num >= loc_max)
    {
      loc_t *tmp = (loc_t *) realloc (locale, (loc_max + 32) * sizeof (loc_t));
      if (!tmp)
      	{
	  fprintf (stderr, "Out of memory!\n");
	  exit (1);
	}
      locale = tmp;
      loc_max += 32;
    }
  locale[loc_num].name = strdup (name);
  locale[loc_num].language = wcsdup (language);
  locale[loc_num].territory = wcsdup (territory);
  strcpy (orig_locale, setlocale (LC_CTYPE, NULL));
  setlocale (LC_CTYPE, name);
  locale[loc_num].codeset = strdup (nl_langinfo (CODESET));
  setlocale (LC_CTYPE, orig_locale);
  locale[loc_num].alias = alias;
  ++loc_num;
}

void
add_locale_alias_locales ()
{
  char alias_buf[LOCALE_ALIAS_LINE_LEN + 1], *c;
  const char *alias, *replace;
  char orig_locale[32];
  loc_t search, *loc;
  size_t orig_loc_num = loc_num;

  FILE *fp = fopen (LOCALE_ALIAS, "rt");
  if (!fp)
    return;
  strcpy (orig_locale, setlocale (LC_CTYPE, NULL));
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
      c = strchr (replace, '.');
      if (c)
	*c = '\0';
      search.name = replace;
      loc = (loc_t *) bsearch (&search, locale, orig_loc_num, sizeof (loc_t),
			       compare_locales);
      add_locale (alias, loc ? loc->language : L"", loc ? loc->territory : L"",
		  true);
    }
  fclose (fp);
}

void
print_all_locales (int verbose)
{
  LCID lcid = 0;
  char name[32];
  DWORD cp;

  unsigned lang, sublang;

  add_locale ("C", L"C", L"POSIX");
  add_locale ("POSIX", L"C", L"POSIX", true);
  for (lang = 1; lang <= 0xff; ++lang)
    {
      struct {
	wchar_t language[256];
	wchar_t country[256];
	char loc[32];
      } loc_list[32];
      int lcnt = 0;

      for (sublang = 1; sublang <= 0x3f; ++sublang)
	{
	  lcid = (sublang << 10) | lang;
	  if (getlocale (lcid, name))
	    {
	      wchar_t language[256];
	      wchar_t country[256];
	      int i;
	      char *c, loc[32];
	      wchar_t wbuf[9];

	      /* Go figure.  Even the English name of a language or
		 locale might contain native characters. */
	      GetLocaleInfoW (lcid, LOCALE_SENGLANGUAGE, language, 256);
	      GetLocaleInfoW (lcid, LOCALE_SENGCOUNTRY, country, 256);
	      /* Avoid dups */
	      for (i = 0; i < lcnt; ++ i)
		if (!wcscmp (loc_list[i].language, language)
		    && !wcscmp (loc_list[i].country, country))
		  break;
	      if (i < lcnt)
		continue;
	      if (lcnt < 32)
		{
		  wcscpy (loc_list[lcnt].language, language);
		  wcscpy (loc_list[lcnt].country, country);
		}
	      c = stpcpy (loc, name);
	      /* Convert old sr_SP silently to sr_CS on old systems.
		 Make sure sr_CS country is in recent shape. */
	      if (lang == LANG_SERBIAN
		  && (sublang == SUBLANG_SERBIAN_LATIN
		      || sublang == SUBLANG_SERBIAN_CYRILLIC))
		{
		  c = stpcpy (loc, "sr_CS");
		  wcscpy (country, L"Serbia and Montenegro (Former)");
		}
	      /* Now check certain conditions to figure out if that
		 locale requires a modifier. */
	      if (lang == LANG_SERBIAN && !strncmp (loc, "sr_", 3)
		  && wcsstr (language, L"(Latin)"))
		stpcpy (c, "@latin");
	      else if (lang == LANG_UZBEK
		       && sublang == SUBLANG_UZBEK_CYRILLIC)
		stpcpy (c, "@cyrillic");
	      /* Avoid more dups */
	      for (i = 0; i < lcnt; ++ i)
		if (!strcmp (loc_list[i].loc, loc))
		  {
		    lcnt++;
		    break;
		  }
	      if (i < lcnt)
		continue;
	      if (lcnt < 32)
		strcpy (loc_list[lcnt++].loc, loc);
	      /* Print */
	      add_locale (loc, language, country);
	      /* Check for locales which sport a modifier for
		 changing the codeset and other stuff. */
	      if (lang == LANG_BELARUSIAN
		  && sublang == SUBLANG_BELARUSIAN_BELARUS)
		stpcpy (c, "@latin");
	      else if (lang == LANG_TATAR
		       && sublang == SUBLANG_TATAR_RUSSIA)
		stpcpy (c, "@iqtelif");
	      else if (GetLocaleInfoW (lcid,
				       LOCALE_IDEFAULTANSICODEPAGE
				       | LOCALE_RETURN_NUMBER,
				       (PWCHAR) &cp, sizeof cp)
		       && cp == 1252 /* Latin1*/
		       && GetLocaleInfoW (lcid, LOCALE_SINTLSYMBOL, wbuf, 9)
		       && !wcsncmp (wbuf, L"EUR", 3))
		stpcpy (c, "@euro");
	      else if (lang == LANG_JAPANESE
		       || lang == LANG_KOREAN
		       || lang == LANG_CHINESE)
		stpcpy (c, "@cjknarrow");
	      else
		continue;
	      add_locale (loc, language, country);
	    }
	}
      /* Check Serbian language for the available territories.  Up to
	 Server 2003 we only had sr_SP (silently converted to sr_CS
	 above), in Vista we had only sr_CS.  First starting with W7 we
	 have the actual sr_RS and sr_ME.  However, all of them are
	 supported on all systems in Cygwin.  So we fake them here, if
	 they are missing. */
      if (lang == LANG_SERBIAN)
	{
	  int sr_CS_idx = -1;
	  int sr_RS_idx = -1;
	  int i;

	  for (i = 0; i < lcnt; ++ i)
	    if (!strcmp (loc_list[i].loc, "sr_CS"))
	      sr_CS_idx = i;
	    else if (!strcmp (loc_list[i].loc, "sr_RS"))
	      sr_RS_idx = i;
	  if (sr_CS_idx > 0 && sr_RS_idx == -1)
	    {
	      add_locale ("sr_RS@latin", L"Serbian (Latin)", L"Serbia");
	      add_locale ("sr_RS", L"Serbian (Cyrillic)", L"Serbia");
	      add_locale ("sr_ME@latin", L"Serbian (Latin)", L"Montenegro");
	      add_locale ("sr_ME", L"Serbian (Cyrillic)", L"Montenegro");
	    }
	}
    }
  /* First sort allows add_locale_alias_locales to bsearch in locales. */
  qsort (locale, loc_num, sizeof (loc_t), compare_locales);
  add_locale_alias_locales ();
  qsort (locale, loc_num, sizeof (loc_t), compare_locales);
  for (size_t i = 0; i < loc_num; ++i)
    print_locale (verbose, &locale[i]);
}

void
print_charmaps ()
{
  /* FIXME: We need a method to fetch the available charsets from Cygwin, */
  const char *charmaps[] =
  {
    "ASCII",
    "BIG5",
    "CP1125",
    "CP1250",
    "CP1251",
    "CP1252",
    "CP1253",
    "CP1254",
    "CP1255",
    "CP1256",
    "CP1257",
    "CP1258",
    "CP437",
    "CP720",
    "CP737",
    "CP775",
    "CP850",
    "CP852",
    "CP855",
    "CP857",
    "CP858",
    "CP862",
    "CP866",
    "CP874",
    "CP932",
    "EUC-CN",
    "EUC-JP",
    "EUC-KR",
    "GB2312",
    "GBK",
    "GEORGIAN-PS",
    "ISO-8859-1",
    "ISO-8859-10",
    "ISO-8859-11",
    "ISO-8859-13",
    "ISO-8859-14",
    "ISO-8859-15",
    "ISO-8859-16",
    "ISO-8859-2",
    "ISO-8859-3",
    "ISO-8859-4",
    "ISO-8859-5",
    "ISO-8859-6",
    "ISO-8859-7",
    "ISO-8859-8",
    "ISO-8859-9",
    "KOI8-R",
    "KOI8-U",
    "PT154",
    "SJIS",
    "TIS-620",
    "UTF-8",
    NULL
  };
  const char **charmap = charmaps;
  while (*charmap)
    printf ("%s\n", *charmap++);
}

void
print_lc_ivalue (int key, const char *name, int value)
{
  if (key)
    printf ("%s=", name);
  printf ("%d", value == CHAR_MAX ? -1 : value);
  fputc ('\n', stdout);
}

void
print_lc_svalue (int key, const char *name, const char *value)
{
  if (key)
    printf ("%s=\"", name);
  fputs (value, stdout);
  if (key)
    fputc ('"', stdout);
  fputc ('\n', stdout);
}

void
print_lc_sepstrings (int key, const char *name, const char *value)
{
  char *c;

  if (key)
    printf ("%s=", name);
  while (value && *value)
    {
      if (key)
	fputc ('"', stdout);
      c = strchr (value, ';');
      if (!c)
	{
	  fputs (value, stdout);
	  value = NULL;
	}
      else
	{
	  printf ("%.*s", (int) (c - value), value);
	  value = c + 1;
	}
      if (key)
	fputc ('"', stdout);
      if (value && *value)
	fputc (';', stdout);
    }
  fputc ('\n', stdout);
}

void
print_lc_strings (int key, const char *name, int from, int to)
{
  if (key)
    printf ("%s=\"", name);
  for (int i = from; i <= to; ++i)
    printf ("%s%s", i > from ? ";" : "", nl_langinfo (i));
  if (key)
    fputc ('"', stdout);
  fputc ('\n', stdout);
}

void
print_lc_grouping (int key, const char *name, const char *grouping)
{
  if (key)
    printf ("%s=", name);
  for (const char *g = grouping; *g; ++g)
    printf ("%s%d", g > grouping ? ";" : "", *g == CHAR_MAX ? -1 : *g);
  fputc ('\n', stdout);
}

enum type_t
{
  is_string_fake,
  is_grouping,
  is_string,
  is_mstrings,
  is_sepstrings,
  is_int,
  is_wchar,
  is_end
};

struct lc_names_t
{
  const char *name;
  type_t      type;
  size_t      fromval;
  size_t      toval;
};

const char *fake_string[] = {
 "upper;lower;alpha;digit;xdigit;space;print;graph;blank;cntrl;punct;alnum",
 "upper\";\"lower\";\"alpha\";\"digit\";\"xdigit\";\"space\";\"print\";\"graph\";\"blank\";\"cntrl\";\"punct\";\"alnum",
 "toupper;tolower",
 "toupper\";\"tolower"
};

lc_names_t lc_ctype_names[] =
{
  { "ctype-class-names", 	 is_string_fake, 0,			 0 },
  { "ctype-map-names",   	 is_string_fake, 2,			 0 },
  { "ctype-outdigit0_mb",	 is_string,	_NL_CTYPE_OUTDIGITS0_MB, 0 },
  { "ctype-outdigit1_mb",	 is_string,	_NL_CTYPE_OUTDIGITS1_MB, 0 },
  { "ctype-outdigit2_mb",	 is_string,	_NL_CTYPE_OUTDIGITS2_MB, 0 },
  { "ctype-outdigit3_mb",	 is_string,	_NL_CTYPE_OUTDIGITS3_MB, 0 },
  { "ctype-outdigit4_mb",	 is_string,	_NL_CTYPE_OUTDIGITS4_MB, 0 },
  { "ctype-outdigit5_mb",	 is_string,	_NL_CTYPE_OUTDIGITS5_MB, 0 },
  { "ctype-outdigit6_mb",	 is_string,	_NL_CTYPE_OUTDIGITS6_MB, 0 },
  { "ctype-outdigit7_mb",	 is_string,	_NL_CTYPE_OUTDIGITS7_MB, 0 },
  { "ctype-outdigit8_mb",	 is_string,	_NL_CTYPE_OUTDIGITS8_MB, 0 },
  { "ctype-outdigit9_mb",	 is_string,	_NL_CTYPE_OUTDIGITS9_MB, 0 },
  { "ctype-outdigit0_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS0_WC, 0 },
  { "ctype-outdigit1_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS1_WC, 0 },
  { "ctype-outdigit2_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS2_WC, 0 },
  { "ctype-outdigit3_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS3_WC, 0 },
  { "ctype-outdigit4_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS4_WC, 0 },
  { "ctype-outdigit5_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS5_WC, 0 },
  { "ctype-outdigit6_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS6_WC, 0 },
  { "ctype-outdigit7_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS7_WC, 0 },
  { "ctype-outdigit8_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS8_WC, 0 },
  { "ctype-outdigit9_wc",	 is_wchar, 	_NL_CTYPE_OUTDIGITS9_WC, 0 },
  { "charmap",			 is_string,	CODESET,		 0 },
  { "ctype-mb-cur-max",		 is_int,	_NL_CTYPE_MB_CUR_MAX,	 0 },
  { NULL, 			 is_end,	0,		 	 0 }
};

lc_names_t lc_numeric_names[] =
{
  { "decimal_point",		 is_string,	RADIXCHAR,		 0 },
  { "thousands_sep",		 is_string,	THOUSEP, 		 0 },
  { "grouping",			 is_grouping,	_NL_NUMERIC_GROUPING,	 0 },
  { "numeric-decimal-point-wc",	 is_wchar,	_NL_NUMERIC_DECIMAL_POINT_WC, 0 },
  { "numeric-thousands-sep-wc",	 is_wchar,	_NL_NUMERIC_THOUSANDS_SEP_WC, 0 },
  { "numeric-codeset",		 is_string,	_NL_NUMERIC_CODESET,	 0 },
  { NULL, 			 is_end,	0,			 0 }
};

lc_names_t lc_time_names[] =
{
  { "abday",			 is_mstrings,	ABDAY_1,	 ABDAY_7  },
  { "day",			 is_mstrings,	DAY_1,		 DAY_7    },
  { "abmon",			 is_mstrings,	ABMON_1,	 ABMON_12 },
  { "mon",			 is_mstrings,	MON_1,		 MON_12   },
  { "am_pm",			 is_mstrings,	AM_STR,		 PM_STR   },
  { "d_t_fmt",			 is_string,	D_T_FMT,		0 },
  { "d_fmt",			 is_string,	D_FMT,			0 },
  { "t_fmt",			 is_string,	T_FMT,			0 },
  { "t_fmt_ampm",		 is_string,	T_FMT_AMPM,		0 },
  { "era",			 is_sepstrings,	ERA,			0 },
  { "era_d_fmt",		 is_string,	ERA_D_FMT,		0 },
  { "alt_digits",		 is_sepstrings,ALT_DIGITS,		0 },
  { "era_d_t_fmt",		 is_string,	ERA_D_T_FMT,		0 },
  { "era_t_fmt",		 is_string,	ERA_T_FMT,		0 },
  { "date_fmt",			 is_string,	_DATE_FMT,		0 },
  { "time-codeset",		 is_string,	_NL_TIME_CODESET,	0 },
  { NULL, 			 is_end,	0,			0 }
};

lc_names_t lc_collate_names[] =
{
  { "collate-codeset",		 is_string,	_NL_COLLATE_CODESET,	0 },
  { NULL, 			 is_end,	0,			0 }
};

lc_names_t lc_monetary_names[] =
{
  { "int_curr_symbol",		 is_string,	_NL_MONETARY_INT_CURR_SYMBOL, 0 },
  { "currency_symbol",		 is_string,	_NL_MONETARY_CURRENCY_SYMBOL, 0 },
  { "mon_decimal_point",	 is_string,	_NL_MONETARY_MON_DECIMAL_POINT, 0 },
  { "mon_thousands_sep",	 is_string,	_NL_MONETARY_MON_THOUSANDS_SEP, 0 },
  { "mon_grouping",		 is_grouping,	_NL_MONETARY_MON_GROUPING, 0 },
  { "positive_sign",		 is_string,	_NL_MONETARY_POSITIVE_SIGN, 0 },
  { "negative_sign",		 is_string,	_NL_MONETARY_NEGATIVE_SIGN, 0 },
  { "int_frac_digits",		 is_int,	_NL_MONETARY_INT_FRAC_DIGITS, 0 },
  { "frac_digits",		 is_int,	_NL_MONETARY_FRAC_DIGITS,   0 },
  { "p_cs_precedes",		 is_int,	_NL_MONETARY_P_CS_PRECEDES, 0 },
  { "p_sep_by_space",		 is_int,	_NL_MONETARY_P_SEP_BY_SPACE, 0 },
  { "n_cs_precedes",		 is_int,	_NL_MONETARY_N_CS_PRECEDES, 0 },
  { "n_sep_by_space",		 is_int,	_NL_MONETARY_N_SEP_BY_SPACE, 0 },
  { "p_sign_posn",		 is_int,	_NL_MONETARY_P_SIGN_POSN,   0 },
  { "n_sign_posn",		 is_int,	_NL_MONETARY_N_SIGN_POSN,   0 },
  { "int_p_cs_precedes",	 is_int,	_NL_MONETARY_INT_P_CS_PRECEDES, 0 },
  { "int_p_sep_by_space",	 is_int,	_NL_MONETARY_INT_P_SEP_BY_SPACE,0 },
  { "int_n_cs_precedes",	 is_int,	_NL_MONETARY_INT_N_CS_PRECEDES, 0 },
  { "int_n_sep_by_space",	 is_int,	_NL_MONETARY_INT_N_SEP_BY_SPACE,0 },
  { "int_p_sign_posn",		 is_int,	_NL_MONETARY_INT_P_SIGN_POSN, 0 },
  { "int_n_sign_posn",		 is_int,	_NL_MONETARY_INT_N_SIGN_POSN, 0 },
  { "monetary-decimal-point-wc", is_wchar,	_NL_MONETARY_WMON_DECIMAL_POINT, 0 },
  { "monetary-thousands-sep-wc", is_wchar,	_NL_MONETARY_WMON_THOUSANDS_SEP, 0 },
  { "monetary-codeset",		 is_string,	_NL_MONETARY_CODESET,	   0 },
  { NULL, 			 is_end,	0,			   0 }
};

lc_names_t lc_messages_names[] =
{
  { "yesexpr",			 is_string,	YESEXPR,		0 },
  { "noexpr",			 is_string,	NOEXPR,			0 },
  { "yesstr",			 is_string,	YESSTR,			0 },
  { "nostr",			 is_string,	NOSTR,			0 },
  { "messages-codeset",		 is_string,	_NL_MESSAGES_CODESET,	0 },
  { NULL, 			 is_end,	0,			0 }
};

void
print_lc (int cat, int key, const char *category, const char *name,
	  lc_names_t *lc_name)
{
  if (cat)
    printf ("%s\n", category);
  for (lc_names_t *lc = lc_name; lc->type != is_end; ++lc)
    if (!name || !strcmp (name, lc->name))
      switch (lc->type)
	{
	case is_string_fake:
	  print_lc_svalue (key, lc->name, fake_string[lc->fromval + key]);
	  break;
	case is_grouping:
	  print_lc_grouping (key, lc->name, nl_langinfo (lc->fromval));
	  break;
	case is_string:
	  print_lc_svalue (key, lc->name, nl_langinfo (lc->fromval));
	  break;
	case is_sepstrings:
	  print_lc_sepstrings (key, lc->name, nl_langinfo (lc->fromval));
	  break;
	case is_mstrings:
	  print_lc_strings (key, lc->name, lc->fromval, lc->toval);
	  break;
	case is_int:
	  print_lc_ivalue (key, lc->name, (int) *nl_langinfo (lc->fromval));
	  break;
	case is_wchar:
	  print_lc_ivalue (key, lc->name,
			   *(wchar_t *) nl_langinfo (lc->fromval));
	  break;
	default:
	  break;
	}
}

struct cat_t
{
  const char *category;
  int lc_cat;
  lc_names_t *lc_names;
} categories[] =
{
  { "LC_CTYPE",    LC_CTYPE,    lc_ctype_names    },
  { "LC_NUMERIC",  LC_NUMERIC,  lc_numeric_names  },
  { "LC_TIME",     LC_TIME,     lc_time_names     },
  { "LC_COLLATE",  LC_COLLATE,  lc_collate_names  },
  { "LC_MONETARY", LC_MONETARY, lc_monetary_names },
  { "LC_MESSAGES", LC_MESSAGES, lc_messages_names },
  { NULL,	   0,		NULL		  }
};

void
print_names (int cat, int key, const char *name)
{
  struct cat_t *c;
  lc_names_t *lc;

  for (c = categories; c->category; ++c)
    if (!strcmp (name, c->category))
      {
	print_lc (cat, key, c->category, NULL, c->lc_names);
	return;
      }
  for (c = categories; c->category; ++c)
    for (lc = c->lc_names; lc->type != is_end; ++lc)
      if (!strcmp (name, lc->name))
      {
	print_lc (cat, key, c->category, lc->name, lc);
	return;
      }
}

void
print_lc ()
{
  printf ("LANG=%s\n", getenv ("LANG") ?: "");
  printf ("LC_CTYPE=\"%s\"\n", setlocale (LC_CTYPE, NULL));
  printf ("LC_NUMERIC=\"%s\"\n", setlocale (LC_NUMERIC, NULL));
  printf ("LC_TIME=\"%s\"\n", setlocale (LC_TIME, NULL));
  printf ("LC_COLLATE=\"%s\"\n", setlocale (LC_COLLATE, NULL));
  printf ("LC_MONETARY=\"%s\"\n", setlocale (LC_MONETARY, NULL));
  printf ("LC_MESSAGES=\"%s\"\n", setlocale (LC_MESSAGES, NULL));
  printf ("LC_ALL=%s\n", getenv ("LC_ALL") ?: "");
}

int
main (int argc, char **argv)
{
  int opt;
  LCID lcid = 0;
  int all = 0;
  int cat = 0;
  int key = 0;
  int maps = 0;
  int verbose = 0;
  const char *utf = "";
  char name[32];

  setlocale (LC_ALL, "");
  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != -1)
    switch (opt)
      {
      case 'a':
	all = 1;
	break;
      case 'c':
	cat = 1;
	break;
      case 'k':
	key = 1;
	break;
      case 'm':
	maps = 1;
	break;
      case 's':
	lcid = GetSystemDefaultUILanguage ();
	break;
      case 'u':
	lcid = GetUserDefaultUILanguage ();
	break;
      case 'f':
	lcid = GetUserDefaultLCID ();
	break;
      case 'n':
	lcid = GetSystemDefaultLCID ();
	break;
      case 'U':
	utf = ".UTF-8";
	break;
      case 'v':
	verbose = 1;
	break;
      case 'h':
	usage ();
	return 0;
      case 'V':
	print_version ();
	return 0;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n",
		 program_invocation_short_name);
	return 1;
      }
  if (all)
    print_all_locales (verbose);
  else if (maps)
    print_charmaps ();
  else if (lcid)
    {
      if (getlocale (lcid, name))
	printf ("%s%s\n", name, utf);
    }
  else if (optind < argc)
    while (optind < argc)
      print_names (cat, key, argv[optind++]);
  else
    print_lc ();
  return 0;
}
