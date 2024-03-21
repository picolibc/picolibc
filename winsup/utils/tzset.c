/* tzset.c: Convert current Windows timezone to POSIX timezone information.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <errno.h>
#include <stdio.h>
#include <inttypes.h>
#include <wchar.h>
#include <locale.h>
#include <getopt.h>
#include <cygwin/version.h>
#include <windows.h>

/* The auto-generated tzmap.h contains the mapping table from Windows timezone
   and country per ISO 3166-1 to POSIX timezone.  For more info, see the file
   itself. */
#include "tzmap.h"

#define TZMAP_SIZE (sizeof tzmap / sizeof tzmap[0])

static struct option longopts[] =
{
  {"help", no_argument, NULL, 'h' },
  {"version", no_argument, NULL, 'V'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "hV";

static void __attribute__ ((__noreturn__))
usage (FILE *stream)
{
  fprintf (stream, ""
  "Usage: %1$s [OPTION]\n"
  "\n"
  "Print POSIX-compatible timezone ID from current Windows timezone setting\n"
  "\n"
  "Options:\n"
  "  -h, --help               output usage information and exit.\n"
  "  -V, --version            output version information and exit.\n"
  "\n"
  "Use %1$s to set your TZ variable. In POSIX-compatible shells like bash,\n"
  "dash, mksh, or zsh:\n"
  "\n"
  "      export TZ=$(%1$s)\n"
  "\n"
  "In csh-compatible shells like tcsh:\n"
  "\n"
  "      setenv TZ `%1$s`\n"
  "\n", program_invocation_short_name);
  exit (stream == stdout ? 0 : 1);
};

static void
print_version ()
{
  printf ("tzset (cygwin) %d.%d.%d\n"
	  "POSIX-timezone generator\n"
	  "Copyright (C) 2012 - %s Cygwin Authors\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

int
main (int argc, char **argv)
{
  DYNAMIC_TIME_ZONE_INFORMATION tz;
  WCHAR country[10], *spc;
  GEOID geo;
  int opt, idx, gotit = -1;

  setlocale (LC_ALL, "");
  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (opt)
      {
      case 'h':
	usage (stdout);
      case 'V':
	print_version ();
	return 0;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n",
		 program_invocation_short_name);
	return 1;
      }
  if (optind < argc)
    usage (stderr);

  if (GetDynamicTimeZoneInformation (&tz) == TIME_ZONE_ID_INVALID)
    return 1;

  /* We fetch the current Geo-location of the user and convert it to an
     ISO 3166-1 compatible nation code. */
  *country = L'\0';
  geo = GetUserGeoID (GEOCLASS_NATION);
  if (geo != GEOID_NOT_AVAILABLE)
    GetGeoInfoW (geo, GEO_ISO2, country, sizeof country / sizeof (*country), 0);
  /* If, for some reason, the Geo-location isn't available, we use the locale
     setting instead. */
  if (!*country)
    GetLocaleInfoW (LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME,
		    country, sizeof country);

  /* Now iterate over the mapping table and find the right entry. */
  for (idx = 0; idx < TZMAP_SIZE; ++idx)
    {
      if (!wcscasecmp (tz.TimeZoneKeyName, tzmap[idx].win_tzkey))
	{
	  if (gotit < 0)
	    gotit = idx;
	  if (!wcscasecmp (country, tzmap[idx].country))
	    break;
	}
      else if (gotit >= 0)
	{
	  idx = gotit;
	  break;
	}
    }
  if (idx >= TZMAP_SIZE)
    {
      if (gotit < 0)
	{
	  fprintf (stderr,
		   "%s: can't find matching POSIX timezone for "
		   "Windows timezone \"%ls\"\n",
		   program_invocation_short_name, tz.TimeZoneKeyName);
	  return 1;
	}
      idx = gotit;
    }
  /* Got one.  Print it.
     Note: The tzmap array is in the R/O data section on x86_64.  Don't
           try to overwrite the space, as the code did originally. */
  spc = wcschr (tzmap[idx].posix_tzid, L' ');
  if (!spc)
    spc = wcschr (tzmap[idx].posix_tzid, L'\0');
  printf ("%.*ls\n", (int) (spc - tzmap[idx].posix_tzid), tzmap[idx].posix_tzid);
  return 0;
}
