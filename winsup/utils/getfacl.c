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
#include <cygwin/acl.h>
#include <sys/stat.h>
#include <cygwin/version.h>
#include <string.h>
#include <errno.h>

static char *prog_name;

char *
permstr (mode_t perm)
{
  static char pbuf[4];

  pbuf[0] = (perm & S_IROTH) ? 'r' : '-';
  pbuf[1] = (perm & S_IWOTH) ? 'w' : '-';
  pbuf[2] = (perm & S_IXOTH) ? 'x' : '-';
  pbuf[3] = '\0';
  return pbuf;
}

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
	"     mask:perm\n"
	"     other:perm\n"
	"     default:user::perm\n"
	"     default:user:name or uid:perm\n"
	"     default:group::perm\n"
	"     default:group:name or gid:perm\n"
	"     default:mask:perm\n"
	"     default:other:perm\n"
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
  int istty = isatty (fileno (stdout));
  struct stat st;
  aclent_t acls[MAX_ACL_ENTRIES];

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
  for (; optind < argc; ++optind)
    {
      int i, num_acls;
      mode_t mask = S_IRWXO, def_mask = S_IRWXO;

      if (stat (argv[optind], &st)
	  || (num_acls = acl (argv[optind], GETACL, MAX_ACL_ENTRIES, acls)) < 0)
	{
	  fprintf (stderr, "%s: %s: %s\n",
		   prog_name, argv[optind], strerror (errno));
	  ret = 2;
	  continue;
	}
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
      for (i = 0; i < num_acls; ++i)
	{
	  if (acls[i].a_type == CLASS_OBJ)
	    mask = acls[i].a_perm;
	  else if (acls[i].a_type == DEF_CLASS_OBJ)
	    def_mask = acls[i].a_perm;
	}
      for (i = 0; i < num_acls; ++i)
	{
	  int n = 0;
	  int print_effective = 0;
	  mode_t effective = acls[i].a_perm;

	  if (acls[i].a_type & ACL_DEFAULT)
	    {
	      if (aopt)
		continue;
	      n += printf ("default:");
	    }
	  else if (dopt)
	    continue;
	  switch (acls[i].a_type & ~ACL_DEFAULT)
	    {
	    case USER_OBJ:
	      printf ("user::");
	      break;
	    case USER:
	      if (nopt)
		n += printf ("user:%lu:", (unsigned long)acls[i].a_id);
	      else
		n += printf ("user:%s:", username (acls[i].a_id));
	      break;
	    case GROUP_OBJ:
	      n += printf ("group::");
	      break;
	    case GROUP:
	      if (nopt)
		n += printf ("group:%lu:", (unsigned long)acls[i].a_id);
	      else
		n += printf ("group:%s:", groupname (acls[i].a_id));
	      break;
	    case CLASS_OBJ:
	      printf ("mask:");
	      break;
	    case OTHER_OBJ:
	      printf ("other:");
	      break;
	    }
	  n += printf ("%s", permstr (acls[i].a_perm));
	  switch (acls[i].a_type)
	    {
	    case USER:
	    case GROUP_OBJ:
	      effective = acls[i].a_perm & mask;
	      print_effective = 1;
	      break;
	    case GROUP:
	      /* Special case SYSTEM and Admins group:  The mask only
	         applies to them as far as the execute bit is concerned. */
	      if (acls[i].a_id == 18 || acls[i].a_id == 544)
		effective = acls[i].a_perm & (mask | S_IROTH | S_IWOTH);
	      else
		effective = acls[i].a_perm & mask;
	      print_effective = 1;
	      break;
	    case DEF_USER:
	    case DEF_GROUP_OBJ:
	      effective = acls[i].a_perm & def_mask;
	      print_effective = 1;
	      break;
	    case DEF_GROUP:
	      /* Special case SYSTEM and Admins group:  The mask only
	         applies to them as far as the execute bit is concerned. */
	      if (acls[i].a_id == 18 || acls[i].a_id == 544)
		effective = acls[i].a_perm & (def_mask | S_IROTH | S_IWOTH);
	      else
		effective = acls[i].a_perm & def_mask;
	      print_effective = 1;
	      break;
	    }
	  if (print_effective && eopt >= 0
	      && (eopt > 0 || effective != acls[i].a_perm))
	    {
	      if (istty)
		{
		  n = 40 - n;
		  if (n <= 0)
		    n = 1;
		  printf ("%*s", n, " ");
		}
	      else
	        putchar ('\t');
	      printf ("#effective:%s", permstr (effective));
	    }
	  putchar ('\n');
	}
      putchar ('\n');
    }
  return ret;
}
