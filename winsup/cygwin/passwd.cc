/* passwd.cc: getpwnam () and friends

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <lm.h>
#include <stdlib.h>
#include <stdio.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "pinfo.h"
#include "cygheap.h"
#include "shared_info.h"
#include "miscfuncs.h"
#include "ldap.h"
#include "tls_pbuf.h"

/* Parse /etc/passwd line into passwd structure. */
bool
pwdgrp::parse_passwd ()
{
  pg_pwd &res = passwd ()[curr_lines];
  res.p.pw_name = next_str (':');
  res.p.pw_passwd = next_str (':');
  if (!next_num (res.p.pw_uid))
    return false;
  if (!next_num (res.p.pw_gid))
    return false;
  res.p.pw_comment = NULL;
  res.p.pw_gecos = next_str (':');
  res.p.pw_dir =  next_str (':');
  res.p.pw_shell = next_str (':');
  cygsid csid;
  if (csid.getfrompw_gecos (&res.p))
    RtlCopySid (SECURITY_MAX_SID_SIZE, res.sid, csid);
  /* lptr points to the \0 after pw_shell.  Increment by one to get the correct
     required buffer len in getpw_cp. */
  res.len = lptr - res.p.pw_name + 1;
  return true;
}

void
pwdgrp::init_pwd ()
{
  pwdgrp_buf_elem_size = sizeof (pg_pwd);
  parse = &pwdgrp::parse_passwd;
}

struct passwd *
pwdgrp::find_user (cygpsid &sid)
{
  for (ULONG i = 0; i < curr_lines; i++)
    if (sid == passwd ()[i].sid)
      return &passwd ()[i].p;
  return NULL;
}

struct passwd *
pwdgrp::find_user (const char *name)
{
  for (ULONG i = 0; i < curr_lines; i++)
    /* on Windows NT user names are case-insensitive */
    if (strcasematch (name, passwd ()[i].p.pw_name))
      return &passwd ()[i].p;
  return NULL;
}

struct passwd *
pwdgrp::find_user (uid_t uid)
{
  for (ULONG i = 0; i < curr_lines; i++)
    if (uid == passwd ()[i].p.pw_uid)
      return &passwd ()[i].p;
  return NULL;
}

