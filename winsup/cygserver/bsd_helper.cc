/* bsd_helper.cc

   Copyright 2003, 2004, 2005, 2007 Red Hat Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */
#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"
#include <errno.h>
#define _KERNEL 1
#define __BSD_VISIBLE 1
#include <sys/smallprint.h>
#include <sys/cygwin.h>
#include <sys/ipc.h>
#include <sys/param.h>
#include <sys/msg.h>
#include <sys/queue.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "cygserver.h"
#include "process.h"
#include "cygserver_ipc.h"
#include "cygserver_msg.h"
#include "cygserver_sem.h"
#include "cygserver_shm.h"

/*
 * Copy a piece of memory from the client process into the server process.
 * Returns an error code.
 */
int
win_copyin (struct thread *td, const void *client_src,
	    void *server_tgt, size_t len)
{
  if (!ReadProcessMemory (td->client->handle (), client_src, server_tgt,
			  len, NULL))
    return cygwin_internal (CW_GET_ERRNO_FROM_WINERROR,
			    GetLastError (), EINVAL);
  return 0;
}

/*
 * Copy a piece of memory from the server process into the client process.
 * Returns an error code.
 */
int
win_copyout (struct thread *td, const void *server_src,
	     void *client_tgt, size_t len)
{
  if (!WriteProcessMemory (td->client->handle (), client_tgt, server_src,
			   len, NULL))
    return cygwin_internal (CW_GET_ERRNO_FROM_WINERROR,
			    GetLastError (), EINVAL);
  return 0;
}

#define enter_critical_section(c) _enter_critical_section((c),__FILE__,__LINE__)
static void
_enter_critical_section (LPCRITICAL_SECTION pcs, const char *file, int line)
{
  _log (file, line, LOG_DEBUG, "Try enter critical section(%p)", pcs);
  EnterCriticalSection (pcs);
  _log (file, line, LOG_DEBUG, "Entered   critical section(%p)", pcs);
}

#define leave_critical_section(c) _leave_critical_section((c),__FILE__,__LINE__)
static void
_leave_critical_section (LPCRITICAL_SECTION pcs, const char *file, int line)
{
  LeaveCriticalSection (pcs);
  _log (file, line, LOG_DEBUG, "Left      critical section(%p)", pcs);
}

CRITICAL_SECTION ipcht_cs;

struct ipc_hookthread_storage {
  HANDLE process_hdl;
  proc ipcblk;
};

struct ipc_hookthread {
  SLIST_ENTRY (ipc_hookthread) sht_next;
  HANDLE thread;
  DWORD  winpid;
  struct vmspace vmspace;
};
static SLIST_HEAD (, ipc_hookthread) ipcht_list; /* list of hook threads */

static HANDLE ipcexit_event;

struct vmspace *
ipc_p_vmspace (struct proc *proc)
{
  struct vmspace *ret = NULL;
  ipc_hookthread *ipcht_entry;
  enter_critical_section (&ipcht_cs);
  SLIST_FOREACH (ipcht_entry, &ipcht_list, sht_next)
    {
      if (ipcht_entry->winpid == proc->winpid)
        {
	  ret = proc->p_vmspace = &ipcht_entry->vmspace;
	  break;
	}
    }
  leave_critical_section (&ipcht_cs);
  return ret;
}

static DWORD WINAPI
ipcexit_hookthread (const LPVOID param)
{
  ipc_hookthread_storage *shs = (ipc_hookthread_storage *) param;
  HANDLE obj[2] = { ipcexit_event, shs->process_hdl };
  switch (WaitForMultipleObjects (2, obj, FALSE, INFINITE))
    {
      case WAIT_OBJECT_0:
        /* Cygserver shutdown. */
	/*FALLTHRU*/
      case WAIT_OBJECT_0 + 1:
        /* Process exited.  Call semexit_myhook to handle SEM_UNDOs for the
	   exiting process and shmexit_myhook to keep track of shared
	   memory. */
	if (Giant.owner == shs->ipcblk.winpid)
	  mtx_unlock (&Giant);
	if (support_semaphores == TUN_TRUE)
	  semexit_myhook (NULL, &shs->ipcblk);
	if (support_sharedmem == TUN_TRUE)
	  {
	    _mtx_lock (&Giant, shs->ipcblk.winpid, __FILE__, __LINE__);
	    ipc_p_vmspace (&shs->ipcblk);
	    shmexit_myhook (shs->ipcblk.p_vmspace);
	    mtx_unlock (&Giant);
	  }
	break;
      default:
        /* FIXME: Panic? */
	break;
    }
  CloseHandle (shs->process_hdl);
  ipc_hookthread *ipcht_entry, *sav_entry;
  enter_critical_section (&ipcht_cs);
  SLIST_FOREACH_SAFE (ipcht_entry, &ipcht_list, sht_next, sav_entry)
    {
      if (ipcht_entry->winpid == shs->ipcblk.winpid)
        {
	  SLIST_REMOVE (&ipcht_list, ipcht_entry, ipc_hookthread, sht_next);
	  CloseHandle (ipcht_entry->thread);
	  delete ipcht_entry;
	}
    }
  leave_critical_section (&ipcht_cs);
  delete shs;
  return 0;
}

