/* getfacl.c

   Copyright 2000, 2001 Red Hat Inc.

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
#include <sys/types.h>
#include <sys/acl.h>
#include <sys/stat.h>
#include <string.h>

char *
permstr (mode_t perm)
{
  static char pbuf[4];

  pbuf[0] = (perm & S_IREAD) ? 'r' : '-';
  pbuf[1] = (perm & S_IWRITE) ? 'w' : '-';
  pbuf[2] = (perm & S_IEXEC) ? 'x' : '-';
  pbuf[3] = '\0';
  return pbuf;
}

const char *
username (uid_t uid)
{
  static char ubuf[256];
  struct passwd *pw;

  if ((pw = getpwuid (uid)))
    strcpy (ubuf, pw->pw_name);
  else
    sprintf (ubuf, "%d <unknown>", uid);
  return ubuf;
}

const char *
groupname (gid_t gid)
{
  static char gbuf[256];
  struct group *gr;

  if ((gr = getgrgid (gid)))
    strcpy (gbuf, gr->gr_name);
  else
    sprintf (gbuf, "%d <unknown>", gid);
  return gbuf;
}

#define pn(txt)	fprintf (fp, txt "\n", name)
#define p(txt)	fprintf (fp, txt "\n")

int
usage (const char *name, int help)
{
  FILE *fp = help ? stdout : stderr;

  pn ("usage: %s [-adn] file...");
  if (!help)
    pn ("Try `%s --help' for more information.");
  else
    {
      p ("");
      p ("Display file and directory access control lists (ACLs).");
      p ("");
      p ("For each argument that is a regular file, special file or");
      p ("directory, getfacl displays the owner, the group, and the ACL.");
      p ("For directories getfacl displays additionally the default ACL.");
      p ("");
      p ("With no options specified, getfacl displays the filename, the");
      p ("owner, the group, and both the ACL and the default ACL, if it");
      p ("exists.");
      p ("");
      p ("The following options are supported:");
      p ("");
      p ("-a   Display the filename, the owner, the group, and the ACL");
      p ("     of the file.");
      p ("");
      p ("-d   Display the filename, the owner, the group, and the default");
      p ("     ACL of the directory, if it exists.");
      p ("");
      p ("-n   Display user and group IDs instead of names.");
      p ("");
      p ("The format for ACL output is as follows:");
      p ("     # file: filename");
      p ("     # owner: name or uid");
      p ("     # group: name or uid");
      p ("     user::perm");
      p ("     user:name or uid:perm");
      p ("     group::perm");
      p ("     group:name or gid:perm");
      p ("     mask:perm");
      p ("     other:perm");
      p ("     default:user::perm");
      p ("     default:user:name or uid:perm");
      p ("     default:group::perm");
      p ("     default:group:name or gid:perm");
      p ("     default:mask:perm");
      p ("     default:other:perm");
      p ("");
      p ("When multiple files are specified on the command line, a blank");
      p ("line separates the ACLs for each file.");
    }
  return 1;
}

struct option longopts[] = {
  {"help", no_argument, NULL, 'h'},
  {0, no_argument, NULL, 0}
};

int
main (int argc, char **argv)
{
  extern int optind;
  int c, i;
  int aopt = 0;
  int dopt = 0;
  int nopt = 0;
  int first = 1;
  struct stat st;
  aclent_t acls[MAX_ACL_ENTRIES];

  while ((c = getopt_long (argc, argv, "adn", longopts, NULL)) != EOF)
    switch (c)
      {
      case 'a':
	aopt = 1;
	break;
      case 'd':
	dopt = 1;
	break;
      case 'n':
	nopt = 1;
	break;
      case 'h':
        return usage (argv[0], 1);
      default:
	return usage (argv[0], 0);
      }
  if (optind > argc - 1)
    return usage (argv[0], 0);
  while ((c = optind++) < argc)
    {
      if (stat (argv[c], &st))
	{
	  perror (argv[0]);
	  continue;
	}
      if (!first)
	putchar ('\n');
      first = 0;
      printf ("# file: %s\n", argv[c]);
      if (nopt)
        {
	  printf ("# owner: %d\n", st.st_uid);
	  printf ("# group: %d\n", st.st_gid);
	}
      else
        {
	  printf ("# owner: %s\n", username (st.st_uid));
	  printf ("# group: %s\n", groupname (st.st_gid));
	}
      if ((c = acl (argv[c], GETACL, MAX_ACL_ENTRIES, acls)) < 0)
	{
	  perror (argv[0]);
	  continue;
	}
      for (i = 0; i < c; ++i)
	{
	  if (acls[i].a_type & ACL_DEFAULT)
	    {
	      if (aopt)
		continue;
	      printf ("default:");
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
		printf ("user:%d:", acls[i].a_id);
	      else
		printf ("user:%s:", username (acls[i].a_id));
	      break;
	    case GROUP_OBJ:
	      printf ("group::");
	      break;
	    case GROUP:
	      if (nopt)
		printf ("group:%d:", acls[i].a_id);
	      else
		printf ("group:%s:", groupname (acls[i].a_id));
	      break;
	    case CLASS_OBJ:
	      printf ("mask::");
	      break;
	    case OTHER_OBJ:
	      printf ("other::");
	      break;
	    }
	  printf ("%s\n", permstr (acls[i].a_perm));
	}
    }
  return 0;
}
