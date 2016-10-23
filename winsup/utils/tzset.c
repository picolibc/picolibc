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

#ifndef GEOID_NOT_AVAILABLE
#define GEOID_NOT_AVAILABLE -1
#endif

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

#define REG_TZINFO L"SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation"
#define REG_TZDB L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"

static inline HKEY
reg_open (HKEY pkey, PCWSTR path, const char *msg)
{
  LONG ret;
  HKEY hkey;

  ret = RegOpenKeyExW (pkey, path, 0, KEY_READ, &hkey);
  if (ret == ERROR_SUCCESS)
    return hkey;
  if (msg)
    fprintf (stderr, "%s: cannot open registry %s, error code %" PRIu32 "\n",
	     program_invocation_short_name, msg, (unsigned int) ret);
  return NULL;
}

/* For symmetry */
#define reg_close(hkey)	RegCloseKey(hkey)

static inline BOOL
reg_query (HKEY hkey, PCWSTR value, PWCHAR buf, DWORD size, const char *msg)
{
  LONG ret;
  DWORD type;

  ret = RegQueryValueExW (hkey, value, 0, &type, (LPBYTE) buf, &size);
  if (ret == ERROR_SUCCESS)
    return TRUE;
  if (msg)
    fprintf (stderr, "%s: cannot query registry %s, error code %" PRIu32 "\n",
	     program_invocation_short_name, msg, (unsigned int) ret);
  return FALSE;
}

static inline BOOL
reg_enum (HKEY hkey, int idx, PWCHAR name, DWORD size)
{
  return RegEnumKeyExW (hkey, idx, name, &size, NULL, NULL, NULL, NULL)
	 == ERROR_SUCCESS;
}

static void
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
  BOOL ret;
  HKEY hkey, skey;
  WCHAR keyname[256], stdname[256], std2name[256], country[10], *spc;
  GEOID geo;
  int opt, idx, gotit = -1;

  setlocale (LC_ALL, "");
  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (opt)
      {
      case 'h':
	usage (stdout);
	return 0;
      case 'V':
	print_version ();
	return 0;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n",
		 program_invocation_short_name);
	return 1;
      }
  if (optind < argc)
    {
	usage (stderr);
	return 1;
    }

  /* First fetch current timezone information from registry. */
  hkey = reg_open (HKEY_LOCAL_MACHINE, REG_TZINFO, "timezone information");
  if (!hkey)
    return 1;
  /* Vista introduced the TimeZoneKeyName value, which simplifies the
     job a lot. */
  if (!reg_query (hkey, L"TimeZoneKeyName", keyname, sizeof keyname, NULL))
    {
      /* Pre-Vista we have a lot more to do.  First fetch the name of the
	 Standard (non-DST) timezone.  If we can't get that, give up. */
      if (!reg_query (hkey, L"StandardName", stdname, sizeof stdname,
		      "timezone information"))
	{
	  reg_close (hkey);
	  return 1;
	}
      reg_close (hkey);
      /* Now open the timezone database registry key.  Every subkey is a
         timezone.  The key name is what we're after, but to find the right
	 one, we have to compare the name of the previously fetched
	 "StandardName" with the "Std" value in the timezone info... */
      hkey = reg_open (HKEY_LOCAL_MACHINE, REG_TZDB, "timezone database");
      if (!hkey)
	return 1;
      for (idx = 0; reg_enum (hkey, idx, keyname, sizeof keyname); ++idx)
	{
	  skey = reg_open (hkey, keyname, NULL);
	  if (skey)
	    {
	      /* ...however, on MUI-enabled machines, the names are not stored
		 directly in the above StandardName, rather it is a resource
		 pointer into tzres.dll.  This is stored in MUI_Std.
		 Fortunately it's easy to recognize this situation: If
		 StandardName starts with @, it's a resource pointer, otherwise
		 it's the cleartext value. */
	      ret = reg_query (skey, stdname[0] == L'@' ? L"MUI_Std" : L"Std",
			       std2name, sizeof std2name, NULL);
	      reg_close (skey);
	      if (ret && !wcscmp (stdname, std2name))
		break;
	    }
	}
    }
  reg_close (hkey);

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
      if (!wcscasecmp (keyname, tzmap[idx].win_tzkey))
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
		   program_invocation_short_name, keyname);
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
