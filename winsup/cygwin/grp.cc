/* grp.cc

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
   2008, 2009, 2011, 2012, 2013 Red Hat, Inc.

   Original stubs by Jason Molenda of Cygnus Support, crash@cygnus.com
   First implementation by Gunther Ebert, gunther.ebert@ixos-leipzig.de

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cygerrno.h"
#include "pinfo.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include "pwdgrp.h"

static group *group_buf;
static pwdgrp gr (group_buf);
static char * NO_COPY_RO null_ptr;

bool
pwdgrp::parse_group ()
{
  group &grp = (*group_buf)[curr_lines];
  grp.gr_name = next_str (':');
  if (!*grp.gr_name)
    return false;

  grp.gr_passwd = next_str (':');

  if (!next_num (grp.gr_gid))
    return false;

  int n;
  char *dp = raw_ptr ();
  for (n = 0; *next_str (','); n++)
    continue;

  grp.gr_mem = &null_ptr;
  if (n)
    {
      char **namearray = (char **) calloc (n + 1, sizeof (char *));
      if (namearray)
	{
	  for (int i = 0; i < n; i++, dp = strchr (dp, '\0') + 1)
	    namearray[i] = dp;
	  grp.gr_mem = namearray;
	}
    }

  return true;
}

/* Cygwin internal */
/* Read in /etc/group and save contents in the group cache */
/* This sets group_in_memory_p to 1 so functions in this file can
   tell that /etc/group has been read in */
void
pwdgrp::read_group ()
{
  for (int i = 0; i < gr.curr_lines; i++)
    if ((*group_buf)[i].gr_mem != &null_ptr)
      free ((*group_buf)[i].gr_mem);

  load (L"\\etc\\group");

  /* Complete /etc/group in memory if needed */
  if (!internal_getgrgid (myself->gid))
    {
      static char linebuf [200];
      char group_name [UNLEN + 1] = "mkgroup";
      char strbuf[128] = "";
      struct group *gr;

      cygheap->user.groups.pgsid.string (strbuf);
      if ((gr = internal_getgrsid (cygheap->user.groups.pgsid)))
	snprintf (group_name, sizeof (group_name),
		  "passwd/group_GID_clash(%u/%u)", myself->gid, gr->gr_gid);
      if (myself->uid == UNKNOWN_UID)
	strcpy (group_name, "mkpasswd"); /* Feedback... */
      snprintf (linebuf, sizeof (linebuf), "%s:%s:%u:%s",
		group_name, strbuf, myself->gid, cygheap->user.name ());
      debug_printf ("Completing /etc/group: %s", linebuf);
      add_line (linebuf);
    }
  static char NO_COPY pretty_ls[] = "????????::-1:";
  add_line (pretty_ls);
}

muto NO_COPY pwdgrp::pglock;

pwdgrp::pwdgrp (passwd *&pbuf) :
  pwdgrp_buf_elem_size (sizeof (*pbuf)), passwd_buf (&pbuf)
{
  read = &pwdgrp::read_passwd;
  parse = &pwdgrp::parse_passwd;
  pglock.init ("pglock");
}

pwdgrp::pwdgrp (group *&gbuf) :
  pwdgrp_buf_elem_size (sizeof (*gbuf)), group_buf (&gbuf)
{
  read = &pwdgrp::read_group;
  parse = &pwdgrp::parse_group;
  pglock.init ("pglock");
}

struct group *
internal_getgrsid (cygpsid &sid)
{
  char sid_string[128];

  gr.refresh (false);

  if (sid.string (sid_string))
    for (int i = 0; i < gr.curr_lines; i++)
      if (!strcmp (sid_string, group_buf[i].gr_passwd))
	return group_buf + i;
  return NULL;
}

struct group *
internal_getgrgid (gid_t gid, bool check)
{
  gr.refresh (check);

  for (int i = 0; i < gr.curr_lines; i++)
    if (group_buf[i].gr_gid == gid)
      return group_buf + i;
  return NULL;
}

struct group *
internal_getgrnam (const char *name, bool check)
{
  gr.refresh (check);

  for (int i = 0; i < gr.curr_lines; i++)
    if (strcasematch (group_buf[i].gr_name, name))
      return group_buf + i;

  /* Didn't find requested group */
  return NULL;
}

