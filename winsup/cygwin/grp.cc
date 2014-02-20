/* grp.cc

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
   2008, 2009, 2011, 2012, 2013, 2014 Red Hat, Inc.

   Original stubs by Jason Molenda of Cygnus Support, crash@cygnus.com
   First implementation by Gunther Ebert, gunther.ebert@ixos-leipzig.de

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <lm.h>
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
#include "miscfuncs.h"
#include "ldap.h"
#include "tls_pbuf.h"

static char * NO_COPY_RO null_ptr;

bool
pwdgrp::parse_group ()
{
  pg_grp &grp = group ()[curr_lines];
  grp.g.gr_name = next_str (':');
  if (!*grp.g.gr_name)
    return false;
  grp.g.gr_passwd = next_str (':');
  if (!next_num (grp.g.gr_gid))
    return false;
  int n;
  char *dp = raw_ptr ();
  for (n = 0; *next_str (','); n++)
    continue;
  grp.g.gr_mem = &null_ptr;
  if (n)
    {
      char **namearray = (char **) ccalloc (HEAP_BUF, n + 1, sizeof (char *));
      if (namearray)
	{
	  for (int i = 0; i < n; i++, dp = strchr (dp, '\0') + 1)
	    namearray[i] = dp;
	  grp.g.gr_mem = namearray;
	}
    }
  grp.sid.getfromgr (&grp.g);
  return true;
}

muto NO_COPY pwdgrp::pglock;

void
pwdgrp::init_grp ()
{
  pwdgrp_buf_elem_size = sizeof (pg_grp);
  parse = &pwdgrp::parse_group;
}

pwdgrp *
pwdgrp::prep_tls_grbuf ()
{
  if (!_my_tls.locals.grbuf)
    {
      _my_tls.locals.grbuf = ccalloc_abort (HEAP_BUF, 1,
					    sizeof (pwdgrp) + sizeof (pg_grp));
      pwdgrp *gr = (pwdgrp *) _my_tls.locals.grbuf;
      gr->init_grp ();
      gr->pwdgrp_buf = (void *) (gr + 1);
      gr->max_lines = 1;
    }
  pwdgrp *gr = (pwdgrp *) _my_tls.locals.grbuf;
  if (gr->curr_lines)
    {
      cfree (gr->group ()[0].g.gr_name);
      gr->curr_lines = 0;
    }
  return gr;
}         

struct group *
pwdgrp::find_group (cygpsid &sid)
{
  for (ULONG i = 0; i < curr_lines; i++)
    if (sid == group ()[i].sid)
      return &group ()[i].g;
  return NULL;
}

struct group *
pwdgrp::find_group (const char *name)
{
  for (ULONG i = 0; i < curr_lines; i++)
    if (strcasematch (group ()[i].g.gr_name, name))
      return &group ()[i].g;
  return NULL;
}

struct group *
pwdgrp::find_group (gid_t gid)
{
  for (ULONG i = 0; i < curr_lines; i++)
    if (gid == group ()[i].g.gr_gid)
      return &group ()[i].g;
  return NULL;
}

struct group *
internal_getgrsid (cygpsid &sid)
{
  struct group *ret;

  cygheap->pg.nss_init ();
  if (cygheap->pg.nss_grp_files ())
    {
      cygheap->pg.grp_cache.file.check_file (true);
      if ((ret = cygheap->pg.grp_cache.file.find_group (sid)))
	return ret;
      if ((ret = cygheap->pg.grp_cache.file.add_group_from_file (sid)))
	return ret;
    }
  if (cygheap->pg.nss_grp_db ())
    {
      if ((ret = cygheap->pg.grp_cache.win.find_group (sid)))
	return ret;
      return cygheap->pg.grp_cache.win.add_group_from_windows (sid);
    }
  return NULL;
}

struct group *
internal_getgrnam (const char *name)
{
  struct group *ret;

  cygheap->pg.nss_init ();
  if (cygheap->pg.nss_grp_files ())
    {
      cygheap->pg.grp_cache.file.check_file (true);
      if ((ret = cygheap->pg.grp_cache.file.find_group (name)))
	return ret;
      if ((ret = cygheap->pg.grp_cache.file.add_group_from_file (name)))
	return ret;
    }
  if (cygheap->pg.nss_grp_db ())
    {
      if ((ret = cygheap->pg.grp_cache.win.find_group (name)))
	return ret;
      return cygheap->pg.grp_cache.win.add_group_from_windows (name);
    }
  return NULL;
}

struct group *
internal_getgrgid (gid_t gid)
{
  struct group *ret;

  cygheap->pg.nss_init ();
  if (cygheap->pg.nss_grp_files ())
    {
      cygheap->pg.grp_cache.file.check_file (true);
      if ((ret = cygheap->pg.grp_cache.file.find_group (gid)))
	return ret;
      if ((ret = cygheap->pg.grp_cache.file.add_group_from_file (gid)))
	return ret;
    }
  if (cygheap->pg.nss_grp_db ())
    {
      if ((ret = cygheap->pg.grp_cache.win.find_group (gid)))
	return ret;
      return cygheap->pg.grp_cache.win.add_group_from_windows (gid);
    }
  else if (gid == ILLEGAL_GID)
    return cygheap->pg.grp_cache.win.add_group_from_windows (gid);
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

  struct group *tempgr = internal_getgrgid (gid);
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
  return internal_getgrgid (gid);
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

  struct group *tempgr = internal_getgrnam (nam);
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
  return internal_getgrnam (name);
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

/* getgrent functions are not reentrant. */
static gr_ent grent;

