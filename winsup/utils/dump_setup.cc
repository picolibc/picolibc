/* path.cc

   Copyright 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "path.h"

static int package_len = 20;
static int version_len = 20;


typedef struct
{
  char pkgtar[MAX_PATH + 1];
  char pkg[MAX_PATH + 1];
  char ver[MAX_PATH + 1];
  char tail[MAX_PATH + 1];
  char what[16];
} fileparse;

static int
find_tar_ext (const char *path)
{
  char *p = strchr (path, '\0') - 7;
  if (p <= path)
    return 0;
  if (*p == '.')
    {
      if (strcmp (p, ".tar.gz") != 0)
	return 0;
    }
  else if (--p <= path || strcmp (p, ".tar.bz2") != 0)
    return 0;

  return p - path;
}

static char *
base (const char *s)
{
  if (!s)
    return 0;
  const char *rv = s;
  while (*s)
    {
      if ((*s == '/' || *s == ':' || *s == '\\') && s[1])
	rv = s + 1;
      s++;
    }
  return (char *) rv;
}

/* Parse a filename into package, version, and extension components. */
int
parse_filename (const char *in_fn, fileparse& f)
{
  char *p, *ver;
  char fn[strlen (in_fn) + 1];

  strcpy (fn, in_fn);
  int n = find_tar_ext (fn);

  if (!n)
    return 0;

  strcpy (f.tail, fn + n);
  fn[n] = '\0';
  f.pkg[0] = f.what[0] = '\0';
  p = base (fn);
  for (ver = p; *ver; ver++)
    if (*ver == '-')
      if (isdigit (ver[1]))
	{
	  *ver++ = 0;
	  strcpy (f.pkg, p);
	  break;
	}
      else if (strcasecmp (ver, "-src") == 0 ||
	       strcasecmp (ver, "-patch") == 0)
	{
	  *ver++ = 0;
	  strcpy (f.pkg, p);
	  strcpy (f.what, strlwr (ver));
	  strcpy (f.pkgtar, p);
	  strcat (f.pkgtar, f.tail);
	  ver = strchr (ver, '\0');
	  break;
	}

  if (!f.pkg[0])
    strcpy (f.pkg, p);

  if (!f.what[0])
    {
      int n;
      p = strchr (ver, '\0');
      strcpy (f.pkgtar, in_fn);
      if ((p -= 4) >= ver && strcasecmp (p, "-src") == 0)
	n = 4;
      else if ((p -= 2) >= ver && strcasecmp (p, "-patch") == 0)
	n = 6;
      else
	n = 0;
      if (n)
	{
	  strcpy (f.what, p + 1);
	  *p = '\0';
	  p = f.pkgtar + (p - fn) + n;
	  memmove (p - 4, p, strlen (p));
	}
    }

  strcpy (f.ver, *ver ? ver : "0.0");
  return 1;
}

static bool
dump_file (const char *msg, const char *fn)
{
  char *path = cygpath ("/etc/setup/", fn, NULL);
  FILE *fp = fopen (path, "rt");
  bool printed;
  char buf[4096];
  if (!fp)
    printed = false;
  else if (!fgets (buf, 4096, fp))
    printed = false;
    {
      char *p = strchr (buf, '\0');
      printf ("%s%s%s", msg, buf, (p == buf) || p[-1] != '\n' ? "\n" : "");
      printed = true;
    }
  if (fp)
    fclose (fp);
  return printed;
}

struct pkgver
{
  char *name;
  char *ver;
};

extern "C" {
int
compar (const void *a, const void *b)
{
  const pkgver *pa = (const pkgver *) a;
  const pkgver *pb = (const pkgver *) b;
  return strcmp (pa->name, pb->name);
}
}

bool
match_argv (char **argv, const char *name)
{
  if (!argv || !*argv)
    return true;
  for (char **a = argv; *a; a++)
    if (strcasecmp (*a, name) == 0)
      return true;
  return false;
}

void
dump_setup (int verbose, char **argv, bool /*check_files*/)
{
  char *setup = cygpath ("/etc/setup/installed.db", NULL);
  FILE *fp = fopen (setup, "rt");
  puts ("Cygwin Package Information");
  if (fp == NULL)
    goto err;
  if (verbose)
    {
      bool need_nl = dump_file ("Last downloaded files to: ", "last-cache");
      if (dump_file ("Last downloaded files from: ", "last-mirror") || need_nl)
	puts ("");
    }

  if (!fp)
    goto err;

  int nlines;
  nlines = 0;
  char buf[4096];
  while (fgets (buf, 4096, fp))
    nlines += 2;	/* potentially binary + source */
  if (!nlines)
    goto err;
  rewind (fp);

  pkgver *packages;

  packages = (pkgver *) calloc (nlines, sizeof(packages[0]));
  printf ("%-*s%-*s\n", package_len, "Package", version_len, "Version");
  int n;
  for (n = 0; fgets (buf, 4096, fp) && n < nlines;)
    {
      char *package = strtok (buf, " ");
      if (!package || !*package || !match_argv (argv, package))
	continue;
      for (int i = 0; i < 2; i++)
	{
	  fileparse f;
	  char *tar = strtok (NULL, " ");
	  if (!tar || !*tar || !parse_filename (tar, f))
	    break;

	  int len = strlen (package);
	  if (f.what[0])
	    len += strlen (f.what) + 1;
	  packages[n].name = (char *) malloc (len + 1);
	  strcpy (packages[n].name , package);
	  if (f.what[0])
	    strcat (strcat (packages[n].name, "-"), f.what);
	  packages[n].ver = strdup (f.ver);
	  n++;
	  if (strtok (NULL, " ") == NULL)
	    break;
	}
    }

  qsort (packages, n, sizeof (packages[0]), compar);

  for (int i = 0; i < n; i++)
    printf ("%-*s%-*s\n", package_len, packages[i].name,
			  version_len, packages[i].ver);
  fclose (fp);

  return;

err:
  puts ("No setup information found");
  if (fp)
    fclose (fp);
  return;
}