#ifndef __x86_64__
static struct __group16 *
grp32togrp16 (struct __group16 *gp16, struct group *gp32)
{
  if (!gp16 || !gp32)
    return NULL;

  /* Copying the pointers is actually unnecessary.  Just having the correct
     return type is important. */
  gp16->gr_name = gp32->gr_name;
  gp16->gr_passwd = gp32->gr_passwd;
  gp16->gr_gid = (__gid16_t) gp32->gr_gid;		/* Not loss-free */
  gp16->gr_mem = gp32->gr_mem;

  return gp16;
}
#endif

extern "C" int
getgrgid_r (gid_t gid, struct group *grp, char *buffer, size_t bufsize,
	    struct group **result)
{
  *result = NULL;

  if (!grp || !buffer)
    return ERANGE;

  struct group *tempgr = internal_getgrgid (gid, true);
  pthread_testcancel ();
  if (!tempgr)
    return 0;

  /* check needed buffer size. */
  int i;
  size_t needsize = strlen (tempgr->gr_name) + strlen (tempgr->gr_passwd)
		    + 2 + sizeof (char *);
  for (i = 0; tempgr->gr_mem[i]; ++i)
    needsize += strlen (tempgr->gr_mem[i]) + 1 + sizeof (char *);
  if (needsize > bufsize)
    return ERANGE;

  /* make a copy of tempgr */
  *result = grp;
  grp->gr_gid = tempgr->gr_gid;
  buffer = stpcpy (grp->gr_name = buffer, tempgr->gr_name);
  buffer = stpcpy (grp->gr_passwd = buffer + 1, tempgr->gr_passwd);
  grp->gr_mem = (char **) (buffer + 1);
  buffer = (char *) grp->gr_mem + (i + 1) * sizeof (char *);
  for (i = 0; tempgr->gr_mem[i]; ++i)
    buffer = stpcpy (grp->gr_mem[i] = buffer, tempgr->gr_mem[i]) + 1;
  grp->gr_mem[i] = NULL;
  return 0;
}

extern "C" struct group *
getgrgid32 (gid_t gid)
{
  return internal_getgrgid (gid, true);
}

#ifdef __x86_64__
EXPORT_ALIAS (getgrgid32, getgrgid)
#else
extern "C" struct __group16 *
getgrgid (__gid16_t gid)
{
  static struct __group16 g16;	/* FIXME: thread-safe? */

  return grp32togrp16 (&g16, getgrgid32 (gid16togid32 (gid)));
}
#endif

extern "C" int
getgrnam_r (const char *nam, struct group *grp, char *buffer,
	    size_t bufsize, struct group **result)
{
  *result = NULL;

  if (!grp || !buffer)
    return ERANGE;

  struct group *tempgr = internal_getgrnam (nam, true);
  pthread_testcancel ();
  if (!tempgr)
    return 0;

  /* check needed buffer size. */
  int i;
  size_t needsize = strlen (tempgr->gr_name) + strlen (tempgr->gr_passwd)
		    + 2 + sizeof (char *);
  for (i = 0; tempgr->gr_mem[i]; ++i)
    needsize += strlen (tempgr->gr_mem[i]) + 1 + sizeof (char *);
  if (needsize > bufsize)
    return ERANGE;

  /* make a copy of tempgr */
  *result = grp;
  grp->gr_gid = tempgr->gr_gid;
  buffer = stpcpy (grp->gr_name = buffer, tempgr->gr_name);
  buffer = stpcpy (grp->gr_passwd = buffer + 1, tempgr->gr_passwd);
  grp->gr_mem = (char **) (buffer + 1);
  buffer = (char *) grp->gr_mem + (i + 1) * sizeof (char *);
  for (i = 0; tempgr->gr_mem[i]; ++i)
    buffer = stpcpy (grp->gr_mem[i] = buffer, tempgr->gr_mem[i]) + 1;
  grp->gr_mem[i] = NULL;
  return 0;
}

extern "C" struct group *
getgrnam32 (const char *name)
{
  return internal_getgrnam (name, true);
}

#ifdef __x86_64__
EXPORT_ALIAS (getgrnam32, getgrnam)
#else
extern "C" struct __group16 *
getgrnam (const char *name)
{
  static struct __group16 g16;	/* FIXME: thread-safe? */

  return grp32togrp16 (&g16, getgrnam32 (name));
}
#endif