/* Deletes all pending hook threads.  Called by ipcunload() which in turn
   is called by the cygserver main routine. */
static void
ipcexit_dispose_hookthreads (void)
{
  SetEvent (ipcexit_event);
  ipc_hookthread *ipcht_entry;
  enter_critical_section (&ipcht_cs);
  SLIST_FOREACH (ipcht_entry, &ipcht_list, sht_next)
    {
      WaitForSingleObject (ipcht_entry->thread, 1000);
      /* Don't bother removing the linked list on cygserver shutdown. */
      /* FIXME: Error handling? */
    }
  leave_critical_section (&ipcht_cs);
}

/* Creates the per process wait thread.  Called by semget() under locked
   Giant mutex condition. */
int
ipcexit_creat_hookthread (struct thread *td)
{
  ipc_hookthread *ipcht_entry;
  int ret = -1;
  enter_critical_section (&ipcht_cs);
  SLIST_FOREACH (ipcht_entry, &ipcht_list, sht_next)
    {
      if (ipcht_entry->winpid == td->ipcblk->winpid)
	ret = 0;
    }
  leave_critical_section (&ipcht_cs);
  if (!ret)
    return 0;

  DWORD tid;
  ipc_hookthread_storage *shs = new ipc_hookthread_storage;
  if (!DuplicateHandle (GetCurrentProcess (), td->client->handle (),
			GetCurrentProcess (), &shs->process_hdl,
			0, FALSE, DUPLICATE_SAME_ACCESS))
    {
      log (LOG_CRIT, "failed to duplicate process handle, error = %lu",
		      GetLastError ());
      return cygwin_internal (CW_GET_ERRNO_FROM_WINERROR,
			      GetLastError (), ENOMEM);
    }
  shs->ipcblk = *td->ipcblk;
  HANDLE thread = CreateThread (NULL, 0, ipcexit_hookthread, shs, 0, &tid);
  if (!thread)
    {
      log (LOG_CRIT, "failed to create thread, error = %lu", GetLastError ());
      return cygwin_internal (CW_GET_ERRNO_FROM_WINERROR,
			      GetLastError (), ENOMEM);
    }
  ipcht_entry = new ipc_hookthread;
  ipcht_entry->thread = thread;
  ipcht_entry->winpid = td->ipcblk->winpid;
  ipcht_entry->vmspace.vm_map = NULL;
  ipcht_entry->vmspace.vm_shm = NULL;
  enter_critical_section (&ipcht_cs);
  SLIST_INSERT_HEAD (&ipcht_list, ipcht_entry, sht_next);
  leave_critical_section (&ipcht_cs);
  return 0;
}

/*
 * Need the admins group SID to compare with groups in client token.
 */
PSID admininstrator_group_sid;

static void
init_admin_sid (void)
{
  SID_IDENTIFIER_AUTHORITY nt_auth = {SECURITY_NT_AUTHORITY};
  if (! AllocateAndInitializeSid (&nt_auth, 2, 32, 544, 0, 0, 0, 0, 0, 0,
				  &admininstrator_group_sid))
    panic ("failed to create well known sids, error = %lu",
	   GetLastError ());
}

SECURITY_DESCRIPTOR sec_all_nih_sd;
SECURITY_ATTRIBUTES sec_all_nih = { sizeof (SECURITY_ATTRIBUTES),
				    &sec_all_nih_sd,
				    FALSE };

