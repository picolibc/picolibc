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
#include <getopt.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#define WINVER 0x0601
#include <windows.h>

extern char *__progname;

void usage (FILE *, int) __attribute__ ((noreturn));

void
usage (FILE * stream, int status)
{
  fprintf (stream,
	   "Usage: %s [-asuh]\n"
	   "Print default locale or list of all supported locales\n"
	   "\n"
	   "Options:\n"
	   "\n"
	   "  -a, --all       List all available supported locales\n"
	   "  -s, --system    Print system default locale\n"
	   "                  (default is current user default locale)\n"
	   "  -u, --utf       Attach \".UTF-8\" to the result\n"
	   "  -h, --help      this text\n",
	   __progname);
  exit (status);
}

struct option longopts[] = {
  {"all", no_argument, NULL, 'a'},
  {"system", no_argument, NULL, 's'},
  {"utf", no_argument, NULL, 'u'},
  {"help", no_argument, NULL, 'h'},
  {0, no_argument, NULL, 0}
};
const char *opts = "ahsu";

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

int main (int argc, char **argv)
{
  int opt;
  LCID lcid = LOCALE_USER_DEFAULT;
  int all = 0;
  const char *utf = "";
  char name[32];

  setlocale (LC_ALL, "");
  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (opt)
      {
      case 'a':
        all = 1;
	break;
      case 's':
      	lcid = LOCALE_SYSTEM_DEFAULT;
	break;
      case 'u':
      	utf = ".UTF-8";
	break;
      case 'h':
	usage (stdout, 0);
	break;
      default:
	usage (stderr, 1);
	break;
      }
  if (all)
    {
      unsigned lang, sublang;

      for (lang = 1; lang <= 0xff; ++lang)
	{
	  struct {
	    wchar_t lang[256];
	    wchar_t country[256];
	    char loc[32];
	  } loc_list[32];
	  int lcnt = 0;

	  for (sublang = 1; sublang <= 0x3f; ++sublang)
	    {
	      lcid = (sublang << 10) | lang;
	      if (getlocale (lcid, name))
		{
		  wchar_t lang[256];
		  wchar_t country[256];
		  int i;
		  char *c, loc[32];
		  wchar_t wbuf[9];

		  /* Go figure.  Even the English name of a language or
		     locale might contain native characters. */
		  GetLocaleInfoW (lcid, LOCALE_SENGLANGUAGE, lang, 256);
		  GetLocaleInfoW (lcid, LOCALE_SENGCOUNTRY, country, 256);
		  /* Avoid dups */
		  for (i = 0; i < lcnt; ++ i)
		    if (!wcscmp (loc_list[i].lang, lang)
			&& !wcscmp (loc_list[i].country, country))
		      break;
		  if (i < lcnt)
		    continue;
		  if (lcnt < 32)
		    {
		      wcscpy (loc_list[lcnt].lang, lang);
		      wcscpy (loc_list[lcnt].country, country);
		    }
		  /* Now check certain conditions to figure out if that
		     locale requires a modifier. */
		  c = stpcpy (loc, name);
		  if (wcsstr (lang, L"(Latin)")
		      && (!strncmp (loc, "sr_", 3)
			  || !strcmp (loc, "be_BY")))
		    stpcpy (c, "@latin");
		  else if (wcsstr (lang, L"(Cyrillic)")
			   && !strcmp (loc, "uz_UZ"))
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
		  printf ("%-16s %ls (%ls)\n", loc, lang, country);
		  /* Check for locales which sport a modifier for
		     changing the codeset and other stuff. */
		  if (!strcmp (loc, "tt_RU"))
		    stpcpy (c, "@iqtelif");
		  else if (GetLocaleInfoW (lcid, LOCALE_SINTLSYMBOL, wbuf, 9)
			   && !wcsncmp (wbuf, L"EUR", 3))
		    stpcpy (c, "@euro");
		  else if (!strncmp (loc, "ja_", 3)
			   || !strncmp (loc, "ko_", 3)
			   || !strncmp (loc, "zh_", 3))
		    stpcpy (c, "@cjknarrow");
		  else
		    continue;
		  printf ("%-16s %ls (%ls)\n", loc, lang, country);
		}
	    }
	}
      return 0;
    }
  if (getlocale (lcid, name))
    printf ("%s%s\n", name, utf);
  return 0;
}
