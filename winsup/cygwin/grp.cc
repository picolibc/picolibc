/* grp.cc

   Copyright 1996, 1997, 1998, 2000 Cygnus Solutions.

   Original stubs by Jason Molenda of Cygnus Support, crash@cygnus.com
   First implementation by Gunther Ebert, gunther.ebert@ixos-leipzig.de

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <grp.h>
#include <wininet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygerrno.h"
#include "security.h"

/* Read /etc/group only once for better performance.  This is done
   on the first call that needs information from it. */

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

/* Set to loaded when /etc/passwd has been read in by read_etc_passwd ().
   Set to emulated if passwd is emulated. */
/* Functions in this file need to check the value of passwd_state
   and read in the password file if it isn't set. */
enum grp_state {
  uninitialized = 0,
  initializing,
  emulated,
  loaded
};
static grp_state group_state = uninitialized;

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

      grp.gr_gid = strtol (dp, NULL, 10);
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

/* Cygwin internal */
/* Read in /etc/group and save contents in the group cache */
/* This sets group_in_memory_p to 1 so functions in this file can
   tell that /etc/group has been read in */
/* FIXME: should be static but this is called in uinfo_init outside this
   file */
void
read_etc_group ()
{
  char linebuf [200];
  char group_name [UNLEN + 1];
  DWORD group_name_len = UNLEN + 1;

  strncpy (group_name, "Administrators", sizeof (group_name));

  static pthread_mutex_t etc_group_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock (&etc_group_mutex);

  /* if we got blocked by the mutex, then etc_group may have been processed */
  if (group_state != uninitialized)
    {
      pthread_mutex_unlock(&etc_group_mutex);
      return;
    }

  if (group_state != initializing)
    {
      group_state = initializing;

      FILE *f = fopen (etc_group, "rt");

      if (f)
	{
	  while (fgets (linebuf, sizeof (linebuf), f) != NULL)
	    {
	      if (strlen (linebuf))
		add_grp_line (linebuf);
	    }

	  fclose (f);
	  group_state = loaded;
	}
      else /* /etc/group doesn't exist -- create default one in memory */
	{
	  char domain_name [INTERNET_MAX_HOST_NAME_LENGTH + 1];
	  DWORD domain_name_len = INTERNET_MAX_HOST_NAME_LENGTH + 1;
	  SID_NAME_USE acType;
	  debug_printf ("Emulating /etc/group");
	  if (! LookupAccountSidA (NULL ,
				    well_known_admin_sid,
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
	  group_state = emulated;
	}
    }

  pthread_mutex_unlock(&etc_group_mutex);
}

extern "C"
struct group *
getgrgid (gid_t gid)
{
  struct group * default_grp = NULL;
  if (group_state  <= initializing)
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
  if (group_state  <= initializing)
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
  if (group_state  <= initializing)
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

/* Internal function. ONLY USE THIS INTERNALLY, NEVER `getgrent'!!! */
struct group *
internal_getgrent (int pos)
{
  if (group_state  <= initializing)
    read_etc_group();

  if (pos < curr_lines)
    return group_buf + pos;
  return NULL;
}

int
getgroups (int gidsetsize, gid_t *grouplist, gid_t gid, const char *username)
{
  HANDLE hToken = NULL;
  DWORD size;
  int cnt = 0;
  struct group *gr;

  if (group_state  <= initializing)
    read_etc_group();

  if (allow_ntsec &&
      OpenProcessToken (hMainProc, TOKEN_QUERY, &hToken))
    {
      if (GetTokenInformation (hToken, TokenGroups, NULL, 0, &size)
          || GetLastError () == ERROR_INSUFFICIENT_BUFFER)
        {
	  char buf[size];
	  TOKEN_GROUPS *groups = (TOKEN_GROUPS *) buf;

	  if (GetTokenInformation (hToken, TokenGroups, buf, size, &size))
	    {
	      cygsid sid;

	      for (int gidx = 0; (gr = internal_getgrent (gidx)); ++gidx)
		if (sid.getfromgr (gr))
		  for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
		    if (sid == groups->Groups[pg].Sid)
		      {
			if (cnt < gidsetsize)
			  grouplist[cnt] = gr->gr_gid;
			++cnt;
			if (gidsetsize && cnt > gidsetsize)
			  {
			    CloseHandle (hToken);
			    goto error;
			  }
			break;
		      }
	    }
        }
      else
        debug_printf ("%d = GetTokenInformation(NULL) %E", size);
      CloseHandle (hToken);
      if (cnt)
        return cnt;
    }

  for (int gidx = 0; (gr = internal_getgrent (gidx)); ++gidx)
    if (gid == gr->gr_gid)
      {
	if (cnt < gidsetsize)
	  grouplist[cnt] = gr->gr_gid;
	++cnt;
	if (gidsetsize && cnt > gidsetsize)
	  goto error;
      }
    else if (gr->gr_mem)
      for (int gi = 0; gr->gr_mem[gi]; ++gi)
	if (strcasematch (username, gr->gr_mem[gi]))
	  {
	    if (cnt < gidsetsize)
	      grouplist[cnt] = gr->gr_gid;
	    ++cnt;
	    if (gidsetsize && cnt > gidsetsize)
	      goto error;
	  }
  return cnt;

error:
  set_errno (EINVAL);
  return -1;
}

extern "C"
int
getgroups (int gidsetsize, gid_t *grouplist)
{
  return getgroups (gidsetsize, grouplist, myself->gid, cygheap->user.name ());
}

extern "C"
int
initgroups (const char *, gid_t)
{
  return 0;
}
