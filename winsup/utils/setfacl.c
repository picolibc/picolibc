/* setfacl.c

   Copyright 2000, 2001, 2002, 2003 Red Hat Inc.

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

static const char version[] = "$Revision$";
static char *prog_name;

typedef enum {
  NoAction,
  Set,
  Modify,
  Delete,
  ModNDel,
  SetFromFile
} action_t;

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
  return (in[0] == 'r' ? S_IROTH : 0)
         | (in[1] == 'w' ? S_IWOTH : 0)
         | (in[2] == 'x' ? S_IXOTH : 0);
}

BOOL
getaclentry (action_t action, char *c, aclent_t *ace)
{
  char *c2;

  ace->a_type = 0;
  ace->a_id = -1;
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
  /* FIXME: currently allow both :: and : */
  else if (*c == ':')
    c++;
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

int
setfacl (action_t action, char *path, aclent_t *acls, int cnt)
{
  aclent_t lacl[MAX_ACL_ENTRIES];
  int lcnt;

  memset (lacl, 0, sizeof lacl);
  if (action == Set)
    {
      if (acl (path, SETACL, cnt, acls))
	{
	  perror (prog_name);
	  return 2;
	}
    }
  else if ((lcnt = acl (path, GETACL, MAX_ACL_ENTRIES, lacl)) < 0
      || (lcnt = modacl (lacl, lcnt, acls, cnt)) < 0
      || (lcnt = acl (path, SETACL, lcnt, lacl)) < 0)
    {
      perror (prog_name);
      return 2;
    }
  return 0;
}

static void
usage (FILE * stream)
{
  fprintf (stream, ""
            "Usage: %s [-r] (-f ACL_FILE | -s acl_entries) FILE...\n"
            "       %s [-r] ([-d acl_entries] [-m acl_entries]) FILE...\n"
            "Modify file and directory access control lists (ACLs)\n"
            "\n"
            "  -d, --delete     delete one or more specified ACL entries\n"
            "  -f, --file       set ACL entries for FILE to ACL entries read\n"
            "                   from a ACL_FILE\n"
            "  -m, --modify     modify one or more specified ACL entries\n"
            "  -r, --replace    replace mask entry with maximum permissions\n"
            "                   needed for the file group class\n"
            "  -s, --substitute substitute specified ACL entries for the\n"
            "                   ACL of FILE\n"
            "  -h, --help       output usage information and exit\n"
            "  -v, --version    output version information and exit\n"
            "\n"
            "At least one of (-d, -f, -m, -s) must be specified\n"
            "\n"
            "", prog_name, prog_name);
  if (stream == stdout) 
  {
    printf(""
            "     Acl_entries are one or more comma-separated ACL entries \n"
            "     from the following list:\n"
            "\n"
            "         u[ser]::perm\n"
            "         u[ser]:uid:perm\n"
            "         g[roup]::perm\n"
            "         g[roup]:gid:perm\n"
            "         m[ask]:perm\n"
            "         o[ther]:perm\n"
            "\n"
            "     Default entries are like the above with the additional\n"
            "     default identifier. For example: \n"
            "\n"
            "         d[efault]:u[ser]:uid:perm\n"
            "\n"
            "     'perm' is either a 3-char permissions string in the form\n"
            "     \"rwx\" with the character - for no permission\n"
            "     or it is the octal representation of the permissions, a\n"
            "     value from 0 (equivalent to \"---\") to 7 (\"rwx\").\n"
            "     'uid' is a user name or a numerical uid.\n"
            "     'gid' is a group name or a numerical gid.\n"
            "\n"
            "\n"
            "For each file given as parameter, %s will either replace its\n"
            "complete ACL (-s, -f), or it will add, modify, or delete ACL\n"
            "entries.\n"
            "\n"
            "The following options are supported:\n"
            "\n"
            "-d   Delete one or more specified entries from the file's ACL.\n"
            "     The owner, group and others entries must not be deleted.\n"
            "     Acl_entries to be deleted should be specified without\n"
            "     permissions, as in the following list:\n"
            "\n"
            "         u[ser]:uid\n"
            "         g[roup]:gid\n"
            "         d[efault]:u[ser]:uid\n"
            "         d[efault]:g[roup]:gid\n"
            "         d[efault]:m[ask]:\n"
            "         d[efault]:o[ther]:\n"
            "\n"
            "-f   Take the Acl_entries from ACL_FILE one per line. Whitespace\n"
            "     characters are ignored, and the character \"#\" may be used\n"
            "     to start a comment.  The special filename \"-\" indicates\n"
            "     reading from stdin.\n"
            "     Required entries are\n"
            "     - One user entry for the owner of the file.\n"
            "     - One group entry for the group of the file.\n"
            "     - One other entry.\n"
            "     If additional user and group entries are given:\n"
            "     - A mask entry for the file group class of the file.\n"
            "     - No duplicate user or group entries with the same uid/gid.\n"
            "     If it is a directory:\n"
            "     - One default user entry for the owner of the file.\n"
            "     - One default group entry for the group of the file.\n"
            "     - One default mask entry for the file group class.\n"
            "     - One default other entry.\n"
            "\n"
            "-m   Add or modify one or more specified ACL entries.\n"
            "     Acl_entries is a comma-separated list of entries from the \n"
            "     same list as above.\n"
            "\n"
            "-r   Causes the permissions specified in the mask entry to be\n"
            "     ignored and replaced by the maximum permissions needed for\n"
            "     the file group class.\n"
            "\n"
            "-s   Like -f, but substitute the file's ACL with Acl_entries\n" 
            "     specified in a comma-separated list on the command line.\n"
            "\n"
            "While the -d and -m options may be used in the same command, the\n"
            "-f and -s options may be used only exclusively.\n"
            "\n"
            "Directories may contain default ACL entries.  Files created\n"
            "in a directory that contains default ACL entries will have\n"
            "permissions according to the combination of the current umask,\n"
            "the explicit permissions requested and the default ACL entries\n"
            "Note: Under Cygwin, the default ACL entries are not taken into\n"
            "account currently.\n", prog_name);
  }
  else
    fprintf(stream, "Try '%s --help' for more information.\n", prog_name);
}