void
securityinit ()
{
  InitializeSecurityDescriptor (&sec_all_nih_sd, SECURITY_DESCRIPTOR_REVISION);
  SetSecurityDescriptorDacl (&sec_all_nih_sd, TRUE, 0, FALSE);
  init_admin_sid ();
}

/* Global vars, determining whether the IPC stuff should be started or not. */
tun_bool_t support_sharedmem = TUN_UNDEF;
tun_bool_t support_msgqueues = TUN_UNDEF;
tun_bool_t support_semaphores = TUN_UNDEF;

void
ipcinit ()
{
  mtx_init (&Giant, "Giant", NULL, MTX_DEF);
  msleep_init ();
  ipcexit_event = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (!ipcexit_event)
    panic ("Failed to create ipcexit event object");
  InitializeCriticalSection (&ipcht_cs);
  if (support_msgqueues == TUN_TRUE)
    msginit ();
  if (support_semaphores == TUN_TRUE)
    seminit ();
  if (support_sharedmem == TUN_TRUE)
    shminit ();
}

int
ipcunload ()
{
  ipcexit_dispose_hookthreads ();
  CloseHandle (ipcexit_event);
  wakeup_all ();
  if (support_semaphores == TUN_TRUE)
    semunload ();
  if (support_sharedmem == TUN_TRUE)
    shmunload ();
  if (support_msgqueues == TUN_TRUE)
    msgunload ();
  mtx_destroy (&Giant);
  return 0;
}

/*
 * Helper function to find a gid in a list of gids.
 */
static bool
is_grp_member (gid_t grp, gid_t *grplist, int listsize)
{
  if (grplist)
    for (; listsize > 0; --listsize)
      if (grp == grplist[listsize - 1])
	return true;
  return false;
}

/*
 * Helper function to get a specific token information from a token.
 * This function mallocs the necessary buffer spcae by itself.  It
 * must be free'd by the calling function.
 */
static void *
get_token_info (HANDLE tok, TOKEN_INFORMATION_CLASS tic)
{
  void *buf;
  DWORD size;

  if (!GetTokenInformation (tok, tic, NULL, 0, &size)
      && GetLastError () != ERROR_INSUFFICIENT_BUFFER)
    return NULL;
  if (!(buf = malloc (size)))
    return NULL;
  if (!GetTokenInformation (tok, tic, buf, size, &size))
    {
      free (buf);
      return NULL;
    }
  return buf;
}

/*
 * Check if client user helds "mode" permission when accessing object
 * associated with "perm" permission record.
 * Returns an error code.
 */
int
ipcperm (struct thread *td, ipc_perm *perm, unsigned int mode)
{
  proc *p = td->ipcblk;

  if (!suser (td))
    return 0;
  if (mode & IPC_M)
    {
      return (p->uid != perm->cuid && p->uid != perm->uid)
	     ? EACCES : 0;
    }
  if (p->uid != perm->cuid && p->uid != perm->uid)
    {
      /* If the user is a member of the creator or owner group, test
      	 against group bits, otherwise against other bits. */
      mode >>= p->gid != perm->gid && p->gid != perm->cgid
	       && !is_grp_member (perm->gid, p->gidlist, p->gidcnt)
	       && !is_grp_member (perm->cgid, p->gidlist, p->gidcnt)
	       ? 6 : 3;
    }
  return (mode & perm->mode) != mode ? EACCES : 0;
}

/*
 * Check for client user being superuser.
 * Returns an error code.
 */
int
suser (struct thread *td)
{
  /* This value has been set at ImpersonateNamedPipeClient() time
     using the token information.  See adjust_identity_info() below. */
  return td->ipcblk->is_admin ? 0 : EACCES;
}

/*
 * Retrieves user and group info from impersonated token and creates the
 * correct uid, gid, gidlist and is_admin entries in p from that.
 */
