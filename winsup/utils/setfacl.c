/* setfacl.c

   Copyright 2000, 2001 Red Hat Inc.

   Written by Corinna Vinschen <vinschen@redhat.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/acl.h>

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef ILLEGAL_MODE
#define ILLEGAL_MODE ((mode_t)0xffffffff)
#endif

typedef enum {
  NoAction,
  Set,
  Modify,
  Delete,
  ModNDel,
  SetFromFile
} action_t;

char *myname;

mode_t getperm (char *in)
{
  if (isdigit (*in) && !in[1])
    {
      int i = atoi (in);
      if (i < 0 || i > 7)
        return ILLEGAL_MODE;
      return i << 6 | i << 3 | i;
    }
  if (strlen (in) > 3 && strchr (" \t\n\r#", in[3]))
    in[3] = '\0';
  if (strlen (in) != 3)
    return ILLEGAL_MODE;
  if (!strchr ("r-", in[0])
      || !strchr ("w-", in[1])
      || !strchr ("x-", in[2]))
    return ILLEGAL_MODE;
  return (in[0] == 'r' ? S_IRUSR | S_IRGRP | S_IROTH : 0)
         | (in[1] == 'w' ? S_IWUSR | S_IWGRP | S_IWOTH : 0)
         | (in[2] == 'x' ? S_IXUSR | S_IXGRP | S_IXOTH : 0);
}

BOOL
getaclentry (action_t action, char *c, aclent_t *ace)
{
  char *c2;

  ace->a_type = 0;
  ace->a_id = 0;
  ace->a_perm = 0;

  if (!strncmp (c, "default:", 8)
      || !strncmp (c, "d:", 2))
    {
      ace->a_type = ACL_DEFAULT;
      c = strchr (c, ':') + 1;
    }
  if (!strncmp (c, "user:", 5)
      || !strncmp (c, "u:", 2))
    {
      ace->a_type |= USER_OBJ;
      c = strchr (c, ':') + 1;
    }
  else if (!strncmp (c, "group:", 6)
           || !strncmp (c, "g:", 2))
    {
      ace->a_type |= GROUP_OBJ;
      c = strchr (c, ':') + 1;
    }
  else if (!strncmp (c, "mask:", 5)
           || !strncmp (c, "m:", 2))
    {
      ace->a_type |= CLASS_OBJ;
      c = strchr (c, ':') + 1;
    }
  else if (!strncmp (c, "other:", 6)
           || !strncmp (c, "o:", 2))
    {
      ace->a_type |= OTHER_OBJ;
      c = strchr (c, ':') + 1;
    }
  else
    return FALSE;
  if (ace->a_type & (USER_OBJ | GROUP_OBJ))
    {
      if ((c2 = strchr (c, ':')))
        {
          if (action == Delete)
            return FALSE;
          *c2 = '\0';
        }
      else if (action != Delete)
        return FALSE;
      if (c2 == c)
        {
          if (action == Delete)
            return FALSE;
        }
      else if (isdigit (*c))
        {
          char *c3;

          ace->a_id = strtol (c, &c3, 10);
          if (*c3)
            return FALSE;
        }
      else if (ace->a_type & USER_OBJ)
        {
          struct passwd *pw = getpwnam (c);
          if (!pw)
            return FALSE;
          ace->a_id = pw->pw_uid;
        }
      else
        {
          struct group *gr = getgrnam (c);
          if (!gr)
            return FALSE;
          ace->a_id = gr->gr_gid;
        }
      if (c2 != c)
        {
	  if (ace->a_type & USER_OBJ)
	    {
	      ace->a_type &= ~USER_OBJ;
	      ace->a_type |= USER;
	    }
	  else
	    {
	      ace->a_type &= ~GROUP_OBJ;
	      ace->a_type |= GROUP;
	    }
	}
      if (c2)
        c = c2 + 1;
    }
  else if (*c++ != ':')
    return FALSE;
  if (action == Delete)
    {
      if ((ace->a_type & (CLASS_OBJ | OTHER_OBJ))
          && *c)
        return FALSE;
      ace->a_perm = ILLEGAL_MODE;
      return TRUE;
    }
  if ((ace->a_perm = getperm (c)) == ILLEGAL_MODE)
    return FALSE;
  return TRUE;
}

BOOL
getaclentries (action_t action, char *buf, aclent_t *acls, int *idx)
{
  char *c;

  if (action == SetFromFile)
    {
      FILE *fp;
      char fbuf[256], *fb;

      if (!strcmp (buf, "-"))
        fp = stdin;
      else if (! (fp = fopen (buf, "r")))
        return FALSE;
      while ((fb = fgets (fbuf, 256, fp)))
        {
	  while (strchr (" \t", *fb))
	    ++fb;
	  if (strchr ("\n\r#", *fb))
	    continue;
          if (!getaclentry (action, fb, acls + (*idx)++))
            {
              fclose (fp);
              return FALSE;
            }
        }
      if (fp != stdin)
	fclose (fp);
    }
  else
    for (c = strtok (buf, ","); c; c = strtok (NULL, ","))
      if (!getaclentry (action, c, acls + (*idx)++))
        return FALSE;
  return TRUE;
}

int
searchace (aclent_t *aclp, int nentries, int type, int id)
{
  int i;

  for (i = 0; i < nentries; ++i)
    if ((aclp[i].a_type == type && (id < 0 || aclp[i].a_id == id))
        || !aclp[i].a_type)
      return i;
  return -1;
}

int
modacl (aclent_t *tgt, int tcnt, aclent_t *src, int scnt)
{
  int t, s, i;

  for (s = 0; s < scnt; ++s)
    {
      t = searchace (tgt, MAX_ACL_ENTRIES, src[s].a_type,
                     (src[s].a_type & (USER | GROUP)) ? src[s].a_id : -1);
      if (t < 0)
        return -1;
      if (src[s].a_perm == ILLEGAL_MODE)
        {
          if (t < tcnt)
            {
              for (i = t + 1; i < tcnt; ++i)
                tgt[i - 1] = tgt[i];
              --tcnt;
            }
        }
      else
        {
          tgt[t] = src[s];
          if (t >= tcnt)
            ++tcnt;
        }
    }
  return tcnt;
}

void
setfacl (action_t action, char *path, aclent_t *acls, int cnt)
{
  aclent_t lacl[MAX_ACL_ENTRIES];
  int lcnt;

  memset (lacl, 0, sizeof lacl);
  if (action == Set)
    {
      if (acl (path, SETACL, cnt, acls))
        perror (myname);
      return;
    }
  if ((lcnt = acl (path, GETACL, MAX_ACL_ENTRIES, lacl)) < 0
      || (lcnt = modacl (lacl, lcnt, acls, cnt)) < 0
      || (lcnt = acl (path, SETACL, lcnt, lacl)) < 0)
    perror (myname);
}

#define pn(txt)	fprintf (fp, txt "\n", myname)
#define p(txt)	fprintf (fp, txt "\n")

int
usage (int help)
{
  FILE *fp = help ? stdout : stderr;

  pn ("usage: %s [-r] -s acl_entries file...");
  pn ("       %s [-r] -md acl_entries file...");
  pn ("       %s [-r] -f acl_file file...");
  if (!help)
    pn ("Try `%s --help' for more information.");
  else
    {
      p ("");
      p ("Modify file and directory access control lists (ACLs)");
      p ("");
      pn ("For each file given as parameter, %s will either replace its");
      p ("complete ACL (-s, -f)), or it will add, modify, or delete ACL");
      p ("entries.");
      p ("");
      p ("The following options are supported:");
      p ("");
      p ("-s   Substitute the ACL of the file by the entries specified on");
      p ("     the command line.  Required entries are");
      p ("     - One user entry for the owner of the file.");
      p ("     - One group entry for the group of the file.");
      p ("     - One other entry.");
      p ("     If additional user and group entries are given:");
      p ("     - A mask entry for the file group class of the file.");
      p ("     - No duplicate user or group entries with the same uid/gid.");
      p ("     If it is a directory:");
      p ("     - One default user entry for the owner of the file.");
      p ("     - One default group entry for the group of the file.");
      p ("     - One default mask entry for the file group class of the file.");
      p ("     - One default other entry.");
      p ("");
      p ("     Acl_entries are one or more comma-separated ACL entries from");
      p ("     the following list:");
      p ("");
      p ("         u[ser]::perm");
      p ("         u[ser]:uid:perm");
      p ("         g[roup]::perm");
      p ("         g[roup]:gid:perm");
      p ("         m[ask]::perm");
      p ("         o[ther]::perm");
      p ("");
      p ("     Default entries are like the above with the trailing default");
      p ("     identifier.  E.g.");
      p ("");
      p ("         d[efault]:u[ser]:uid:perm");
      p ("");
      p ("     `perm' is either a 3-char permissions string in the form");
      p ("     \"rwx\" with the character - for not setting a permission");
      p ("     or it is the octal representation of the permissions, a");
      p ("     value from 0 (equivalent to \"---\") to 7 (\"rwx\").");
      p ("     `uid' is a user name or a numerical uid.");
      p ("     `gid' is a group name or a numerical gid.");
      p ("");
      p ("-f   Like -s but take the ACL entries from `acl_file'.  Acl_entries");
      p ("     are given one per line.  Whitespace characters are ignored,");
      p ("     the character \"#\" may be used to start a comment.  The");
      p ("     special filename \"-\" indicates reading from stdin.");
      p ("");
      p ("-m   Add or modify one or more specified ACL entries.  Acl_entries");
      p ("     is a comma-separated list of entries from the same list as");
      p ("     above.");
      p ("");
      p ("-d   Delete one or more specified entries from the file's ACL.");
      p ("     The owner, group and others entries must not be deleted");
      p ("     Acl_entries are one or more comma-separated ACL entries");
      p ("     without permissions, taken from the following list:");
      p ("");
      p ("         u[ser]:uid");
      p ("         g[roup]:gid");
      p ("         d[efault]:u[ser]:uid");
      p ("         d[efault]:g[roup]:gid");
      p ("         d[efault]:m[ask]:");
      p ("         d[efault]:o[ther]:");
      p ("");
      p ("-r   Causes the permissions specified in the mask entry to be");
      p ("     ignored and replaced by the maximum permissions needed for");
      p ("     the file group class.");
      p ("");
      p ("While the -m and -d options may be used in the same command, the");
      p ("-s and -f options may be used only exclusively.");
      p ("");
      p ("Directories may contain default ACL entries.  Files created");
      p ("in a directory that contains default ACL entries will have");
      p ("permissions according to the combination of the current umask,");
      p ("the explicit permissions requested and the default ACL entries");
      p ("Note: Under Cygwin, the default ACL entries are not taken into");
      p ("account currently.");
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
  extern char *optarg;
  extern int optind;
  int c;
  action_t action = NoAction;
  int ropt = 0;
  aclent_t acls[MAX_ACL_ENTRIES];
  int aclidx = 0;

  myname = argv[0];
  memset (acls, 0, sizeof acls);
  while ((c = getopt_long (argc, argv, "d:f:m:rs:", longopts, NULL)) != EOF)
    switch (c)
      {
      case 'd':
        if (action == NoAction)
          action = Delete;
        else if (action == Modify)
          action = ModNDel;
        else
          return usage (0);
        if (! getaclentries (Delete, optarg, acls, &aclidx))
          {
            fprintf (stderr, "%s: illegal acl entries\n", myname);
            return 2;
          }
        break;
      case 'f':
        if (action == NoAction)
          action = Set;
        else
          return usage (0);
        if (! getaclentries (SetFromFile, optarg, acls, &aclidx))
          {
            fprintf (stderr, "%s: illegal acl entries\n", myname);
            return 2;
          }
        break;
      case 'm':
        if (action == NoAction)
          action = Modify;
        else if (action == Delete)
          action = ModNDel;
        else
          return usage (0);
        if (! getaclentries (Modify, optarg, acls, &aclidx))
          {
            fprintf (stderr, "%s: illegal acl entries\n", myname);
            return 2;
          }
        break;
      case 'r':
        if (!ropt)
          ropt = 1;
        else
          return usage (0);
        break;
      case 's':
        if (action == NoAction)
          action = Set;
        else
          return usage (0);
        break;
        if (! getaclentries (Set, optarg, acls, &aclidx))
          {
            fprintf (stderr, "%s: illegal acl entries\n", myname);
            return 2;
          }
        break;
      case 'h':
        return usage (1);
      default:
        return usage (0);
      }
  if (action == NoAction)
    return usage (0);
  if (optind > argc - 1)
    return usage (0);
  if (action == Set)
    switch (aclcheck (acls, aclidx, NULL))
      {
      case GRP_ERROR:
        fprintf (stderr, "%s: more than one group entry.\n", myname);
        return 2;
      case USER_ERROR:
        fprintf (stderr, "%s: more than one user entry.\n", myname);
        return 2;
      case CLASS_ERROR:
        fprintf (stderr, "%s: more than one mask entry.\n", myname);
        return 2;
      case OTHER_ERROR:
        fprintf (stderr, "%s: more than one other entry.\n", myname);
        return 2;
      case DUPLICATE_ERROR:
        fprintf (stderr, "%s: duplicate additional user or group.\n", myname);
        return 2;
      case ENTRY_ERROR:
        fprintf (stderr, "%s: invalid entry type.\n", myname);
        return 2;
      case MISS_ERROR:
        fprintf (stderr, "%s: missing entries.\n", myname);
        return 2;
      case MEM_ERROR:
        fprintf (stderr, "%s: out of memory.\n", myname);
        return 2;
      default:
        break;
      }
  for (c = optind; c < argc; ++c)
    setfacl (action, argv[c], acls, aclidx);
  return 0;
}
