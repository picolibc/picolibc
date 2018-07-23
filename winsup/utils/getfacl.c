/* getfacl.c

   Written by Corinna Vinschen <vinschen@redhat.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/acl.h>
#include <acl/libacl.h>
#include <sys/stat.h>
#include <cygwin/version.h>
#include <string.h>
#include <errno.h>

static char *prog_name;

const char *
username (uid_t uid)
{
  static char ubuf[256];
  struct passwd *pw;

  if ((pw = getpwuid (uid)))
    snprintf (ubuf, sizeof ubuf, "%s", pw->pw_name);
  else
    sprintf (ubuf, "%lu <unknown>", (unsigned long)uid);
  return ubuf;
}

const char *
groupname (gid_t gid)
{
  static char gbuf[256];
  struct group *gr;

  if ((gr = getgrgid (gid)))
    snprintf (gbuf, sizeof gbuf, "%s", gr->gr_name);
  else
    sprintf (gbuf, "%lu <unknown>", (unsigned long)gid);
  return gbuf;
}

static void
usage (FILE * stream)
{
  fprintf (stream, "Usage: %s [-adn] FILE [FILE2...]\n"
	"\n"
	"Display file and directory access control lists (ACLs).\n"
	"\n"
	"  -a, --access        display the file access control list only\n"
	"  -d, --default       display the default access control list only\n"
	"  -c, --omit-header   do not display the comment header\n"
	"  -e, --all-effective print all effective rights\n"
	"  -E, --no-effective  print no effective rights\n"
	"  -n, --numeric       print numeric user/group identifiers\n"
	"  -V, --version       print version and exit\n"
	"  -h, --help          this help text\n"
	"\n"
	"When multiple files are specified on the command line, a blank\n"
	"line separates the ACLs for each file.\n", prog_name);
  if (stream == stdout)
    {
      fprintf (stream, ""
	"For each argument that is a regular file, special file or\n"
	"directory, getfacl displays the owner, the group, and the ACL.\n"
	"For directories getfacl displays additionally the default ACL.\n"
	"\n"
	"With no options specified, getfacl displays the filename, the\n"
	"owner, the group, the setuid (s), setgid (s), and sticky (t)\n"
	"bits if available, and both the ACL and the default ACL, if it\n"
	"exists.\n"
	"\n"
	"The format for ACL output is as follows:\n"
	"     # file: filename\n"
	"     # owner: name or uid\n"
	"     # group: name or uid\n"
	"     # flags: sst\n"
	"     user::perm\n"
	"     user:name or uid:perm\n"
	"     group::perm\n"
	"     group:name or gid:perm\n"
	"     mask::perm\n"
	"     other::perm\n"
	"     default:user::perm\n"
	"     default:user:name or uid:perm\n"
	"     default:group::perm\n"
	"     default:group:name or gid:perm\n"
	"     default:mask::perm\n"
	"     default:other::perm\n"
	"\n");
    }
}

struct option longopts[] = {
  {"access", no_argument, NULL, 'a'},
  {"all", no_argument, NULL, 'a'},
  {"omit-header", no_argument, NULL, 'c'},
  {"all-effective", no_argument, NULL, 'e'},
  {"no-effective", no_argument, NULL, 'E'},
  {"default", no_argument, NULL, 'd'},
  {"dir", no_argument, NULL, 'd'},
  {"help", no_argument, NULL, 'h'},
  {"noname", no_argument, NULL, 'n'},	/* Backward compat */
  {"numeric", no_argument, NULL, 'n'},
  {"version", no_argument, NULL, 'V'},
  {0, no_argument, NULL, 0}
};
const char *opts = "acdeEhnV";

static void
print_version ()
{
  printf ("getfacl (cygwin) %d.%d.%d\n"
	  "Get POSIX ACL information\n"
	  "Copyright (C) 2000 - %s Cygwin Authors\n"
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
  int c;
  int ret = 0;
  int aopt = 0;
  int copt = 0;
  int eopt = 0;
  int dopt = 0;
  int nopt = 0;
  int options = 0;
  int istty = isatty (fileno (stdout));
  struct stat st;

  prog_name = program_invocation_short_name;

  while ((c = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (c)
      {
      case 'a':
	aopt = 1;
	break;
      case 'c':
	copt = 1;
	break;
      case 'd':
	dopt = 1;
	break;
      case 'e':
	eopt = 1;
	break;
      case 'E':
	eopt = -1;
	break;
      case 'h':
	usage (stdout);
	return 0;
      case 'n':
	nopt = 1;
	break;
      case 'V':
	print_version ();
	return 0;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n", prog_name);
	return 1;
      }
  if (optind > argc - 1)
    {
      usage (stderr);
      return 1;
    }
  if (nopt)
    options |= TEXT_NUMERIC_IDS;
  if (eopt > 0)
    options |= TEXT_ALL_EFFECTIVE;
  else if (!eopt)
    options |= TEXT_SOME_EFFECTIVE;
  if (istty)
    options |= TEXT_SMART_INDENT;
  for (; optind < argc; ++optind)
    {
      acl_t access_acl = NULL, default_acl = NULL;
      char *access_txt, *default_txt;

      if (stat (argv[optind], &st)
	  || (!dopt
	      && !(access_acl = acl_get_file (argv[optind], ACL_TYPE_ACCESS)))
	  || (!aopt && S_ISDIR (st.st_mode)
	      && !(default_acl = acl_get_file (argv[optind],
					       ACL_TYPE_DEFAULT))))
	goto err;
      if (!copt)
	{
	  printf ("# file: %s\n", argv[optind]);
	  if (nopt)
	    {
	      printf ("# owner: %lu\n", (unsigned long)st.st_uid);
	      printf ("# group: %lu\n", (unsigned long)st.st_gid);
	    }
	  else
	    {
	      printf ("# owner: %s\n", username (st.st_uid));
	      printf ("# group: %s\n", groupname (st.st_gid));
	    }
	  if (st.st_mode & (S_ISUID | S_ISGID | S_ISVTX))
	    printf ("# flags: %c%c%c\n", (st.st_mode & S_ISUID) ? 's' : '-',
					 (st.st_mode & S_ISGID) ? 's' : '-',
					 (st.st_mode & S_ISVTX) ? 't' : '-');
	}
      if (access_acl)
	{
	  if (!(access_txt = acl_to_any_text (access_acl, NULL, '\n', options)))
	    {
	      acl_free (access_acl);
	      goto err;
	    }
	  printf ("%s\n", access_txt);
	  acl_free (access_txt);
	  acl_free (access_acl);
	}
      if (default_acl)
	{
	  if (!(default_txt = acl_to_any_text (default_acl, "default:",
					       '\n', options)))
	    {
	      acl_free (default_acl);
	      goto err;
	    }
	  printf ("%s\n", default_txt);
	  acl_free (default_txt);
	  acl_free (default_acl);
	}
      putchar ('\n');
      continue;
    err:
      fprintf (stderr, "%s: %s: %s\n\n",
	       prog_name, argv[optind], strerror (errno));
      ret = 2;
    }
  return ret;
}