struct passwd *
internal_getpwsid (cygpsid &sid, cyg_ldap *pldap)
{
  struct passwd *ret;

  cygheap->pg.nss_init ();
  /* Check caches first. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.pwd_cache.cygserver.find_user (sid)))
    return ret;
  if (cygheap->pg.nss_pwd_files ()
      && (ret = cygheap->pg.pwd_cache.file.find_user (sid)))
    return ret;
  if (cygheap->pg.nss_pwd_db ()
      && (ret = cygheap->pg.pwd_cache.win.find_user (sid)))
    return ret;
  /* Ask sources afterwards. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.pwd_cache.cygserver.add_user_from_cygserver (sid)))
    return ret;
  if (cygheap->pg.nss_pwd_files ())
    {
      cygheap->pg.pwd_cache.file.check_file ();
      if ((ret = cygheap->pg.pwd_cache.file.add_user_from_file (sid)))
	return ret;
    }
  if (cygheap->pg.nss_pwd_db ())
    return cygheap->pg.pwd_cache.win.add_user_from_windows (sid, pldap);
  return NULL;
}

/* This function gets only called from mkpasswd via cygwin_internal. */
struct passwd *
internal_getpwsid_from_db (cygpsid &sid)
{
  cygheap->pg.nss_init ();
  return cygheap->pg.pwd_cache.win.add_user_from_windows (sid);
}

struct passwd *
internal_getpwnam (const char *name, cyg_ldap *pldap)
{
  struct passwd *ret;

  cygheap->pg.nss_init ();
  /* Check caches first. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.pwd_cache.cygserver.find_user (name)))
    return ret;
  if (cygheap->pg.nss_pwd_files ()
      && (ret = cygheap->pg.pwd_cache.file.find_user (name)))
    return ret;
  if (cygheap->pg.nss_pwd_db ()
      && (ret = cygheap->pg.pwd_cache.win.find_user (name)))
    return ret;
  /* Ask sources afterwards. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.pwd_cache.cygserver.add_user_from_cygserver (name)))
    return ret;
  if (cygheap->pg.nss_pwd_files ())
    {
      cygheap->pg.pwd_cache.file.check_file ();
      if ((ret = cygheap->pg.pwd_cache.file.add_user_from_file (name)))
	return ret;
    }
  if (cygheap->pg.nss_pwd_db ())
    return cygheap->pg.pwd_cache.win.add_user_from_windows (name, pldap);
  return NULL;
}

struct passwd *
internal_getpwuid (uid_t uid, cyg_ldap *pldap)
{
  struct passwd *ret;

  cygheap->pg.nss_init ();
  /* Check caches first. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.pwd_cache.cygserver.find_user (uid)))
    return ret;
  if (cygheap->pg.nss_pwd_files ()
      && (ret = cygheap->pg.pwd_cache.file.find_user (uid)))
    return ret;
  if (cygheap->pg.nss_pwd_db ()
      && (ret = cygheap->pg.pwd_cache.win.find_user (uid)))
    return ret;
  /* Ask sources afterwards. */
  if (cygheap->pg.nss_cygserver_caching ()
      && (ret = cygheap->pg.pwd_cache.cygserver.add_user_from_cygserver (uid)))
    return ret;
  if (cygheap->pg.nss_pwd_files ())
    {
      cygheap->pg.pwd_cache.file.check_file ();
      if ((ret = cygheap->pg.pwd_cache.file.add_user_from_file (uid)))
	return ret;
    }
  if (cygheap->pg.nss_pwd_db () || uid == ILLEGAL_UID)
    return cygheap->pg.pwd_cache.win.add_user_from_windows (uid, pldap);
  return NULL;
}

/* getpwuid/getpwnam are not reentrant. */
static struct {
  struct passwd p;
  char *buf;
  size_t bufsiz;
} app_pw;

static struct passwd *
getpw_cp (struct passwd *temppw)
{
  if (!temppw)
    return NULL;
  pg_pwd *pw = (pg_pwd *) temppw;
  if (app_pw.bufsiz < pw->len)
    {
      char *newbuf = (char *) realloc (app_pw.buf, pw->len);
      if (!newbuf)
	{
	  set_errno (ENOMEM);
	  return NULL;
	}
      app_pw.buf = newbuf;
      app_pw.bufsiz = pw->len;
    }
  memcpy (app_pw.buf, pw->p.pw_name, pw->len);
  memcpy (&app_pw.p, &pw->p, sizeof pw->p);
  ptrdiff_t diff = app_pw.buf - pw->p.pw_name;
  app_pw.p.pw_name += diff;
  app_pw.p.pw_passwd += diff;
  app_pw.p.pw_gecos += diff;
  app_pw.p.pw_dir += diff;
  app_pw.p.pw_shell += diff;
  return &app_pw.p;
}

extern "C" struct passwd *
getpwuid32 (uid_t uid)
{
  struct passwd *temppw = internal_getpwuid (uid);
  pthread_testcancel ();
  return getpw_cp (temppw);
}

#ifdef __i386__
extern "C" struct passwd *
getpwuid (__uid16_t uid)
{
  return getpwuid32 (uid16touid32 (uid));
}
#else
EXPORT_ALIAS (getpwuid32, getpwuid)
#endif

extern "C" int
getpwuid_r32 (uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
  *result = NULL;

  if (!pwd || !buffer)
    return ERANGE;

  struct passwd *temppw = internal_getpwuid (uid);
  pthread_testcancel ();
  if (!temppw)
    return 0;

  /* check needed buffer size. */
  size_t needsize = strlen (temppw->pw_name) + strlen (temppw->pw_passwd)
		    + strlen (temppw->pw_gecos) + strlen (temppw->pw_dir)
		    + strlen (temppw->pw_shell) + 5;
  if (needsize > bufsize)
    return ERANGE;

  /* make a copy of temppw */
  *result = pwd;
  pwd->pw_uid = temppw->pw_uid;
  pwd->pw_gid = temppw->pw_gid;
  buffer = stpcpy (pwd->pw_name = buffer, temppw->pw_name);
  buffer = stpcpy (pwd->pw_passwd = buffer + 1, temppw->pw_passwd);
  buffer = stpcpy (pwd->pw_gecos = buffer + 1, temppw->pw_gecos);
  buffer = stpcpy (pwd->pw_dir = buffer + 1, temppw->pw_dir);
  stpcpy (pwd->pw_shell = buffer + 1, temppw->pw_shell);
  pwd->pw_comment = NULL;
  return 0;
}

#ifdef __x86_64__
EXPORT_ALIAS (getpwuid_r32, getpwuid_r)
#else
extern "C" int
getpwuid_r (__uid16_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
  return getpwuid_r32 (uid16touid32 (uid), pwd, buffer, bufsize, result);
}
#endif

extern "C" struct passwd *
getpwnam (const char *name)
{
  struct passwd *temppw = internal_getpwnam (name);
  pthread_testcancel ();
  return getpw_cp (temppw);
}


/* the max size buffer we can expect to
 * use is returned via sysconf with _SC_GETPW_R_SIZE_MAX.
 * This may need updating! - Rob Collins April 2001.
 */
extern "C" int
getpwnam_r (const char *nam, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
  *result = NULL;

  if (!pwd || !buffer || !nam)
    return ERANGE;

  struct passwd *temppw = internal_getpwnam (nam);
  pthread_testcancel ();

  if (!temppw)
    return 0;

  /* check needed buffer size. */
  size_t needsize = strlen (temppw->pw_name) + strlen (temppw->pw_passwd)
		    + strlen (temppw->pw_gecos) + strlen (temppw->pw_dir)
		    + strlen (temppw->pw_shell) + 5;
  if (needsize > bufsize)
    return ERANGE;

  /* make a copy of temppw */
  *result = pwd;
  pwd->pw_uid = temppw->pw_uid;
  pwd->pw_gid = temppw->pw_gid;
  buffer = stpcpy (pwd->pw_name = buffer, temppw->pw_name);
  buffer = stpcpy (pwd->pw_passwd = buffer + 1, temppw->pw_passwd);
  buffer = stpcpy (pwd->pw_gecos = buffer + 1, temppw->pw_gecos);
  buffer = stpcpy (pwd->pw_dir = buffer + 1, temppw->pw_dir);
  stpcpy (pwd->pw_shell = buffer + 1, temppw->pw_shell);
  pwd->pw_comment = NULL;
  return 0;
}

/* getpwent functions are not reentrant. */
static pw_ent pwent;

void
pg_ent::clear_cache ()
{
  if (pg.curr_lines)
    {
      if (state > from_file)
	cfree (group ? grp.g.gr_name : pwd.p.pw_name);
      pg.curr_lines = 0;
    }
}

void
pg_ent::setent (bool _group, int _enums, PCWSTR _enum_tdoms)
{
  cygheap->dom.init ();
  endent (_group);
  if (!_enums && !_enum_tdoms)
    {
      /* This is the default, when called from the usual setpwent/setgrent
         functions. */
      enums = cygheap->pg.nss_db_enums ();
      enum_tdoms = cygheap->pg.nss_db_enum_tdoms ();
      if (_group)
	{
	  from_files = cygheap->pg.nss_grp_files ();
	  from_db = cygheap->pg.nss_grp_db ();
	}
      else
	{
	  from_files = cygheap->pg.nss_pwd_files ();
	  from_db = cygheap->pg.nss_pwd_db ();
	}
    }
  else
    {
      /* This case is when called from mkpasswd/mkgroup via cygwin_internal. */
      enums = _enums;
      enum_tdoms = _enum_tdoms;
      from_files = false;
      from_db = true;
    }
  state = from_cache;
}

void *
pg_ent::getent (void)
{
  void *entry;

  switch (state)
    {
    case rewound:
      state = from_cache;
      /*FALLTHRU*/
    case from_cache:
      if (nss_db_enum_caches ()
	  && (entry = enumerate_caches ()))
	return entry;
      state = from_file;
      /*FALLTHRU*/
    case from_file:
      if (from_files
	  && nss_db_enum_files ()
	  && (entry = enumerate_file ()))
	return entry;
      state = from_builtin;
      /*FALLTHRU*/
    case from_builtin:
      if (from_db
	  && nss_db_enum_builtin ()
	  && (entry = enumerate_builtin ()))
	return entry;
      state = from_local;
      /*FALLTHRU*/
    case from_local:
      if (from_db
	  && nss_db_enum_local ()
	  && (!cygheap->dom.member_machine ()
	      || !nss_db_enum_primary ())
	  && (entry = enumerate_local ()))
	return entry;
      state = from_sam;
      /*FALLTHRU*/
    case from_sam:
      if (from_db
	  && nss_db_enum_local ()
	  /* Domain controller?  If so, sam and ad are one and the same
	     and "local ad" would list all domain accounts twice without
	     this test. */
	  && (cygheap->dom.account_flat_name ()[0] != L'@'
	      || !nss_db_enum_primary ())
	  && (entry = enumerate_sam ()))
	return entry;
      state = from_ad;
      /*FALLTHRU*/
    case from_ad:
      if (cygheap->dom.member_machine ()
	  && from_db
	  && (entry = enumerate_ad ()))
	return entry;
      state = finished;
      /*FALLTHRU*/
    case finished:
      break;
    }
  return NULL;
}

void
pg_ent::endent (bool _group)
{
  if (buf)
    {
      if (state == from_file)
	free (buf);
      else if (state == from_local || state == from_sam)
      	NetApiBufferFree (buf);
      buf = NULL;
    }
  if (!pg.curr_lines)
    {
      if ((group = _group))
	{
	  pg.init_grp ();
	  pg.pwdgrp_buf = (void *) &grp;
	}
      else
	{
	  pg.init_pwd ();
	  pg.pwdgrp_buf = (void *) &pwd;
	}
      pg.max_lines = 1;
    }
  else
    clear_cache ();
  cldap.close ();
  rl.close ();
  cnt = max = resume = 0;
  enums = 0;
  enum_tdoms = NULL;
  state = rewound;
}

void *
pg_ent::enumerate_file ()
{
  void *entry;

  if (!cnt)
    {
      pwdgrp &prf = group ? cygheap->pg.grp_cache.file
			  : cygheap->pg.pwd_cache.file;
      if (prf.check_file ())
	{
	  if (!buf)
	    buf = (char *) malloc (NT_MAX_PATH);
	  if (buf
	      && !rl.init (prf.file_attr (), buf, NT_MAX_PATH))
	    {
	      free (buf);
	      buf = NULL;
	    }
	}
    }
  ++cnt;
  if ((entry = pg.add_account_post_fetch (rl.gets (), false)))
    return entry;
  rl.close ();
  free (buf);
  buf = NULL;
  cnt = max = resume = 0;
  return NULL;
}

void *
pg_ent::enumerate_builtin ()
{
  static cygpsid *pwd_builtins[] = {
    &well_known_system_sid,
    &well_known_local_service_sid,
    &well_known_network_service_sid,
    &well_known_admins_sid,
    &trusted_installer_sid,
    NULL
  };
  static cygpsid *grp_builtins[] = {
    &well_known_system_sid,
    &trusted_installer_sid,
    NULL
  };

  cygpsid **builtins = group ? grp_builtins : pwd_builtins;
  if (!builtins[cnt])
    {
      cnt = max = resume = 0;
      return NULL;
    }
  cygsid sid (*builtins[cnt++]);
  fetch_user_arg_t arg;
  arg.type = SID_arg;
  arg.sid = &sid;
  char *line = pg.fetch_account_from_windows (arg);
  return pg.add_account_post_fetch (line, false);
} 

void *
pg_ent::enumerate_sam ()
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
	  else if (group)
	    ret = NetGroupEnum (NULL, 2, (PBYTE *) &buf, MAX_PREFERRED_LENGTH,
				&max, &total, &resume);
	  else
	    ret = NetUserEnum (NULL, 20, FILTER_NORMAL_ACCOUNT, (PBYTE *) &buf,
			       MAX_PREFERRED_LENGTH, &max, &total,
			       (PDWORD) &resume);
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
	  cygsid sid (cygheap->dom.account_sid ());
	  sid_sub_auth (sid, sid_sub_auth_count (sid)) = 
	    group ? ((PGROUP_INFO_2) buf)[cnt].grpi2_group_id
		  : ((PUSER_INFO_20) buf)[cnt].usri20_user_id;
	  ++cnt;
	  ++sid_sub_auth_count (sid);
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

void *
pg_ent::enumerate_ad ()
{
  while (true)
    {
      if (!cnt)
	{
	  PDS_DOMAIN_TRUSTSW td;

	  if (!resume)
	    {
	      ++resume;
	      if (!nss_db_enum_primary ()
		  || cldap.enumerate_ad_accounts (NULL, group) != NO_ERROR)
		continue;
	      RtlInitUnicodeString (&dom, cygheap->dom.primary_flat_name ());
	    }
	  else if ((td = cygheap->dom.trusted_domain (resume - 1)))
	    {
	      ++resume;
	      /* Ignore primary domain in list of trusted domains only if all
		 trusted domains are enumerated anyway.  This handles an
		 annoying backward compatibility problem in mkpasswd/mkgroup.
		 Without this test, `mkpasswd -d PRIMARY_DOMAIN' wouldn't
		 work as expected. */
	      if (((enums & ENUM_TDOMS_ALL) && td->Flags & DS_DOMAIN_PRIMARY)
		  || !td->DomainSid
		  || (!nss_db_enum_tdom (td->NetbiosDomainName)
		      && !nss_db_enum_tdom (td->DnsDomainName))
		      || cldap.enumerate_ad_accounts (td->DnsDomainName, group)
			 != NO_ERROR)
		continue;
	      RtlInitUnicodeString (&dom, td->NetbiosDomainName);
	    }
	  else
	    {
	      cldap.close ();
	      return NULL;
	    }
	}
      ++cnt;
      cygsid sid;
      int ret = cldap.next_account (sid);
      if (ret == NO_ERROR)
	{
	  fetch_acc_t full;
	  fetch_user_arg_t arg;
	  UNICODE_STRING name;

	  arg.type = FULL_acc_arg;
	  arg.full_acc = &full;
	  full.sid = sid;
	  RtlInitUnicodeString (&name,
				cldap.get_string_attribute (L"sAMAccountName"));
	  full.name = &name;
	  full.dom = &dom;
	  if (sid_sub_auth (sid, 0) == SECURITY_BUILTIN_DOMAIN_RID)
	    full.acc_type = SidTypeAlias;
	  else
	    full.acc_type = group ? SidTypeGroup : SidTypeUser;
	  char *line = pg.fetch_account_from_windows (arg, &cldap);
	  if (line)
	    return pg.add_account_post_fetch (line, false);
	  ret = EIO;
	}
      if (ret != ENMFILE)
	{
	  cldap.close ();
	  set_errno (ret);
	  return NULL;
	}
      cnt = 0;
    }
}

void *
pw_ent::enumerate_caches ()
{
  switch (max)
    {
    case 0:
      if (cygheap->pg.nss_cygserver_caching ())
	{
	  pwdgrp &prc = cygheap->pg.pwd_cache.cygserver;
	  if (cnt < prc.cached_users ())
	    return &prc.passwd ()[cnt++].p;
	}
      cnt = 0;
      max = 1;
      /*FALLTHRU*/
    case 1:
      if (from_files)
	{
	  pwdgrp &prf = cygheap->pg.pwd_cache.file;
	  prf.check_file ();
	  if (cnt < prf.cached_users ())
	    return &prf.passwd ()[cnt++].p;
	}
      cnt = 0;
      max = 2;
      /*FALLTHRU*/
    default:
      if (from_db)
	{
	  pwdgrp &prw = cygheap->pg.pwd_cache.win;
	  if (cnt < prw.cached_users ())
	    return &prw.passwd ()[cnt++].p;
	}
      break;
    }
  cnt = max = 0;
  return NULL;
}

void *
pw_ent::enumerate_local ()
{
  return NULL;
}

struct passwd *
pw_ent::getpwent (void)
{
  if (state == rewound)
    setent (false);
  else
    clear_cache ();
  return (struct passwd *) getent ();
}

extern "C" void
setpwent ()
{
  pwent.setpwent ();
}

extern "C" struct passwd *
getpwent (void)
{
  return pwent.getpwent ();
}

extern "C" void
endpwent (void)
{
  pwent.endpwent ();
}

/* *_filtered functions are called from mkpasswd */
void *
setpwent_filtered (int enums, PCWSTR enum_tdoms)
{
  pw_ent *pw = new pw_ent;
  if (pw)
    pw->setpwent (enums, enum_tdoms);
  return (void *) pw;
}

void *
getpwent_filtered (void *pw)
{
  return (void *) ((pw_ent *) pw)->getpwent ();
}

void
endpwent_filtered (void *pw)
{
  ((pw_ent *) pw)->endpwent ();
}

#ifdef __i386__
extern "C" struct passwd *
getpwduid (__uid16_t)
{
  return NULL;
}
#endif

extern "C" int
setpassent (int)
{
  return 0;
}

static void
_getpass_close_fd (void *arg)
{
  if (arg)
    fclose ((FILE *) arg);
}

extern "C" char *
getpass (const char * prompt)
{
  char *pass = _my_tls.locals.pass;
  struct termios ti, newti;
  bool tc_set = false;

  /* Try to use controlling tty in the first place.  Use stdin and stderr
     only as fallback. */
  FILE *in = stdin, *err = stderr;
  FILE *tty = fopen ("/dev/tty", "w+b");
  pthread_cleanup_push  (_getpass_close_fd, tty);
  if (tty)
    {
      /* Set close-on-exec for obvious reasons. */
      fcntl (fileno (tty), F_SETFD, fcntl (fileno (tty), F_GETFD) | FD_CLOEXEC);
      in = err = tty;
    }

  /* Make sure to notice if stdin is closed. */
  if (fileno (in) >= 0)
    {
      flockfile (in);
      /* Change tty attributes if possible. */
      if (!tcgetattr (fileno (in), &ti))
	{
	  newti = ti;
	  newti.c_lflag &= ~(ECHO | ISIG); /* No echo, no signal handling. */
	  if (!tcsetattr (fileno (in), TCSANOW, &newti))
	    tc_set = true;
	}
      fputs (prompt, err);
      fflush (err);
      fgets (pass, _PASSWORD_LEN, in);
      fprintf (err, "\n");
      if (tc_set)
	tcsetattr (fileno (in), TCSANOW, &ti);
      funlockfile (in);
      char *crlf = strpbrk (pass, "\r\n");
      if (crlf)
	*crlf = '\0';
    }
  pthread_cleanup_pop (1);
  return pass;
}
