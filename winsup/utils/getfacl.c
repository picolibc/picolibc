
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/acl.h>
#include <sys/stat.h>

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

#if 0
char *
username (uid_t uid)
{
  static char ubuf[256];
  struct passwd *pw;

  if (pw = getpwuid (uid))
    strcpy (ubuf, pw->pw_name);
  else
    strcpy (ubuf, "<unknown>");
}

char *
groupname (gid_t gid)
{
  static char gbuf[256];
  struct group *gr;

  if (gr = getgruid (gid))
    strcpy (gbuf, gr->gr_name);
  else
    strcpy (gbuf, "<unknown>");
}
#endif

int
main (int argc, char **argv)
{
  extern int optind;
  int c, i;
  int aopt = 0;
  int dopt = 0;
  int first = 1;
  struct stat st;
  aclent_t acls[MAX_ACL_ENTRIES];

  while ((c = getopt (argc, argv, "ad")) != EOF)
    switch (c)
      {
      case 'a':
	aopt = 1;
	break;
      case 'd':
	dopt = 1;
	break;
      default:
	fprintf (stderr, "usage: %s [-ad] file...\n", argv[0]);
	return 1;
      }
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
      printf ("# owner: %d\n", st.st_uid);
      printf ("# group: %d\n", st.st_gid);
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
	      printf ("user:%d:", acls[i].a_id);
	      break;
	    case GROUP_OBJ:
	      printf ("group::");
	      break;
	    case GROUP:
	      printf ("group:%d:", acls[i].a_id);
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