bool
adjust_identity_info (struct proc *p)
{
  HANDLE tok;

  if (!OpenThreadToken (GetCurrentThread (), TOKEN_READ, TRUE, &tok))
    {
      debug ("Failed to open worker thread access token for pid %d, winpid %d",
	     p->cygpid, p->winpid);
      return false;
    }

  /* Get uid from user SID in token. */
  PTOKEN_USER user;
  if (!(user = (PTOKEN_USER)get_token_info (tok, TokenUser)))
    goto faulty;
  p->uid = cygwin_internal (CW_GET_UID_FROM_SID, user->User.Sid);
  free (user);
  if (p->uid == (uid_t)-1)
    log (LOG_WARNING, "WARNING: User not found in /etc/passwd! Using uid -1!");

  /* Get gid from primary group SID in token. */
  PTOKEN_PRIMARY_GROUP pgrp;
  if (!(pgrp = (PTOKEN_PRIMARY_GROUP)get_token_info (tok, TokenPrimaryGroup)))
    goto faulty;
  p->gid = cygwin_internal (CW_GET_GID_FROM_SID, pgrp->PrimaryGroup);
  free (pgrp);
  if (p->gid == (gid_t)-1)
    log (LOG_WARNING,"WARNING: Group not found in /etc/passwd! Using gid -1!");

  /* Generate gid list from token group's SID list.  Also look if the token
     has an enabled admin group SID.  That means, the process has admin
     privileges.  That knowledge is used in suser(). */
  PTOKEN_GROUPS gsids;
  if (!(gsids = (PTOKEN_GROUPS)get_token_info (tok, TokenGroups)))
    goto faulty;
  if (gsids->GroupCount)
    {
      p->gidlist = (gid_t *) calloc (gsids->GroupCount, sizeof (gid_t));
      if (p->gidlist)
        p->gidcnt = gsids->GroupCount;
    }
  for (DWORD i = 0; i < gsids->GroupCount; ++i)
    {
      if (p->gidlist)
	p->gidlist[i] = cygwin_internal (CW_GET_GID_FROM_SID,
					 gsids->Groups[i].Sid);
      if (EqualSid (gsids->Groups[i].Sid, admininstrator_group_sid)
      	  && (gsids->Groups[i].Attributes & SE_GROUP_ENABLED))
	p->is_admin = true;
    }
  free (gsids);

  CloseHandle (tok);
  return true;

faulty:
  CloseHandle (tok);
  log (LOG_CRIT, "Failed to get token information for pid %d, winpid %d",
		  p->cygpid, p->winpid);
  return false;
}

/*
 * Windows wrapper implementation of the VM functions called by sysv_shm.cc.
 */

vm_object_t
_vm_pager_allocate (int size, int shmflg)
{
  /* Create the file mapping object with full access for everyone.  This is
     necessary to allow later calls to shmctl(..., IPC_SET,...) to
     change the access rights and ownership of a shared memory region.
     The access rights are tested at the beginning of every shm... function.
     Note that this does not influence the actual read or write access
     defined in a call to shmat. */
  vm_object_t object = CreateFileMapping (INVALID_HANDLE_VALUE, &sec_all_nih,
					  PAGE_READWRITE, 0, size, NULL);
  if (!object)
    panic ("CreateFileMapping in _vm_pager_allocate failed, %lu", GetLastError ());
  return object;
}

vm_object_t
vm_object_duplicate (struct thread *td, vm_object_t object)
{
  vm_object_t dup_object;
  if (!DuplicateHandle (GetCurrentProcess (), object,
		        td->client->handle (), &dup_object,
		        0, TRUE, DUPLICATE_SAME_ACCESS))
    panic ("!DuplicateHandle in vm_object_duplicate failed, %lu", GetLastError ());
  return dup_object;
}

void
vm_object_deallocate (vm_object_t object)
{
  if (object)
    CloseHandle (object);
}

/*
 * Tunable parameters are read from a system wide cygserver.conf file.
 * On the first call to tunable_int_fetch, the file is read and the
 * parameters are set accordingly.  Each parameter has default, max and
 * min settings.
 */

enum tun_params_type {
  TUN_NULL,
  TUN_INT,
  TUN_BOOL
};

union tun_value {
  long ival;
  tun_bool_t bval;
};

struct tun_struct {
  const char *name;
  tun_params_type type;
  union tun_value value;
  union tun_value min;
  union tun_value max;
  void (*check_func)(tun_struct *, char *, const char *);
};

