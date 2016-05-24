/* setfacl.c

   Written by Corinna Vinschen <vinschen@redhat.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pwd.h>
#include <grp.h>
#include <cygwin/acl.h>
#include <cygwin/version.h>

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

static char *prog_name;

typedef enum {
  NoAction = 0,
  DeleteExt = 1,	/* The values 1,2,3 allow bitmasking below. */
  DeleteDef = 2,
  DeleteAll = 3,
  Set,
  Modify,
  Delete,
  ModNDel,
  SetFromFile
} action_t;

int mask_opt = 0;

mode_t getperm (char *in)
{
  if (isdigit ((unsigned char) *in) && !in[1])
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
  ace->a_id = (uid_t) -1;
  ace->a_perm = 0;

  /* First, check if we're handling a default entry. */
  if (!strncmp (c, "default:", 8) || !strncmp (c, "d:", 2))
    {
      ace->a_type = ACL_DEFAULT;
      c = strchr (c, ':') + 1;
    }
  /* c now points to the type.  Check for next colon.  If we find a colon,
     NUL it.  Otherwise the string is invalid, except when deleting. */
  c2 = strchrnul (c, ':');
  if (*c2 == ':')
    *c2++ = '\0';
  else if (action != Delete)
    return FALSE;
  /* Fetch the type. */
  if (!strcmp (c, "u") || !strcmp (c, "user"))
      ace->a_type |= USER_OBJ;
  else if (!strcmp (c, "g") || !strcmp (c, "group"))
      ace->a_type |= GROUP_OBJ;
  else if (!strcmp (c, "m") || !strcmp (c, "mask"))
      ace->a_type |= CLASS_OBJ;
  else if (!strcmp (c, "o") || !strcmp (c, "other"))
      ace->a_type |= OTHER_OBJ;
  else
    return FALSE;
  /* Skip to next field. */
  c = c2;
  if (!*c)
    {
      /* Nothing follows.  This is only valid if action is Delete and the
	 type is CLASS_OBJ, or if ACL_DEFAULT is set. */
      if (action != Delete
	  || (!(ace->a_type & (CLASS_OBJ | ACL_DEFAULT))))
	return FALSE;
    }
  else if (!(ace->a_type & (USER_OBJ | GROUP_OBJ)))
    {
      /* Mask and other entries may contain an extra colon. */
      if (*c == ':')
	++c;
    }
  /* If this is a user or group entry, check if next char is a colon char.
     If so, skip it, otherwise it's the name of a user or group. */
  else if (*c == ':')
    ++c;
  else if (*c)
    {
      /* c now points to the id.  Check for next colon.  If we find a colon,
	 NUL it.  Otherwise the string is invalid, except when deleting.
	 If we delete, it must be a default entry since standard ugo entries
	 can't be deleted. */
      c2 = strchrnul (c + 1, ':');
      if (*c2 == ':')
	*c2++ = '\0';
      else if (action != Delete)
	return FALSE;
      /* Fetch user/group id. */
      if (isdigit ((unsigned char) *c))
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
      /* Skip to next field. */
      c = c2;
    }
  if (action == Delete)
    {
      /* Trailing garbage? */
      if (*c)
	return FALSE;
      /* No, we're good. */
      ace->a_perm = ILLEGAL_MODE;
      return TRUE;
    }
  /* Check perms. */
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
delace (aclent_t *tgt, int tcnt, int t)
{
  int i;

  for (i = t + 1; i < tcnt; ++i)
    tgt[i - 1] = tgt[i];
  --tcnt;
  return tcnt;
}

int
delacl (aclent_t *tgt, int tcnt, aclent_t *src, int scnt)
{
  int t, s;

  for (s = 0; s < scnt; ++s)
    {
      t = searchace (tgt, MAX_ACL_ENTRIES, src[s].a_type,
		     (src[s].a_type & (USER | GROUP)) ? src[s].a_id : -1);
      if (t < 0)
	return -1;
      if (t < tcnt)
	tcnt = delace (tgt, tcnt, t);
    }
  return tcnt;
}

int
modacl (aclent_t *tgt, int tcnt, aclent_t *src, int scnt)
{
  int t, s;

  /* Delete, replace or add given acl entries. */
  for (s = 0; s < scnt; ++s)
    {
      t = searchace (tgt, MAX_ACL_ENTRIES, src[s].a_type,
		     (src[s].a_type & (USER | GROUP)) ? src[s].a_id : -1);
      if (t < 0)
	return -1;
      /* ILLEGAL_MODE means "delete". */
      if (src[s].a_perm == ILLEGAL_MODE && t < tcnt)
	tcnt = delace (tgt, tcnt, t);
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
check_got_mask (aclent_t *src, int scnt, int *got_mask, int *got_def_mask)
{
  *got_mask = searchace (src, scnt, CLASS_OBJ, -1) >= 0;
  *got_def_mask = searchace (src, scnt, DEF_CLASS_OBJ, -1) >= 0;
}

int
recompute_mask (aclent_t *tgt, int tcnt, int got_mask, int got_def_mask)
{
  int t;
  int need_mask = 0, need_def_mask = 0;
  int mask_idx = -1, def_mask_idx = -1;
  mode_t mask = 0, def_mask = 0;

  /* Now recompute mask, if requested (default) */
  for (t = 0; t < tcnt; ++t)
    {
      switch (tgt[t].a_type)
	{
	case USER:
	case GROUP:
	  /* Do we need a CLASS_OBJ at all? */
	  need_mask = 1;
	  /*FALLTHRU*/
	case GROUP_OBJ:
	  /* Compute resulting maximum mask. */
	  mask |= tgt[t].a_perm;
	  break;
	case CLASS_OBJ:
	  /* Do we already have a CLASS_OBJ? */
	  mask_idx = t;
	  break;
	case DEF_USER:
	case DEF_GROUP:
	  /* Do we need a DEF_CLASS_OBJ at all? */
	  need_def_mask = 1;
	  /*FALLTHRU*/
	case DEF_GROUP_OBJ:
	  /* Compute resulting maximum default mask. */
	  def_mask |= tgt[t].a_perm;
	  break;
	case DEF_CLASS_OBJ:
	  /* Do we already have a DEF_CLASS_OBJ? */
	  def_mask_idx = t;
	  break;
	}
    }
  /* Recompute mask, if requested
     - If we got a mask in the input string, recompute only if --mask has been
       specified.
     - If we got no mask in the input, but we either need a mask or we already
       have one, and --no-mask has *not* been specified, recompute. */
  if ((got_mask && mask_opt > 0)
      || (!got_mask && mask_opt >= 0 && (need_mask || mask_idx >= 0)))
    {
      if (mask_idx >= 0)
	t = mask_idx;
      else
	t = searchace (tgt, MAX_ACL_ENTRIES, CLASS_OBJ, -1);
      if (t < 0)
	return -1;
      if (t >= tcnt)
	++tcnt;
      tgt[t].a_type = CLASS_OBJ;
      tgt[t].a_id = -1;
      tgt[t].a_perm = mask;
    }
  /* Recompute default mask, if requested */
  if ((got_def_mask && mask_opt > 0)
      || (!got_def_mask && mask_opt >= 0
	  && (need_def_mask || def_mask_idx >= 0)))
    {
      if (def_mask_idx >= 0)
	t = def_mask_idx;
      else
	t = searchace (tgt, MAX_ACL_ENTRIES, DEF_CLASS_OBJ, -1);
      if (t < 0)
	return -1;
      if (t >= tcnt)
	++tcnt;
      tgt[t].a_type = DEF_CLASS_OBJ;
      tgt[t].a_id = -1;
      tgt[t].a_perm = def_mask;
    }
  return tcnt;
}

int
addmissing (aclent_t *tgt, int tcnt)
{
  int t;
  int types = 0, def_types = 0;
  int perm = 0, def_perm = 0;

  /* Check if we have all the required entries now. */
  for (t = 0; t < tcnt; ++t)
    if (tgt[t].a_type & ACL_DEFAULT)
      {
	def_types |= tgt[t].a_type;
	if (tgt[t].a_type & GROUP_OBJ)
	  def_perm |= tgt[t].a_perm;
	else if ((tgt[t].a_type & (USER | GROUP)) && mask_opt >= 0)
	  def_perm |= tgt[t].a_perm;
      }
    else
      {
	types |= tgt[t].a_type;
	if (tgt[t].a_type & GROUP_OBJ)
	  perm |= tgt[t].a_perm;
	else if ((tgt[t].a_type & (USER | GROUP)) && mask_opt >= 0)
	  perm |= tgt[t].a_perm;
      }
  /* Add missing CLASS_OBJ */
  if ((types & (USER | GROUP)) && !(types & CLASS_OBJ))
    {
      tgt[tcnt].a_type = CLASS_OBJ;
      tgt[tcnt].a_id = (uid_t) -1;
      tgt[tcnt++].a_perm = perm;
    }
  if (def_types)
    {
      /* Add missing default entries. */
      if (!(def_types & USER_OBJ) && tcnt < MAX_ACL_ENTRIES)
	{
	  t = searchace (tgt, tcnt, USER_OBJ, -1);
	  tgt[tcnt].a_type = DEF_USER_OBJ;
	  tgt[tcnt].a_id = (uid_t) -1;
	  tgt[tcnt++].a_perm = t >= 0 ? tgt[t].a_perm : S_IRWXO;
	}
      if (!(def_types & GROUP_OBJ) && tcnt < MAX_ACL_ENTRIES)
	{
	  t = searchace (tgt, tcnt, GROUP_OBJ, -1);
	  tgt[tcnt].a_type = DEF_GROUP_OBJ;
	  tgt[tcnt].a_id = (uid_t) -1;
	  tgt[tcnt].a_perm = t >= 0 ? tgt[t].a_perm : (S_IROTH | S_IXOTH);
	  def_perm |= tgt[tcnt++].a_perm;
	}
      if (!(def_types & OTHER_OBJ) && tcnt < MAX_ACL_ENTRIES)
	{
	  t = searchace (tgt, tcnt, OTHER_OBJ, -1);
	  tgt[tcnt].a_type = DEF_OTHER_OBJ;
	  tgt[tcnt].a_id = (uid_t) -1;
	  tgt[tcnt++].a_perm = t >= 0 ? tgt[t].a_perm : (S_IROTH | S_IXOTH);
	}
      /* Add missing DEF_CLASS_OBJ */
      if ((def_types & (USER | GROUP)) && !(def_types & CLASS_OBJ))
	{
	  tgt[tcnt].a_type = DEF_CLASS_OBJ;
	  tgt[tcnt].a_id = (uid_t) -1;
	  tgt[tcnt++].a_perm = def_perm;
	}
    }
  return tcnt;
}

int
delallacl (aclent_t *tgt, int tcnt, action_t action)
{
  int t;

  for (t = 0; t < tcnt; ++t)
    /* -b (DeleteExt):    Remove all extended ACL entries.
       -k (DeleteDef):    Remove all default ACL entries.
       -b -k (DeleteAll): Remove extended and remove defaults.  That means,
			  only preserve standard POSIX perms. */
    if (((action & DeleteExt) && (tgt[t].a_type & (USER | GROUP | CLASS_OBJ)))
	|| ((action & DeleteDef) && (tgt[t].a_type & ACL_DEFAULT)))
      {
	--tcnt;
	if (t < tcnt)
	  memmove (&tgt[t], &tgt[t + 1], (tcnt - t) * sizeof (aclent_t));
	--t;
      }
  return tcnt;
}

int
setfacl (action_t action, const char *path, aclent_t *acls, int cnt)
{
  aclent_t lacl[MAX_ACL_ENTRIES];
  int lcnt, got_mask = 0, got_def_mask = 0;

  memset (lacl, 0, sizeof lacl);
  switch (action)
    {
    case Set:
      check_got_mask (acls, cnt, &got_mask, &got_def_mask);
      memcpy (lacl, acls, (lcnt = cnt) * sizeof (aclent_t));
      if ((lcnt = recompute_mask (lacl, lcnt, got_mask, got_def_mask)) < 0
	  || (lcnt = addmissing (lacl, lcnt)) < 0
	  || acl (path, SETACL, lcnt, lacl) < 0)
	{
	  perror (prog_name);
	  return 2;
	}
      break;
    case Delete:
      check_got_mask (acls, cnt, &got_mask, &got_def_mask);
      if ((lcnt = acl (path, GETACL, MAX_ACL_ENTRIES, lacl)) < 0
	  || (lcnt = delacl (lacl, lcnt, acls, cnt)) < 0
	  || (lcnt = recompute_mask (lacl, lcnt, got_mask, got_def_mask)) < 0
	  || acl (path, SETACL, lcnt, lacl) < 0)
	{
	  perror (prog_name);
	  return 2;
	}
      break;
    case DeleteExt:
    case DeleteDef:
    case DeleteAll:
      if ((lcnt = acl (path, GETACL, MAX_ACL_ENTRIES, lacl)) < 0
	  || (lcnt = delallacl (lacl, lcnt, action)) < 0
	  || acl (path, SETACL, lcnt, lacl) < 0)
	{
	  perror (prog_name);
	  return 2;
	}
      break;
    default:
      check_got_mask (acls, cnt, &got_mask, &got_def_mask);
      if ((lcnt = acl (path, GETACL, MAX_ACL_ENTRIES, lacl)) < 0
	  || (lcnt = modacl (lacl, lcnt, acls, cnt)) < 0
	  || (lcnt = recompute_mask (lacl, lcnt, got_mask, got_def_mask)) < 0
	  || (lcnt = addmissing (lacl, lcnt)) < 0
	  || acl (path, SETACL, lcnt, lacl) < 0)
	{
	  perror (prog_name);
	  return 2;
	}
      break;
    }
  return 0;
}

static void
usage (FILE *stream)
{
  fprintf (stream, ""
"Usage: %s [-n] {-f ACL_FILE | -s acl_entries} FILE...\n"
"       %s [-n] {[-bk]|[-x acl_entries] [-m acl_entries]} FILE...\n"
"\n"
"Modify file and directory access control lists (ACLs)\n"
"\n"
"  -b, --remove-all       remove all extended ACL entries\n"
"  -x, --delete           delete one or more specified ACL entries\n"
"  -f, --file             set ACL entries for FILE to ACL entries read\n"
"                         from ACL_FILE\n"
"  -k, --remove-default   remove all default ACL entries\n"
"  -m, --modify           modify one or more specified ACL entries\n"
"  -n, --no-mask          don't recalculate the effective rights mask\n"
"      --mask             do recalculate the effective rights mask\n"
"  -s, --set              set specified ACL entries on FILE\n"
"  -V, --version          print version and exit\n"
"  -h, --help             this help text\n"
"\n"
"At least one of (-b, -x, -f, -k, -m, -s) must be specified\n"
"\n", prog_name, prog_name);
    if (stream == stdout)
    {
      printf(""
"  Acl_entries are one or more comma-separated ACL entries from the following\n"
"  list:\n"
"\n"
"    u[ser]::perm\n"
"    u[ser]:uid:perm\n"
"    g[roup]::perm\n"
"    g[roup]:gid:perm\n"
"    m[ask]:perm\n"
"    o[ther]:perm\n"
"\n"
"  Default entries are like the above with the additional default identifier.\n"
"  For example: \n"
"\n"
"    d[efault]:u[ser]:uid:perm\n"
"\n"
"  'perm' is either a 3-char permissions string in the form \"rwx\" with the\n"
"  character - for no permission, or it is the octal representation of the\n"
"  permissions, a value from 0 (equivalent to \"---\") to 7 (\"rwx\").\n"
"  'uid' is a user name or a numerical uid.\n"
"  'gid' is a group name or a numerical gid.\n"
"\n"
"For each file given as parameter, %s will either replace its complete ACL\n"
"(-s, -f), or it will add, modify, or delete ACL entries.\n"
"\n"
"The following options are supported:\n"
"\n"
"-b, --remove-all\n"
"  Remove all extended ACL entries.  The base ACL entries of the owner, group\n"
"  and others are retained.  This option can be combined with the\n"
"  -k,--remove-default option to delete all non-standard POSIX permissions.\n"
"\n"
"-x, --delete\n"
"  Delete one or more specified entries from the file's ACL.  The owner, group\n"
"  and others entries must not be deleted.  Acl_entries to be deleted should\n"
"  be specified without permissions, as in the following list:\n"
"\n"
"    u[ser]:uid[:]\n"
"    g[roup]:gid[:]\n"
"    m[ask][:]\n"
"    d[efault]:u[ser][:uid]\n"
"    d[efault]:g[roup][:gid]\n"
"    d[efault]:m[ask][:]\n"
"    d[efault]:o[ther][:]\n"
"\n"
"-f, --file\n"
"  Take the Acl_entries from ACL_FILE one per line.  Whitespace characters are\n"
"  ignored, and the character \"#\" may be used to start a comment.  The special\n"
"  filename \"-\" indicates reading from stdin.\n"
"  Required entries are\n"
"  - One user entry for the owner of the file.\n"
"  - One group entry for the group of the file.\n"
"  - One other entry.\n"
"  If additional user and group entries are given:\n"
"  - A mask entry for the file group class of the file.\n"
"  - No duplicate user or group entries with the same uid/gid.\n"
"  If it is a directory:\n"
"  - One default user entry for the owner of the file.\n"
"  - One default group entry for the group of the file.\n"
"  - One default mask entry for the file group class.\n"
"  - One default other entry.\n"
"\n"
"-k, --remove-default\n"
"  Remove all default ACL entries. If no default ACL entries exist, no\n"
"  warnings are issued.  This option can be combined with the -b,--remove-all\n"
"  option to delete all non-standard POSIX permissions.\n"
"\n"
"-m, --modify\n"
"  Add or modify one or more specified ACL entries.  Acl_entries is a\n"
"  comma-separated list of entries from the same list as above.\n"
"\n"
"-n, --no-mask\n"
"  Valid in conjunction with -m.  Do not recalculate the effective rights\n"
"  mask. The default behavior of setfacl is to recalculate the ACL mask entry,\n"
"  unless a mask entry was explicitly given.  The mask entry is set to the\n"
"  union of all permissions of the owning group, and all named user and group\n"
"  entries.  (These are exactly the entries affected by the mask entry).\n"
"\n"
"--mask\n"
"  Valid in conjunction with -m.  Do recalculate the effective rights mask,\n"
"  even if an ACL mask entry was explicitly given. (See the -n option.)\n"
"\n"
"-s, --set\n"
"  Like -f, but set the file's ACL with ACL entries specified in a\n"
"  comma-separated list on the command line.\n"
"\n"
"While the -x and -m options may be used in the same command, the -f and -s\n"
"options may be used only exclusively.\n"
"\n"
"Directories may contain default ACL entries.  Files created in a directory\n"
"that contains default ACL entries will have permissions according to the\n"
"combination of the current umask, the explicit permissions requested and\n"
"the default ACL entries.\n"
"\n", prog_name);
  }
  else
    fprintf(stream, "Try '%s --help' for more information.\n", prog_name);
}

struct option longopts[] = {
  {"remove-all", no_argument, NULL, 'b'},
  {"delete", required_argument, NULL, 'x'},
  {"file", required_argument, NULL, 'f'},
  {"remove-default", no_argument, NULL, 'k'},
  {"modify", required_argument, NULL, 'm'},
  {"no-mask", no_argument, NULL, 'n'},
  {"mask", no_argument, NULL, '\n'},
  {"replace", no_argument, NULL, 'r'},
  {"set", required_argument, NULL, 's'},
  {"substitute", required_argument, NULL, 's'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {0, no_argument, NULL, 0}
};
const char *opts = "bd:f:hkm:nrs:Vx:";

static void
print_version ()
{
  printf ("setfacl (cygwin) %d.%d.%d\n"
	  "POSIX ACL modification utility\n"
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
  action_t action = NoAction;
  aclent_t acls[MAX_ACL_ENTRIES];
  int aclidx = 0;
  int ret = 0;

  prog_name = program_invocation_short_name;

  memset (acls, 0, sizeof acls);
  while ((c = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (c)
      {
      case 'b':
	if (action == NoAction)
	  action = DeleteExt;
	else if (action == DeleteDef)
	  action = DeleteAll;
	else
	  {
	    usage (stderr);
	    return 1;
	  }
	break;
      case 'd':		/* Backward compat */
      case 'x':
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
      case 'k':
	if (action == NoAction)
	  action = DeleteDef;
	else if (action == DeleteExt)
	  action = DeleteAll;
	else
	  {
	    usage (stderr);
	    return 1;
	  }
	break;
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
      case 'n':
	mask_opt = -1;
	break;
      case '\n':
	mask_opt = 1;
	break;
      case 'r':
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
      case 'V':
	print_version ();
	return 0;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n", prog_name);
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