struct option longopts[] = {
  {"delete", required_argument, NULL, 'd'},
  {"file", required_argument, NULL, 'f'},
  {"modify", required_argument, NULL, 'm'},
  {"replace", no_argument, NULL, 'r'},
  {"substitute", required_argument, NULL, 's'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {0, no_argument, NULL, 0}
};

static void
print_version ()
{
  const char *v = strchr (version, ':');
  int len;
  if (!v)
    {
      v = "?";
      len = 1;
    }
  else
    {
      v += 2;
      len = strchr (v, ' ') - v;
    }
  printf ("\
setfacl (cygwin) %.*s\n\
ACL Modification Utility\n\
Copyright 2000, 2001, 2002 Red Hat, Inc.\n\
Compiled on %s\n\
", len, v, __DATE__);
}

int
main (int argc, char **argv)
{
  int c;
  action_t action = NoAction;
  int ropt = 0;
  aclent_t acls[MAX_ACL_ENTRIES];
  int aclidx = 0;
  int ret = 0;

  prog_name = strrchr (argv[0], '/');
  if (prog_name == NULL)
    prog_name = strrchr (argv[0], '\\');
  if (prog_name == NULL)
    prog_name = argv[0];
  else
    prog_name++;

  memset (acls, 0, sizeof acls);
  while ((c = getopt_long (argc, argv, "d:f:hm:rs:v", longopts, NULL)) != EOF)
    switch (c)
      {
      case 'd':
        if (action == NoAction)
          action = Delete;
        else if (action == Modify)
          action = ModNDel;
        else
          {
            usage (stderr);
            return 1;
	  }
        if (! getaclentries (Delete, optarg, acls, &aclidx))
          {
            fprintf (stderr, "%s: illegal acl entries\n", prog_name);
            return 2;
          }
        break;
      case 'f':
        if (action == NoAction)
          action = Set;
        else
          {
            usage (stderr);
            return 1;
	  }
        if (! getaclentries (SetFromFile, optarg, acls, &aclidx))
          {
            fprintf (stderr, "%s: illegal acl entries\n", prog_name);
            return 2;
          }
        break;
      case 'h':
        usage (stdout);
        return 0;
      case 'm':
        if (action == NoAction)
          action = Modify;
        else if (action == Delete)
          action = ModNDel;
        else
          {
            usage (stderr);
            return 1;
	  }
        if (! getaclentries (Modify, optarg, acls, &aclidx))
          {
            fprintf (stderr, "%s: illegal acl entries\n", prog_name);
            return 2;
          }
        break;
      case 'r':
        if (!ropt)
          ropt = 1;
        else
          {
            usage (stderr);
            return 1;
	  }
        break;
      case 's':
        if (action == NoAction)
          action = Set;
        else
          {
            usage (stderr);
            return 1;
	  }
        if (! getaclentries (Set, optarg, acls, &aclidx))
          {
            fprintf (stderr, "%s: illegal acl entries\n", prog_name);
            return 2;
          }
        break;
      case 'v':
        print_version ();
        return 0;
      default:
        usage (stderr);
        return 1;
      }
  if (action == NoAction)
    {
      usage (stderr);
      return 1;
    }
  if (optind > argc - 1)
    {
      usage (stderr);
      return 1;
    }
  if (action == Set)
    switch (aclcheck (acls, aclidx, NULL))
      {
      case GRP_ERROR:
        fprintf (stderr, "%s: more than one group entry.\n", prog_name);
        return 2;
      case USER_ERROR:
        fprintf (stderr, "%s: more than one user entry.\n", prog_name);
        return 2;
      case CLASS_ERROR:
        fprintf (stderr, "%s: more than one mask entry.\n", prog_name);
        return 2;
      case OTHER_ERROR:
        fprintf (stderr, "%s: more than one other entry.\n", prog_name);
        return 2;
      case DUPLICATE_ERROR:
        fprintf (stderr, "%s: duplicate additional user or group.\n", prog_name);
        return 2;
      case ENTRY_ERROR:
        fprintf (stderr, "%s: invalid entry type.\n", prog_name);
        return 2;
      case MISS_ERROR:
        fprintf (stderr, "%s: missing entries.\n", prog_name);
        return 2;
      case MEM_ERROR:
        fprintf (stderr, "%s: out of memory.\n", prog_name);
        return 2;
      default:
        break;
      }
  for (c = optind; c < argc; ++c)
    ret |= setfacl (action, argv[c], acls, aclidx);
  return ret;
}
