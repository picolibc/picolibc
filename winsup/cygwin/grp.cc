/* grp.cc

   Copyright 1996, 1997, 1998, 2000 Cygnus Solutions.

   Original stubs by Jason Molenda of Cygnus Support, crash@cygnus.com
   First implementation by Gunther Ebert, gunther.ebert@ixos-leipzig.de

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include "winsup.h"

/* Read /etc/group only once for better performance.  This is done
   on the first call that needs information from it. */

#define MAX_DOMAIN_NAME	100

static NO_COPY const char *etc_group = "/etc/group";
static struct group *group_buf = NULL;		/* group contents in memory */
static int curr_lines = 0;
static int max_lines = 0;

/* Position in the group cache */
#ifdef _MT_SAFE
#define grp_pos _reent_winsup()->_grp_pos
#else
static int grp_pos = 0;
#endif

/* Set to 1 when /etc/group has been read in by read_etc_group (). */
/* Functions in this file need to check the value of group_in_memory_p
   and read in the group file if it isn't set. */
/* FIXME: This should be static but this is called in uinfo_init outside
   this file */
int group_in_memory_p = 0;

static int
parse_grp (struct group &grp, const char *line)
{
  int len = strlen(line);
  char *newline = (char *) malloc (len + 1);
  (void) memcpy (newline, line, len + 1);

  if (newline[--len] == '\n')
    newline[len] = '\0';

  char *dp = strchr (newline, ':');

  if (!dp)
    return 0;

  *dp++ = '\0';
  grp.gr_name = newline;

  grp.gr_passwd = dp;
  dp = strchr (grp.gr_passwd, ':');
  if (dp)
    {
      *dp++ = '\0';
      if (!strlen (grp.gr_passwd))
        grp.gr_passwd = NULL;

      grp.gr_gid = atoi (dp);
      dp = strchr (dp, ':');
      if (dp)
        {
          if (*++dp)
            {
              int i = 0;
              char *cp;

              for (cp = dp; (cp = strchr (cp, ',')) != NULL; ++cp)
                ++i;
              char **namearray = (char **) calloc (i + 2, sizeof (char *));
              if (namearray)
                {
                  i = 0;
                  for (cp = dp; (cp = strchr (dp, ',')) != NULL; dp = cp + 1)
                    {
                      *cp = '\0';
                      namearray[i++] = dp;
                    }
                  namearray[i++] = dp;
                  namearray[i] = NULL;
                }
	      grp.gr_mem = namearray;
            }
          else
            grp.gr_mem = (char **) calloc (1, sizeof (char *));
          return 1;
        }
    }
  return 0;
}

/* Read one line from /etc/group into the group cache */
static void
add_grp_line (const char *line)
{
    if (curr_lines == max_lines)
    {
	max_lines += 10;
	group_buf = (struct group *) realloc (group_buf, max_lines * sizeof (struct group));
    }
    if (parse_grp (group_buf[curr_lines], line))
      curr_lines++;
}

extern PSID get_admin_sid ();

/* Cygwin internal */
/* Read in /etc/group and save contents in the group cache */
/* This sets group_in_memory_p to 1 so functions in this file can
   tell that /etc/group has been read in */
/* FIXME: should be static but this is called in uinfo_init outside this
   file */
void
read_etc_group ()
{
  extern int group_sem;
  char linebuf [ 200 ];
  char group_name [ MAX_USER_NAME ];
  DWORD group_name_len = MAX_USER_NAME;

  strncpy (group_name, "Administrators", sizeof (group_name));

  ++group_sem;
  FILE *f = fopen (etc_group, "r");
  --group_sem;

  if (f)
    {
      while (fgets (linebuf, sizeof (linebuf), f) != NULL)
	{
	  if (strlen (linebuf))
	    add_grp_line (linebuf);
	}

      fclose (f);
    }
  else /* /etc/group doesn't exist -- create default one in memory */
    {
      char domain_name [ MAX_DOMAIN_NAME ];
      DWORD domain_name_len = MAX_DOMAIN_NAME;
      SID_NAME_USE acType;
      debug_printf ("Emulating /etc/group");
      if (! LookupAccountSidA (NULL ,
				get_admin_sid () ,
				group_name,
				&group_name_len,
				domain_name,
				&domain_name_len,
				&acType))
	{
	  strcpy (group_name, "unknown");
	  debug_printf ("Failed to get local admins group name. %E");
        }

      snprintf (linebuf, sizeof (linebuf), "%s::%u:\n", group_name, DEFAULT_GID);
      add_grp_line (linebuf);
    }

  group_in_memory_p = 1;
}

extern "C"
struct group *
getgrgid (gid_t gid)
{
  struct group * default_grp = NULL;
  if (!group_in_memory_p)
    read_etc_group();

  for (int i = 0; i < curr_lines; i++)
    {
      if (group_buf[i].gr_gid == DEFAULT_GID)
        default_grp = group_buf + i;
      if (group_buf[i].gr_gid == gid)
        return group_buf + i;
    }

  return default_grp;
}

extern "C"
struct group *
getgrnam (const char *name)
{
  if (!group_in_memory_p)
    read_etc_group();

  for (int i = 0; i < curr_lines; i++)
    if (strcasematch (group_buf[i].gr_name, name))
      return group_buf + i;

  /* Didn't find requested group */
  return NULL;
}

extern "C"
void
endgrent()
{
  grp_pos = 0;
}

extern "C"
struct group *
getgrent()
{
  if (!group_in_memory_p)
    read_etc_group();

  if (grp_pos < curr_lines)
    return group_buf + grp_pos++;

  return NULL;
}

extern "C"
void
setgrent ()
{
  grp_pos = 0;
}

int
getgroups (int gidsetsize, gid_t *grouplist, gid_t gid, const char *username)
{
  if (!group_in_memory_p)
    read_etc_group();

  int cnt = 0;

  for (int i = 0; i < curr_lines; ++i)
    if (gid == group_buf[i].gr_gid)
      {
        if (cnt < gidsetsize)
          grouplist[cnt] = group_buf[i].gr_gid;
        ++cnt;
        if (gidsetsize && cnt >= gidsetsize)
          goto out;
      }
    else if (group_buf[i].gr_mem)
      for (int gi = 0; group_buf[i].gr_mem[gi]; ++gi)
        if (! strcasecmp (username, group_buf[i].gr_mem[gi]))
          {
            if (cnt < gidsetsize)
              grouplist[cnt] = group_buf[i].gr_gid;
            ++cnt;
            if (gidsetsize && cnt >= gidsetsize)
              goto out;
          }
out:
  return cnt;
}

extern "C"
int
getgroups (int gidsetsize, gid_t *grouplist)
{
#if 0
  if (gidsetsize <= 0)
      return 0;
  grouplist[0] = myself->gid;
  return 1;
#else
  return getgroups (gidsetsize, grouplist, myself->gid, myself->username);
#endif
}

extern "C"
int
initgroups (const char *, gid_t)
{
  return 0;
}