extern "C" void
endgrent ()
{
  _my_tls.locals.grp_pos = 0;
}

extern "C" struct group *
getgrent32 ()
{
  if (_my_tls.locals.grp_pos == 0)
    gr.refresh (true);
  if (_my_tls.locals.grp_pos < gr.curr_lines)
    return group_buf + _my_tls.locals.grp_pos++;

  return NULL;
}

#ifdef __x86_64__
EXPORT_ALIAS (getgrent32, getgrent)
#else
extern "C" struct __group16 *
getgrent ()
{
  static struct __group16 g16;	/* FIXME: thread-safe? */

  return grp32togrp16 (&g16, getgrent32 ());
}
#endif

extern "C" void
setgrent ()
{
  _my_tls.locals.grp_pos = 0;
}

/* Internal function. ONLY USE THIS INTERNALLY, NEVER `getgrent'!!! */
struct group *
internal_getgrent (int pos)
{
  gr.refresh (false);

  if (pos < gr.curr_lines)
    return group_buf + pos;
  return NULL;
}

int
internal_getgroups (int gidsetsize, gid_t *grouplist, cygpsid * srchsid)
{
  NTSTATUS status;
  HANDLE hToken = NULL;
  ULONG size;
  int cnt = 0;
  struct group *gr;

  if (!srchsid && cygheap->user.groups.issetgroups ())
    {
      cygsid sid;
      for (int gidx = 0; (gr = internal_getgrent (gidx)); ++gidx)
	if (sid.getfromgr (gr))
	  for (int pg = 0; pg < cygheap->user.groups.sgsids.count (); ++pg)
	    if (sid == cygheap->user.groups.sgsids.sids[pg]
		&& sid != well_known_world_sid)
	      {
		if (cnt < gidsetsize)
		  grouplist[cnt] = gr->gr_gid;
		++cnt;
		if (gidsetsize && cnt > gidsetsize)
		  goto error;
		break;
	      }
      return cnt;
    }


  /* If impersonated, use impersonation token. */
  if (cygheap->user.issetuid ())
    hToken = cygheap->user.primary_token ();
  else
    hToken = hProcToken;

  status = NtQueryInformationToken (hToken, TokenGroups, NULL, 0, &size);
  if (NT_SUCCESS (status) || status == STATUS_BUFFER_TOO_SMALL)
    {
      PTOKEN_GROUPS groups = (PTOKEN_GROUPS) alloca (size);

      status = NtQueryInformationToken (hToken, TokenGroups, groups,
					size, &size);
      if (NT_SUCCESS (status))
	{
	  cygsid sid;

	  if (srchsid)
	    {
	      for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
		if ((cnt = (*srchsid == groups->Groups[pg].Sid)))
		  break;
	    }
	  else
	    for (int gidx = 0; (gr = internal_getgrent (gidx)); ++gidx)
	      if (sid.getfromgr (gr))
		for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
		  if (sid == groups->Groups[pg].Sid
		      && (groups->Groups[pg].Attributes
			  & (SE_GROUP_ENABLED | SE_GROUP_INTEGRITY_ENABLED))
		      && sid != well_known_world_sid)
		    {
		      if (cnt < gidsetsize)
			grouplist[cnt] = gr->gr_gid;
		      ++cnt;
		      if (gidsetsize && cnt > gidsetsize)
			goto error;
		      break;
		    }
	}
    }
  else
    debug_printf ("%u = NtQueryInformationToken(NULL) %y", size, status);
  return cnt;

error:
  set_errno (EINVAL);
  return -1;
}

extern "C" int
getgroups32 (int gidsetsize, gid_t *grouplist)
{
  return internal_getgroups (gidsetsize, grouplist);
}