static void
default_tun_check (tun_struct *that, char *value, const char *fname)
{
  char *c = NULL;
  tun_value val;
  switch (that->type)
    {
      case TUN_INT:
	val.ival = strtoul (value, &c, 10);
	if (!val.ival || (c && *c))
	  panic ("Error in config file %s: Value of parameter %s malformed",
		 fname, that->name);
        if (val.ival < that->min.ival || val.ival > that->max.ival)
	  panic ("Error in config file %s: Value of parameter %s must be "
		 "between %lu and %lu",
		 fname, that->name, that->min.ival, that->max.ival);
	if (that->value.ival)
	  panic ("Error in config file %s: Parameter %s set twice.\n",
		 fname, that->name);
	that->value.ival = val.ival;
	break;
      case TUN_BOOL:
        if (!strcasecmp (value, "no") || !strcasecmp (value, "n")
	    || !strcasecmp (value, "false") || !strcasecmp (value, "f")
	    || !strcasecmp (value, "0"))
	  val.bval = TUN_FALSE;
	else if (!strcasecmp (value, "yes") || !strcasecmp (value, "y")
		 || !strcasecmp (value, "true") || !strcasecmp (value, "t")
		 || !strcasecmp (value, "1"))
	  val.bval = TUN_TRUE;
	else
	  panic ("Error in config file %s: Value of parameter %s malformed\n"
	  	 "Allowed values: \"yes\", \"no\", \"y\", \"n\", \"true\", \"false\", \"t\", \"f\", \"1\" and \"0\"", fname, that->name);
	that->value.bval = val.bval;
        break;
      default:
	/* Shouldn't happen. */
        panic ("Internal error: Wrong type of tunable parameter");
	break;
    }
}

static tun_struct tunable_params[] =
{
  /* SRV */
  { "kern.srv.cleanup_threads", TUN_INT, {0}, {1}, {32}, default_tun_check},
  { "kern.srv.request_threads", TUN_INT, {0}, {1}, {310}, default_tun_check},
  { "kern.srv.process_cache_size", TUN_INT, {0}, {1}, {310}, default_tun_check},
  { "kern.srv.sharedmem", TUN_BOOL, {TUN_UNDEF}, {TUN_FALSE}, {TUN_TRUE}, default_tun_check},
  { "kern.srv.msgqueues", TUN_BOOL, {TUN_UNDEF}, {TUN_FALSE}, {TUN_TRUE}, default_tun_check},
  { "kern.srv.semaphores", TUN_BOOL, {TUN_UNDEF}, {TUN_FALSE}, {TUN_TRUE}, default_tun_check},

  /* LOG */
  { "kern.log.syslog", TUN_BOOL, {TUN_UNDEF}, {TUN_FALSE}, {TUN_TRUE}, default_tun_check},
  { "kern.log.stderr", TUN_BOOL, {TUN_UNDEF}, {TUN_FALSE}, {TUN_TRUE}, default_tun_check},
  { "kern.log.debug", TUN_BOOL, {TUN_UNDEF}, {TUN_FALSE}, {TUN_TRUE}, default_tun_check},
  { "kern.log.level", TUN_INT, {0}, {1}, {7}, default_tun_check},

  /* MSG */
  { "kern.ipc.msgseg", TUN_INT, {0}, {256}, {65535}, default_tun_check},
  { "kern.ipc.msgssz", TUN_INT, {0}, {8}, {1024}, default_tun_check},
  { "kern.ipc.msgmnb", TUN_INT, {0}, {1}, {65535}, default_tun_check},
  { "kern.ipc.msgmni", TUN_INT, {0}, {1}, {1024}, default_tun_check},
  { "kern.ipc.msgtql", TUN_INT, {0}, {1}, {1024}, default_tun_check},

  /* SEM */
  //{ "kern.ipc.semmap", TUN_INT, {0}, {1}, {1024}, default_tun_check},
  { "kern.ipc.semmni", TUN_INT, {0}, {1}, {1024}, default_tun_check},
  { "kern.ipc.semmns", TUN_INT, {0}, {1}, {1024}, default_tun_check},
  { "kern.ipc.semmnu", TUN_INT, {0}, {1}, {1024}, default_tun_check},
  { "kern.ipc.semmsl", TUN_INT, {0}, {1}, {1024}, default_tun_check},
  { "kern.ipc.semopm", TUN_INT, {0}, {1}, {1024}, default_tun_check},
  { "kern.ipc.semume", TUN_INT, {0}, {1}, {1024}, default_tun_check},
  //{ "kern.ipc.semusz", TUN_INT, {0}, {1}, {1024}, default_tun_check},
  { "kern.ipc.semvmx", TUN_INT, {0}, {1}, {32767}, default_tun_check},
  { "kern.ipc.semaem", TUN_INT, {0}, {1}, {32767}, default_tun_check},

  /* SHM */
  { "kern.ipc.shmmaxpgs", TUN_INT, {0}, {1}, {32767}, default_tun_check},
  //{ "kern.ipc.shmmin", TUN_INT, {0}, {1}, {32767}, default_tun_check},
  { "kern.ipc.shmmni", TUN_INT, {0}, {1}, {32767}, default_tun_check},
  { "kern.ipc.shmseg", TUN_INT, {0}, {1}, {32767}, default_tun_check},
  { "kern.ipc.shm_allow_removed", TUN_BOOL, {TUN_UNDEF}, {TUN_FALSE}, {TUN_TRUE}, default_tun_check},
  //{ "kern.ipc.shm_use_phys", TUN_INT, {0}, {1}, {32767}, default_tun_check},
  { NULL, TUN_NULL, {0}, {0}, {0}, NULL}
};

