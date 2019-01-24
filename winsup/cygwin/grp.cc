/* grp.cc

   Original stubs by Jason Molenda of Cygnus Support, crash@cygnus.com
   First implementation by Gunther Ebert, gunther.ebert@ixos-leipzig.de

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <lm.h>
#include <ntsecapi.h>
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
  /* Note that lptr points to the first byte of the gr_gid field.
     We deliberately ignore the gr_gid and gr_mem entries when copying
     the buffer content since they are not referenced anymore. */
  grp.len = lptr - grp.g.gr_name;
  if (!next_num (grp.g.gr_gid))
    return false;
  /* Don't generate gr_mem entries. */
  grp.g.gr_mem = &null_ptr;
  cygsid csid;
  if (csid.getfromgr_passwd (&grp.g))
    RtlCopySid (SECURITY_MAX_SID_SIZE, grp.sid, csid);
  return true;
}

muto NO_COPY pwdgrp::pglock;

void
pwdgrp::init_grp ()
{
  pwdgrp_buf_elem_size = sizeof (pg_grp);
  parse = &pwdgrp::parse_group;
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
internal_getgrsid (cygpsid &sid, cyg_ldap *pldap)
{
  struct group *ret;

  cygheap->pg.nss_init ();
  /* Check caches first. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.grp_cache.cygserver.find_group (sid)))
    return ret;
  if (cygheap->pg.nss_grp_files ()
      && (ret = cygheap->pg.grp_cache.file.find_group (sid)))
    return ret;
  if (cygheap->pg.nss_grp_db ()
      && (ret = cygheap->pg.grp_cache.win.find_group (sid)))
    return ret;
  /* Ask sources afterwards. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.grp_cache.cygserver.add_group_from_cygserver (sid)))
    return ret;
  if (cygheap->pg.nss_grp_files ())
    {
      cygheap->pg.grp_cache.file.check_file ();
      if ((ret = cygheap->pg.grp_cache.file.add_group_from_file (sid)))
	return ret;
    }
  if (cygheap->pg.nss_grp_db ())
    return cygheap->pg.grp_cache.win.add_group_from_windows (sid, pldap);
  return NULL;
}

/* Like internal_getgrsid but return only already cached data,
   NULL otherwise. */
static struct group *
internal_getgrsid_cachedonly (cygpsid &sid)
{
  struct group *ret;

  /* Check caches only. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.grp_cache.cygserver.find_group (sid)))
    return ret;
  if (cygheap->pg.nss_grp_files ()
      && (ret = cygheap->pg.grp_cache.file.find_group (sid)))
    return ret;
  if (cygheap->pg.nss_grp_db ()
      && (ret = cygheap->pg.grp_cache.win.find_group (sid)))
    return ret;
  return NULL;
}

/* Called from internal_getgroups.  The full information required to create
   a group account entry is already available from the LookupAccountSids
   call.  internal_getgrfull passes all available info into
   pwdgrp::fetch_account_from_line, thus avoiding a LookupAccountSid call
   for each group.  This is quite a bit faster, especially in slower
   environments. */
static struct group * __attribute__((used))
internal_getgrfull (fetch_acc_t &full_acc, cyg_ldap *pldap)
{
  struct group *ret;

  cygheap->pg.nss_init ();
  /* Skip local caches, internal_getgroups already called
     internal_getgrsid_cachedonly. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.grp_cache.cygserver.add_group_from_cygserver
      							(full_acc.sid)))
    return ret;
  if (cygheap->pg.nss_grp_files ())
    {
      cygheap->pg.grp_cache.file.check_file ();
      if ((ret = cygheap->pg.grp_cache.file.add_group_from_file
      							(full_acc.sid)))
	return ret;
    }
  if (cygheap->pg.nss_grp_db ())
    return cygheap->pg.grp_cache.win.add_group_from_windows (full_acc, pldap);
  return NULL;
}

/* This function gets only called from mkgroup via cygwin_internal. */
struct group *
internal_getgrsid_from_db (cygpsid &sid)
{
  cygheap->pg.nss_init ();
  return cygheap->pg.grp_cache.win.add_group_from_windows (sid);
}

struct group *
internal_getgrnam (const char *name, cyg_ldap *pldap)
{
  struct group *ret;

  cygheap->pg.nss_init ();
  /* Check caches first. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.grp_cache.cygserver.find_group (name)))
    return ret;
  if (cygheap->pg.nss_grp_files ()
      && (ret = cygheap->pg.grp_cache.file.find_group (name)))
    return ret;
  if (cygheap->pg.nss_grp_db ()
      && (ret = cygheap->pg.grp_cache.win.find_group (name)))
    return ret;
  /* Ask sources afterwards. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.grp_cache.cygserver.add_group_from_cygserver (name)))
    return ret;
  if (cygheap->pg.nss_grp_files ())
    {
      cygheap->pg.grp_cache.file.check_file ();
      if ((ret = cygheap->pg.grp_cache.file.add_group_from_file (name)))
	return ret;
    }
  if (cygheap->pg.nss_grp_db ())
    return cygheap->pg.grp_cache.win.add_group_from_windows (name, pldap);
  return NULL;
}

struct group *
internal_getgrgid (gid_t gid, cyg_ldap *pldap)
{
  struct group *ret;

  cygheap->pg.nss_init ();
  /* Check caches first. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.grp_cache.cygserver.find_group (gid)))
    return ret;
  if (cygheap->pg.nss_grp_files ()
      && (ret = cygheap->pg.grp_cache.file.find_group (gid)))
    return ret;
  if (cygheap->pg.nss_grp_db ()
      && (ret = cygheap->pg.grp_cache.win.find_group (gid)))
    return ret;
  /* Ask sources afterwards. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.grp_cache.cygserver.add_group_from_cygserver (gid)))
    return ret;
  if (cygheap->pg.nss_grp_files ())
    {
      cygheap->pg.grp_cache.file.check_file ();
      if ((ret = cygheap->pg.grp_cache.file.add_group_from_file (gid)))
	return ret;
    }
  if (cygheap->pg.nss_grp_db () || gid == ILLEGAL_GID)
    return cygheap->pg.grp_cache.win.add_group_from_windows (gid, pldap);
  return NULL;
}

#ifdef __i386__
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

  /* Check needed buffer size.  Deliberately ignore gr_mem. */
  size_t needsize = strlen (tempgr->gr_name) + strlen (tempgr->gr_passwd)
		    + 2 + sizeof (char *);
  if (needsize > bufsize)
    return ERANGE;

  /* Make a copy of tempgr.  Deliberately ignore gr_mem. */
  *result = grp;
  grp->gr_gid = tempgr->gr_gid;
  buffer = stpcpy (grp->gr_name = buffer, tempgr->gr_name);
  buffer = stpcpy (grp->gr_passwd = buffer + 1, tempgr->gr_passwd);
  grp->gr_mem = (char **) (buffer + 1);
  grp->gr_mem[0] = NULL;
  return 0;
}

/* getgrgid/getgrnam are not reentrant. */
static struct {
  struct group g;
  char *buf;
  size_t bufsiz;
} app_gr;

static struct group *
getgr_cp (struct group *tempgr)
{
  if (!tempgr)
    return NULL;
  pg_grp *gr = (pg_grp *) tempgr;
  if (app_gr.bufsiz < gr->len)
    {
      char *newbuf = (char *) realloc (app_gr.buf, gr->len);
      if (!newbuf)
        {
          set_errno (ENOMEM);
          return NULL;
        }
      app_gr.buf = newbuf;
      app_gr.bufsiz = gr->len;
    }
  memcpy (app_gr.buf, gr->g.gr_name, gr->len);
  memcpy (&app_gr.g, &gr->g, sizeof gr->g);
  ptrdiff_t diff = app_gr.buf - gr->g.gr_name;
  app_gr.g.gr_name += diff;
  app_gr.g.gr_passwd += diff;
  return &app_gr.g;
}

extern "C" struct group *
getgrgid32 (gid_t gid)
{
  struct group *tempgr = internal_getgrgid (gid);
  pthread_testcancel ();
  return getgr_cp (tempgr);
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

  /* Check needed buffer size.  Deliberately ignore gr_mem. */
  size_t needsize = strlen (tempgr->gr_name) + strlen (tempgr->gr_passwd)
		    + 2 + sizeof (char *);
  if (needsize > bufsize)
    return ERANGE;

  /* Make a copy of tempgr.  Deliberately ignore gr_mem. */
  *result = grp;
  grp->gr_gid = tempgr->gr_gid;
  buffer = stpcpy (grp->gr_name = buffer, tempgr->gr_name);
  buffer = stpcpy (grp->gr_passwd = buffer + 1, tempgr->gr_passwd);
  grp->gr_mem = (char **) (buffer + 1);
  grp->gr_mem[0] = NULL;
  return 0;
}

extern "C" struct group *
getgrnam32 (const char *name)
{
  struct group *tempgr = internal_getgrnam (name);
  pthread_testcancel ();
  return getgr_cp (tempgr);
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
  switch (max)
    {
    case 0:
      if (cygheap->pg.nss_cygserver_caching ())
	{
	  pwdgrp &grc = cygheap->pg.grp_cache.cygserver;
	  if (cnt < grc.cached_groups ())
	    return &grc.group ()[cnt++].g;
	}
      cnt = 0;
      max = 1;
      /*FALLTHRU*/
    case 1:
      if (from_files)
	{
	  pwdgrp &grf = cygheap->pg.grp_cache.file;
	  grf.check_file ();
	  if (cnt < grf.cached_groups ())
	    return &grf.group ()[cnt++].g;
	}
      cnt = 0;
      max = 2;
      /*FALLTHRU*/
    case 2:
      if (from_db)
	{
	  pwdgrp &grw = cygheap->pg.grp_cache.win;
	  if (cnt < grw.cached_groups ())
	    return &grw.group ()[cnt++].g;
	}
      break;
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
	  DWORD slen = SECURITY_MAX_SID_SIZE;
	  WCHAR dom[DNLEN + 1];
	  DWORD dlen = DNLEN + 1;
	  SID_NAME_USE acc_type;

	  LookupAccountNameW (NULL,
			      ((PLOCALGROUP_INFO_0) buf)[cnt++].lgrpi0_name,
			      sid, &slen, dom, &dlen, &acc_type);
	  fetch_user_arg_t arg;
	  arg.type = SID_arg;
	  arg.sid = &sid;
	  char *line = pg.fetch_account_from_windows (arg);
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
internal_getgroups (int gidsetsize, gid_t *grouplist, cyg_ldap *pldap)
{
  NTSTATUS status;
  HANDLE tok;
  ULONG size;
  PTOKEN_GROUPS groups;
  PSID *sidp_buf;
  ULONG scnt;
  PLSA_REFERENCED_DOMAIN_LIST dlst = NULL;
  PLSA_TRANSLATED_NAME nlst = NULL;

  tmp_pathbuf tp;
  struct group *grp;
  int cnt = 0;

  if (cygheap->user.groups.issetgroups ())
    {
      for (int pg = 0; pg < cygheap->user.groups.sgsids.count (); ++pg)
	if ((grp = internal_getgrsid (cygheap->user.groups.sgsids.sids[pg],
				      pldap)))
	  {
	    if (cnt < gidsetsize)
	      grouplist[cnt] = grp->gr_gid;
	    ++cnt;
	    if (gidsetsize && cnt > gidsetsize)
	      {
		cnt = -1;
		break;
	      }
	  }
      goto out;
    }

  /* If impersonated, use impersonation token. */
  tok = cygheap->user.issetuid () ? cygheap->user.primary_token ()
				  : hProcToken;

  /* Fetch groups from user token. */
  groups = (PTOKEN_GROUPS) tp.w_get ();
  status = NtQueryInformationToken (tok, TokenGroups, groups, 2 * NT_MAX_PATH,
				    &size);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQueryInformationToken(TokenGroups) %y", status);
      goto out;
    }
  /* Iterate over the group list and check which of them are already cached.
     Those are simply copied to grouplist.  The non-cached ones are collected
     in sidp_buf for a later call to LsaLookupSids. */
  sidp_buf = (PSID *) tp.w_get ();
  scnt = 0;
  for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
    {
      cygpsid sid = groups->Groups[pg].Sid;
      if ((groups->Groups[pg].Attributes
	  & (SE_GROUP_ENABLED | SE_GROUP_INTEGRITY_ENABLED)) == 0
	  || sid == well_known_world_sid)
	continue;
      if ((grp = internal_getgrsid_cachedonly (sid)))
	{
	  if (cnt < gidsetsize)
	    grouplist[cnt] = grp->gr_gid;
	  ++cnt;
	  if (gidsetsize && cnt > gidsetsize)
	    {
	      cnt = -1;
	      goto out;
	    }
	}
      else 
	sidp_buf[scnt++] = sid;
    }
  /* If there are non-cached groups left, try to fetch them. */
  if (scnt > 0)
    {
      /* Don't call LsaLookupSids if we're not utilizing the Windows account
	 DBs.  If we don't have access to the AD, which is one good reason to
	 disable passwd/group: db in nsswitch.conf, then the subsequent call
	 to LsaLookupSids will take 5 - 10 seconds in some environments. */
      if (!cygheap->pg.nss_grp_db ())
	{
	  for (DWORD pg = 0; pg < scnt; ++pg)
	    {
	      cygpsid sid = sidp_buf[pg];
	      if ((grp = internal_getgrsid (sid, NULL)))
		{
		  if (cnt < gidsetsize)
		    grouplist[cnt] = grp->gr_gid;
		  ++cnt;
		  if (gidsetsize && cnt > gidsetsize)
		    {
		      cnt = -1;
		      break;
		    }
		}
	    }
	  goto out;
	}
      /* Otherwise call LsaLookupSids and call internal_getgrfull on the
	 returned groups.  This performs a lot better than calling
	 internal_getgrsid on each group. */
      status = STATUS_ACCESS_DENIED;
      HANDLE lsa = lsa_open_policy (NULL, POLICY_LOOKUP_NAMES);
      if (!lsa)
	{
	  debug_printf ("POLICY_LOOKUP_NAMES right not given?");
	  goto out;
	}
      status = LsaLookupSids (lsa, scnt, sidp_buf, &dlst, &nlst);
      lsa_close_policy (lsa);
      if (NT_SUCCESS (status))
	{
	  for (ULONG ncnt = 0; ncnt < scnt; ++ncnt)
	    {
	      static UNICODE_STRING empty = { 0, 0, (PWSTR) L"" };
	      fetch_acc_t full_acc =
		{
		  .sid = sidp_buf[ncnt],
		  .name = &nlst[ncnt].Name,
		  .dom = &empty,
		  .acc_type = nlst[ncnt].Use
		};

	      if (nlst[ncnt].DomainIndex >= 0)
	        full_acc.dom = &dlst->Domains[nlst[ncnt].DomainIndex].Name;
	      if ((grp = internal_getgrfull (full_acc, pldap)))
		{
		  if (cnt < gidsetsize)
		    grouplist[cnt] = grp->gr_gid;
		  ++cnt;
		  if (gidsetsize && cnt > gidsetsize)
		    {
		      cnt = -1;
		      break;
		    }
		}
	    }
	}
    }

out:
  if (dlst)
    LsaFreeMemory (dlst);
  if (nlst)
    LsaFreeMemory (nlst);
  if (cnt == -1)
    set_errno (EINVAL);
  return cnt;
}

extern "C" int
getgroups32 (int gidsetsize, gid_t *grouplist)
{
  cyg_ldap cldap;

  return internal_getgroups (gidsetsize, grouplist, &cldap);
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

  int ret = getgroups32 (gidsetsize, grouplist32);

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
  cyg_ldap cldap;

  cygheap->user.deimpersonate ();
  struct passwd *pw = internal_getpwnam (user, &cldap);
  struct group *grp = internal_getgrgid (gid, &cldap);
  cygsid usersid, grpsid;
  if (usersid.getfrompw (pw))
    get_server_groups (gsids, usersid, NO_CHK_DISABLED);
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
  cyg_ldap cldap;

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
    if ((grp = internal_getgrsid (tmp_gsids.sids[i], &cldap)) != NULL)
      {
	if (groups && cnt < *ngroups)
	  groups[cnt] = grp->gr_gid;
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
  struct group *grp;
  cyg_ldap cldap;

  if (ngroups && !gsids.sids)
    return -1;

  for (int gidx = 0; gidx < ngroups; ++gidx)
    {
      if ((grp = internal_getgrgid (grouplist[gidx], &cldap))
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

#ifdef __i386__
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
#else
EXPORT_ALIAS (setgroups32, setgroups)
#endif
