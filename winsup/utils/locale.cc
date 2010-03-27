/*
 * Copyright (c) 2010, Corinna Vinschen
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
#include <stdio.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <langinfo.h>
#include <limits.h>
#include <sys/cygwin.h>
#define WINVER 0x0601
#include <windows.h>

#define LOCALE_ALIAS		"/usr/share/locale/locale.alias"
#define LOCALE_ALIAS_LINE_LEN	255

extern char *__progname;

void usage (FILE *, int) __attribute__ ((noreturn));

void
usage (FILE * stream, int status)
{
  fprintf (stream,
	   "Usage: %s [-amsuUvh]\n"
	   "   or: %s [-ck] NAME\n"
	   "Get locale-specific information.\n"
	   "\n"
	   "Options:\n"
	   "\n"
	   "  -a, --all-locales    List all available supported locales\n"
	   "  -c, --category-name  List information about given category NAME\n"
	   "  -k, --keyword-name   Print information about given keyword NAME\n"
	   "  -m, --charmaps       List all available character maps\n"
	   "  -s, --system         Print system default locale\n"
	   "  -u, --user           Print user's default locale\n"
	   "  -U, --utf            Attach \".UTF-8\" to the result\n"
	   "  -v, --verbose        More verbose output\n"
	   "  -h, --help           This text\n",
	   __progname, __progname);
  exit (status);
}

struct option longopts[] = {
  {"all-locales", no_argument, NULL, 'a'},
  {"category-name", no_argument, NULL, 'c'},
  {"keyword-name", no_argument, NULL, 'k'},
  {"charmaps", no_argument, NULL, 'm'},
  {"system", no_argument, NULL, 's'},
  {"user", no_argument, NULL, 'u'},
  {"utf", no_argument, NULL, 'U'},
  {"verbose", no_argument, NULL, 'v'},
  {"help", no_argument, NULL, 'h'},
  {0, no_argument, NULL, 0}
};
const char *opts = "achkmsuUv";

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
      char sysbuf[PATH_MAX];
      stpcpy (stpcpy (sysbuf, getenv ("SYSTEMROOT")),
	      "\\system32\\kernel32.dll");
      sysroot = (const char *) cygwin_create_path (CCP_WIN_A_TO_POSIX, sysbuf);
      if (!sysroot)
      	sysroot = "kernel32.dll";
    }
  snprintf (locname, 32, "%s%s%s%s", locale->name, utf8 ? ".utf8" : "",
				     modifier ? "@" : "", modifier ?: "");
  if (verbose)
    fputs ("locale: ", stdout);
  printf ("%-15s ", locname);
  if (verbose)
    {
      printf ("archive: %s\n",
      locale->alias ? LOCALE_ALIAS : sysroot);
      puts ("-------------------------------------------------------------------------------");
      printf (" language | %ls\n", locale->language);
      printf ("territory | %ls\n", locale->territory);
      printf ("  codeset | %s\n", utf8 ? "UTF-8" : locale->codeset);
    }
  putc ('\n', stdout);
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
	  printf ("%.*s", c - value, value);
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
print_lc_xxx_charset (int key, int lc_cat, const char *name)
{
  char lc_ctype_locale[32];
  char lc_xxx_locale[32];

  strcpy (lc_ctype_locale, setlocale (LC_CTYPE, NULL));
  strcpy (lc_xxx_locale, setlocale (lc_cat, NULL));
  setlocale (LC_CTYPE, lc_xxx_locale);
  print_lc_svalue (key, name, nl_langinfo (CODESET));
  setlocale (LC_CTYPE, lc_ctype_locale);
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
  is_string_lconv,
  is_int_lconv,
  is_grouping_lconv,
  is_string_linf,
  is_mstrings_linf,
  is_sepstrings_linf,
  is_mb_cur_max,
  is_codeset,
  is_end
};

struct lc_names_t
{
  const char *name;
  type_t      type;
  size_t      fromval;
  size_t      toval;
};

#define _O(M)		__builtin_offsetof (struct lconv, M)
#define _MS(l,lc)	(*(const char **)(((const char *)(l))+(lc)->fromval))
#define _MI(l,lc)	((int)*(((const char *)(l))+(lc)->fromval))

const char *fake_string[] = {
 "upper;lower;alpha;digit;xdigit;space;print;graph;blank;cntrl;punct;alnum",
 "upper\";\"lower\";\"alpha\";\"digit\";\"xdigit\";\"space\";\"print\";\"graph\";\"blank\";\"cntrl\";\"punct\";\"alnum",
 "toupper;tolower",
 "toupper\";\"tolower"
};

lc_names_t lc_ctype_names[] =
{
  { "ctype-class-names",is_string_fake,	   0,			0 },
  { "ctype-map-names",	is_string_fake,	   2,			0 },
  { "charmap",		is_string_linf,	   CODESET,		0 },
  { "ctype-mb-cur-max", is_mb_cur_max,	   0,			0 },
  { NULL, 		is_end,		   0,			0 }
};

lc_names_t lc_numeric_names[] =
{
  { "decimal_point",	is_string_lconv,   _O(decimal_point),	0 },
  { "thousands_sep",	is_string_lconv,   _O(thousands_sep), 	0 },
  { "grouping",		is_grouping_lconv, _O(grouping),	0 },
  { "numeric-codeset",	is_codeset,	   LC_NUMERIC,		0 },
  { NULL, 		is_end,		   0,			0 }
};

lc_names_t lc_time_names[] =
{
  { "abday",		is_mstrings_linf,  ABDAY_1,		ABDAY_7  },
  { "day",		is_mstrings_linf,  DAY_1,		DAY_7    },
  { "abmon",		is_mstrings_linf,  ABMON_1,		ABMON_12 },
  { "mon",		is_mstrings_linf,  MON_1,		MON_12   },
  { "am_pm",		is_mstrings_linf,  AM_STR,		PM_STR   },
  { "d_t_fmt",		is_string_linf,    D_T_FMT,		0        },
  { "d_fmt",		is_string_linf,    D_FMT,		0        },
  { "t_fmt",		is_string_linf,    T_FMT,		0        },
  { "t_fmt_ampm",	is_string_linf,    T_FMT_AMPM,		0	 },
  { "era",		is_sepstrings_linf,ERA,			0	 },
  { "era_d_fmt",	is_string_linf,    ERA_D_FMT,		0	 },
  { "alt_digits",	is_sepstrings_linf,ALT_DIGITS,		0	 },
  { "era_d_t_fmt",	is_string_linf,    ERA_D_T_FMT,		0	 },
  { "era_t_fmt",	is_string_linf,    ERA_T_FMT,		0	 },
  { "date_fmt",		is_string_linf,    _DATE_FMT,		0	 },
  { "time-codeset",	is_codeset,	   LC_TIME,		0	 },
  { NULL, 		is_end,		   0,			0	 }
};

lc_names_t lc_collate_names[] =
{
  { "collate-codeset",	is_codeset,	   LC_COLLATE,		0	 },
  { NULL, 		is_end,		   0,			0	 }
};

lc_names_t lc_monetary_names[] =
{
  { "int_curr_symbol",	is_string_lconv,   _O(int_curr_symbol),		0 },
  { "currency_symbol",	is_string_lconv,   _O(currency_symbol),		0 },
  { "mon_decimal_point",is_string_lconv,   _O(mon_decimal_point), 	0 },
  { "mon_thousands_sep",is_string_lconv,   _O(mon_thousands_sep),	0 },
  { "mon_grouping",	is_grouping_lconv, _O(mon_grouping),		0 },
  { "positive_sign",	is_string_lconv,   _O(positive_sign),		0 },
  { "negative_sign",	is_string_lconv,   _O(negative_sign),		0 },
  { "int_frac_digits",	is_int_lconv,      _O(int_frac_digits),		0 },
  { "frac_digits",	is_int_lconv,	   _O(frac_digits),		0 },
  { "p_cs_precedes",	is_int_lconv,	   _O(p_cs_precedes),		0 },
  { "p_sep_by_space",	is_int_lconv,	   _O(p_sep_by_space),		0 },
  { "n_cs_precedes",	is_int_lconv,	   _O(n_cs_precedes),		0 },
  { "n_sep_by_space",	is_int_lconv,	   _O(n_sep_by_space),		0 },
  { "p_sign_posn",	is_int_lconv,	   _O(p_sign_posn),		0 },
  { "n_sign_posn",	is_int_lconv,	   _O(n_sign_posn),		0 },
  { "int_p_cs_precedes",is_int_lconv,	   _O(int_p_cs_precedes),	0 },
  { "int_p_sep_by_space",is_int_lconv,	   _O(int_p_sep_by_space),	0 },
  { "int_n_cs_precedes",is_int_lconv,	   _O(int_n_cs_precedes),	0 },
  { "int_n_sep_by_space",is_int_lconv,	   _O(int_n_sep_by_space),	0 },
  { "int_p_sign_posn",	is_int_lconv,	   _O(int_p_sign_posn),		0 },
  { "int_n_sign_posn",	is_int_lconv,	   _O(int_n_sign_posn),		0 },
  { "monetary-codeset",	is_codeset,	   LC_MONETARY,			0 },
  { NULL, 		is_end,		   0,				0 }
};

lc_names_t lc_messages_names[] =
{
  { "yesexpr",		is_string_linf,	   YESEXPR,			0 },
  { "noexpr",		is_string_linf,	   NOEXPR,			0 },
  { "yesstr",		is_string_linf,	   YESSTR,			0 },
  { "nostr",		is_string_linf,	   NOSTR,			0 },
  { "messages-codeset",	is_codeset,	   LC_MESSAGES,			0 },
  { NULL, 		is_end,		   0,				0 }
};

void
print_lc (int cat, int key, const char *category, const char *name,
	  lc_names_t *lc_name)
{
  struct lconv *l = localeconv ();

  if (cat)
    printf ("%s\n", category);
  for (lc_names_t *lc = lc_name; lc->type != is_end; ++lc)
    if (!name || !strcmp (name, lc->name))
      switch (lc->type)
      	{
	case is_string_fake:
	  print_lc_svalue (key, lc->name, fake_string[lc->fromval + key]);
	  break;
	case is_string_lconv:
	  print_lc_svalue (key, lc->name, _MS (l, lc));
	  break;
	case is_int_lconv:
	  print_lc_ivalue (key, lc->name, _MI (l, lc));
	  break;
	case is_grouping_lconv:
	  print_lc_grouping (key, lc->name, _MS (l, lc));
	  break;
	case is_string_linf:
	  print_lc_svalue (key, lc->name, nl_langinfo (lc->fromval));
	  break;
	case is_sepstrings_linf:
	  print_lc_sepstrings (key, lc->name, nl_langinfo (lc->fromval));
	  break;
	case is_mstrings_linf:
	  print_lc_strings (key, lc->name, lc->fromval, lc->toval);
	  break;
	case is_mb_cur_max:
	  print_lc_ivalue (key, lc->name, MB_CUR_MAX);
	  break;
	case is_codeset:
	  print_lc_xxx_charset (key, lc->fromval, lc->name);
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
  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
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
      	lcid = LOCALE_SYSTEM_DEFAULT;
	break;
      case 'u':
      	lcid = LOCALE_USER_DEFAULT;
	break;
      case 'U':
      	utf = ".UTF-8";
	break;
      case 'v':
	verbose = 1;
	break;
      case 'h':
	usage (stdout, 0);
	break;
      default:
	usage (stderr, 1);
	break;
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