#define skip_whitespace(c)	while (*(c) && isspace (*(c))) ++(c)
#define skip_nonwhitespace(c)	while (*(c) && !isspace (*(c)) && *(c) != '#') ++(c)
#define end_of_content(c)	(!*(c) || *(c) == '#')

void
tunable_param_init (const char *config_file, bool force)
{
  FILE *fp = fopen (config_file, "rt");
  if (!fp)
    {
      if (force)
        panic ("can't open config file %s\n", config_file);
      return;
    }
  char line[1024];
  while (fgets (line, 1024, fp))
    {
      char *c = strrchr (line, '\n');
      if (!c)
        panic ("Line too long in confg file %s\n", config_file);
      /* Overwrite trailing NL. */
      *c = '\0';
      c = line;
      skip_whitespace (c);
      if (end_of_content (c))
        continue;
      /* So we are on the first character of a parameter name. */
      char *name = c;
      /* Find end of name. */
      skip_nonwhitespace (c);
      if (end_of_content (c))
	{
	  *c++ = '\0';
	  panic ("Error in config file %s: Parameter %s has no value.\n",
		 config_file, name);
	}
      /* Mark end of name. */
      *c++ = '\0';
      skip_whitespace (c);
      if (end_of_content (c))
        panic ("Error in config file %s: Parameter %s has no value.\n",
	       config_file, name);
      /* Now we are on the first character of a parameter's value. */
      char *value = c;
      /* This only works for simple parameters.  If complex string parameters
         are added at one point, the scanning routine must be changed here. */
      /* Find end of value. */
      skip_nonwhitespace (c);
      /* Mark end of value. */
      *c++ = '\0';
      /* Now look if name is one from our list. */
      tun_struct *s;
      for (s = &tunable_params[0]; s->name; ++s)
	if (!strcmp (name, s->name))
	  {
	    /* Now read value and check for validity.  check_func doesn't
	       return on error. */
	    s->check_func (s, value, config_file);
	    break;
	  }
      if (!s->name)
        panic ("Error in config file %s: Unknown parameter %s.\n",
	       config_file, name);
    }
  fclose (fp);
}

void
tunable_int_fetch (const char *name, long *tunable_target)
{
  tun_struct *s;
  for (s = &tunable_params[0]; s->name; ++s)
    if (!strcmp (name, s->name))
      break;
  if (!s)			/* Not found */
    return;
  if (s->type != TUN_INT)	/* Wrong type */
    return;
  if (!s->value.ival)		/* Not set in config file */
    return;
  *tunable_target = s->value.ival;
  debug ("\nSet %s to %lu\n", name, *tunable_target);
}

void
tunable_bool_fetch (const char *name, tun_bool_t *tunable_target)
{
  tun_struct *s;
  const char *tun_bool_val_string[] = { "undefined", "no", "yes" };
  for (s = &tunable_params[0]; s->name; ++s)
    if (!strcmp (name, s->name))
      break;
  if (!s)			/* Not found */
    return;
  if (s->type != TUN_BOOL)	/* Wrong type */
    return;
  if (!s->value.ival)		/* Not set in config file */
    return;
  *tunable_target = s->value.bval;
  debug ("\nSet %s to %s\n", name, tun_bool_val_string[*tunable_target]);
}
#endif /* __OUTSIDE_CYGWIN__ */