void *
gr_ent::enumerate_caches ()
{
  if (!max && from_files)
    {
      pwdgrp &grf = cygheap->pg.grp_cache.file;
      grf.check_file (true);
      if (cnt < grf.cached_groups ())
        return &grf.group ()[cnt++].g;
      cnt = 0;
      max = 1;
    }
  if (from_db && cygheap->pg.nss_db_caching ())
    {
      pwdgrp &grw = cygheap->pg.grp_cache.win;
      if (cnt < grw.cached_groups ())
        return &grw.group ()[cnt++].g;
    }
  cnt = max = 0;
  return NULL;
}

void *
gr_ent::enumerate_local ()
{
  while (true)
    {
      if (!cnt)
	{
	  DWORD total;
	  NET_API_STATUS ret;

	  if (buf)
	    {
	      NetApiBufferFree (buf);
	      buf = NULL;
	    }
	  if (resume == ULONG_MAX)
	    ret = ERROR_NO_MORE_ITEMS;
	  else
	    ret = NetLocalGroupEnum (NULL, 0, (PBYTE *) &buf,
				     MAX_PREFERRED_LENGTH,
				     &max, &total, &resume);
	  if (ret == NERR_Success)
	    resume = ULONG_MAX;
	  else if (ret != ERROR_MORE_DATA)
	    {
	      cnt = max = resume = 0;
	      return NULL;
	    }
	}
      while (cnt < max)
	{
	  cygsid sid;
	  DWORD slen = MAX_SID_LEN;
	  WCHAR dom[DNLEN + 1];
	  DWORD dlen = DNLEN + 1;
	  SID_NAME_USE acc_type;

	  LookupAccountNameW (NULL,
			      ((PLOCALGROUP_INFO_0) buf)[cnt++].lgrpi0_name,
			      sid, &slen, dom, &dlen, &acc_type);
	  fetch_user_arg_t arg;
	  arg.type = SID_arg;
	  arg.sid = &sid;
	  char *line = pg.fetch_account_from_windows (arg, true);
	  if (line)
	    return pg.add_account_post_fetch (line, false);
	}
      cnt = 0;
    }
}

struct group *
gr_ent::getgrent (void)
{
  if (state == rewound)
    setent (true);
  else
    clear_cache ();
  return (struct group *) getent ();
}

extern "C" void
setgrent ()
{
  grent.setgrent ();
}

extern "C" struct group *
getgrent32 (void)
{
  return grent.getgrent ();
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
endgrent (void)
{
  grent.endgrent ();
}

/* *_filtered functions are called from mkgroup */
void *
setgrent_filtered (int enums, PCWSTR enum_tdoms)
{
  gr_ent *gr = new gr_ent;
  if (gr)
    gr->setgrent (enums, enum_tdoms);
  return (void *) gr;
}

void *
getgrent_filtered (void *gr)
{
  return (void *) ((gr_ent *) gr)->getgrent ();
}

void
endgrent_filtered (void *gr)
{
  ((gr_ent *) gr)->endgrent ();
}

int
internal_getgroups (int gidsetsize, gid_t *grouplist, cygpsid *srchsid)
{
  NTSTATUS status;
  HANDLE hToken = NULL;
  ULONG size;
  int cnt = 0;
  struct group *grp;

  if (!srchsid && cygheap->user.groups.issetgroups ())
    {
      for (int pg = 0; pg < cygheap->user.groups.sgsids.count (); ++pg)
	if ((grp = internal_getgrsid (cygheap->user.groups.sgsids.sids[pg])))
	  {
	    if (cnt < gidsetsize)
	      grouplist[cnt] = grp->gr_gid;
	    ++cnt;
	    if (gidsetsize && cnt > gidsetsize)
	      goto error;
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
	    {
	      for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
		{
		  cygpsid sid = groups->Groups[pg].Sid;
		  if ((grp = internal_getgrsid (sid)))
		    {
		      if ((groups->Groups[pg].Attributes
			  & (SE_GROUP_ENABLED | SE_GROUP_INTEGRITY_ENABLED))
			  && sid != well_known_world_sid)
			{
			  if (cnt < gidsetsize)
			    grouplist[cnt] = grp->gr_gid;
			  ++cnt;
			  if (gidsetsize && cnt > gidsetsize)
			    goto error;
			}
		    }
		}
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
  struct group *grp = internal_getgrgid (gid);
  cygsid usersid, grpsid;
  if (usersid.getfrompw (pw))
    get_server_groups (gsids, usersid, pw);
  if (gid != ILLEGAL_GID && grpsid.getfromgr (grp))
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
  struct group *grp;

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
    if ((grp = internal_getgrsid (tmp_gsids.sids[i])) != NULL)
      {
	if (groups && cnt < *ngroups)
	  groups[cnt] = grp->gr_gid;
	++cnt;
      }
  *ngroups = cnt;
  if (cnt > *ngroups)
    ret = -1;
  else
    ret = cnt;

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
  struct group *grp;

  if (ngroups && !gsids.sids)
    return -1;

  for (int gidx = 0; gidx < ngroups; ++gidx)
    {
      if ((grp = internal_getgrgid (grouplist[gidx]))
	  && gsids.addfromgr (grp))
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