#ifdef __x86_64__
EXPORT_ALIAS (getgroups32, getgroups)
#else
extern "C" int
getgroups (int gidsetsize, __gid16_t *grouplist)
{
  gid_t *grouplist32 = NULL;

  if (gidsetsize < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (gidsetsize > 0 && grouplist)
    grouplist32 = (gid_t *) alloca (gidsetsize * sizeof (gid_t));

  int ret = internal_getgroups (gidsetsize, grouplist32);

  if (gidsetsize > 0 && grouplist)
    for (int i = 0; i < ret; ++ i)
      grouplist[i] = grouplist32[i];

  return ret;
}
#endif

/* Core functionality of initgroups and getgrouplist. */
static void
get_groups (const char *user, gid_t gid, cygsidlist &gsids)
{
  cygheap->user.deimpersonate ();
  struct passwd *pw = internal_getpwnam (user);
  struct group *gr = internal_getgrgid (gid);
  cygsid usersid, grpsid;
  if (usersid.getfrompw (pw))
    get_server_groups (gsids, usersid, pw);
  if (gid != ILLEGAL_GID && grpsid.getfromgr (gr))
    gsids += grpsid;
  cygheap->user.reimpersonate ();
}

extern "C" int
initgroups32 (const char *user, gid_t gid)
{
  assert (user != NULL);
  cygsidlist tmp_gsids (cygsidlist_auto, 12);
  get_groups (user, gid, tmp_gsids);
  cygsidlist new_gsids (cygsidlist_alloc, tmp_gsids.count ());
  for (int i = 0; i < tmp_gsids.count (); i++)
    new_gsids.sids[i] = tmp_gsids.sids[i];
  new_gsids.count (tmp_gsids.count ());
  cygheap->user.groups.update_supp (new_gsids);
  syscall_printf ( "0 = initgroups(%s, %u)", user, gid);
  return 0;
}

#ifdef __x86_64__
EXPORT_ALIAS (initgroups32, initgroups)
#else
extern "C" int
initgroups (const char *user, __gid16_t gid)
{
  return initgroups32 (user, gid16togid32(gid));
}
#endif

extern "C" int
getgrouplist (const char *user, gid_t gid, gid_t *groups, int *ngroups)
{
  int ret = 0;
  int cnt = 0;
  struct group *gr;

  /* Note that it's not defined if groups or ngroups may be NULL!
     GLibc does not check the pointers on entry and just uses them.
     FreeBSD calls assert for ngroups and allows a NULL groups if
     *ngroups is 0.  We follow FreeBSD's lead here, but always allow
     a NULL groups pointer. */
  assert (user != NULL);
  assert (ngroups != NULL);

  cygsidlist tmp_gsids (cygsidlist_auto, 12);
  get_groups (user, gid, tmp_gsids);
  for (int i = 0; i < tmp_gsids.count (); i++)
    if ((gr = internal_getgrsid (tmp_gsids.sids[i])) != NULL)
      {
	if (groups && cnt < *ngroups)
	  groups[cnt] = gr->gr_gid;
	++cnt;
      }
  if (cnt > *ngroups)
    ret = -1;
  else
    ret = cnt;
  *ngroups = cnt;

  syscall_printf ( "%d = getgrouplist(%s, %u, %p, %d)",
		  ret, user, gid, groups, *ngroups);
  return ret;
}

/* setgroups32: standards? */
extern "C" int
setgroups32 (int ngroups, const gid_t *grouplist)
{
  syscall_printf ("setgroups32 (%d)", ngroups);
  if (ngroups < 0 || (ngroups > 0 && !grouplist))
    {
      set_errno (EINVAL);
      return -1;
    }

  cygsidlist gsids (cygsidlist_alloc, ngroups);
  struct group *gr;

  if (ngroups && !gsids.sids)
    return -1;

  for (int gidx = 0; gidx < ngroups; ++gidx)
    {
      if ((gr = internal_getgrgid (grouplist[gidx]))
	  && gsids.addfromgr (gr))
	continue;
      debug_printf ("No sid found for gid %u", grouplist[gidx]);
      gsids.free_sids ();
      set_errno (EINVAL);
      return -1;
    }
  cygheap->user.groups.update_supp (gsids);
  return 0;
}

#ifdef __x86_64__
EXPORT_ALIAS (setgroups32, setgroups)
#else
extern "C" int
setgroups (int ngroups, const __gid16_t *grouplist)
{
  gid_t *grouplist32 = NULL;

  if (ngroups > 0 && grouplist)
    {
      grouplist32 = (gid_t *) alloca (ngroups * sizeof (gid_t));
      if (grouplist32 == NULL)
	return -1;
      for (int i = 0; i < ngroups; i++)
	grouplist32[i] = grouplist[i];
    }
  return setgroups32 (ngroups, grouplist32);
}
#endif
