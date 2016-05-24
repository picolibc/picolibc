/* shm.cc: XSI IPC interface for Cygwin.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/queue.h>
#include <unistd.h>

#include "pinfo.h"
#include "sigproc.h"

#include "cygserver_shm.h"
#include "cygtls.h"
#include "sync.h"
#include "ntdll.h"

/* __getpagesize is only available from libcygwin.a */
#undef SHMLBA
#define SHMLBA (wincap.allocation_granularity ())

/*
 * client_request_shm Constructors
 */

client_request_shm::client_request_shm (int shmid,
					const void *shmaddr,
					int shmflg)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmat;
  ipc_set_proc_info (_parameters.in.ipcblk);

  _parameters.in.atargs.shmid = shmid;
  _parameters.in.atargs.shmaddr = shmaddr;
  _parameters.in.atargs.shmflg = shmflg;

  msglen (sizeof (_parameters.in));
}

client_request_shm::client_request_shm (int shmid,
					int cmd,
					struct shmid_ds *buf)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmctl;
  ipc_set_proc_info (_parameters.in.ipcblk);

   _parameters.in.ctlargs.shmid = shmid;
   _parameters.in.ctlargs.cmd = cmd;
   _parameters.in.ctlargs.buf = buf;

  msglen (sizeof (_parameters.in));
}

client_request_shm::client_request_shm (const void *shmaddr)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmdt;
  ipc_set_proc_info (_parameters.in.ipcblk);

  _parameters.in.dtargs.shmaddr = shmaddr;

  msglen (sizeof (_parameters.in));
}

client_request_shm::client_request_shm (key_t key,
					size_t size,
					int shmflg)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmget;
  ipc_set_proc_info (_parameters.in.ipcblk);

  _parameters.in.getargs.key = key;
  _parameters.in.getargs.size = size;
  _parameters.in.getargs.shmflg = shmflg;

  msglen (sizeof (_parameters.in));
}

client_request_shm::client_request_shm (proc *p1)
  : client_request (CYGSERVER_REQUEST_SHM, &_parameters, sizeof (_parameters))
{
  _parameters.in.shmop = SHMOP_shmfork;
  ipc_set_proc_info (_parameters.in.ipcblk, true);

  _parameters.in.forkargs = *p1;
}

/* List of shmid's with file mapping HANDLE and size, returned by shmget. */
struct shm_shmid_list {
  SLIST_ENTRY (shm_shmid_list) ssh_next;
  int shmid;
  vm_object_t hdl;
  size_t size;
  int ref_count;
};

static SLIST_HEAD (, shm_shmid_list) ssh_list;

/* List of attached mappings, as returned by shmat. */
struct shm_attached_list {
  SLIST_ENTRY (shm_attached_list) sph_next;
  vm_object_t ptr;
  shm_shmid_list *parent;
  ULONG access;
};

static SLIST_HEAD (, shm_attached_list) sph_list;

static NO_COPY muto shm_guard;
#define SLIST_LOCK()	(shm_guard.init ("shm_guard")->acquire ())
#define SLIST_UNLOCK()	(shm_guard.release ())

int __stdcall
fixup_shms_after_fork ()
{
  if (!SLIST_FIRST (&sph_list))
    return 0;
  pinfo p (myself->ppid);
  proc parent = { myself->ppid, p->dwProcessId, p->uid, p->gid };

  client_request_shm request (&parent);
  if (request.make_request () == -1 || request.retval () == -1)
    {
      syscall_printf ("-1 [%d] = fixup_shms_after_fork ()", request.error_code ());
      set_errno (request.error_code ());
      return 0;
    }
  shm_attached_list *sph_entry;
  /* Reconstruct map from list... */
  SLIST_FOREACH (sph_entry, &sph_list, sph_next)
    {
      NTSTATUS status;
      vm_object_t ptr = sph_entry->ptr;
      SIZE_T viewsize = sph_entry->parent->size;
      status = NtMapViewOfSection (sph_entry->parent->hdl, NtCurrentProcess (),
				   &ptr, 0, sph_entry->parent->size, NULL,
				   &viewsize, ViewShare, 0, sph_entry->access);
      if (!NT_SUCCESS (status) || ptr != sph_entry->ptr)
	api_fatal ("fixup_shms_after_fork: NtMapViewOfSection (%p), status %y.  Terminating.",
		   sph_entry->ptr, status);
    }
  return 0;
}

/*
 * XSI shmaphore API.  These are exported by the DLL.
 */

extern "C" void *
shmat (int shmid, const void *shmaddr, int shmflg)
{
  syscall_printf ("shmat (shmid = %d, shmaddr = %p, shmflg = %y)",
		  shmid, shmaddr, shmflg);

  SLIST_LOCK ();
  shm_shmid_list *ssh_entry;
  SLIST_FOREACH (ssh_entry, &ssh_list, ssh_next)
    {
      if (ssh_entry->shmid == shmid)
	break;
    }
  if (!ssh_entry)
    {
      /* The shmid is unknown to this process so far.  Try to get it from
	 the server if it exists.  Use special internal call to shmget,
	 which interprets the key as a shmid and only returns a valid
	 shmid if one exists.  Since shmctl inserts a new entry for this
	 shmid into ssh_list automatically, we just have to go through
	 that list again.  If that still fails, well, bad luck. */
      if (shmid && shmget ((key_t) shmid, 0, IPC_KEY_IS_SHMID) != -1)
	{
	  SLIST_FOREACH (ssh_entry, &ssh_list, ssh_next)
	    {
	      if (ssh_entry->shmid == shmid)
		break;
	    }
	}
      if (!ssh_entry)
	{
	  /* Invalid shmid */
	  set_errno (EINVAL);
	  SLIST_UNLOCK ();
	  return (void *) -1;
	}
    }
  /* Early increment ref counter.  This allows further actions to run with
     unlocked lists, because shmdt or shmctl(IPC_RMID) won't delete this
     ssh_entry. */
  ++ssh_entry->ref_count;
  SLIST_UNLOCK ();

  vm_object_t attach_va = NULL;
  if (shmaddr)
    {
      if (shmflg & SHM_RND)
	attach_va = (vm_object_t)((vm_offset_t)shmaddr & ~(SHMLBA-1));
      else
	attach_va = (vm_object_t)shmaddr;
      /* Don't even bother to call anything if shmaddr is NULL or
	 not aligned. */
      if (!attach_va || (vm_offset_t)attach_va % SHMLBA)
	{
	  set_errno (EINVAL);
	  --ssh_entry->ref_count;
	  return (void *) -1;
	}
    }
  /* Try allocating memory before calling cygserver. */
  shm_attached_list *sph_entry = new (shm_attached_list);
  if (!sph_entry)
    {
      set_errno (ENOMEM);
      --ssh_entry->ref_count;
      return (void *) -1;
    }
  NTSTATUS status;
  vm_object_t ptr = NULL;
  SIZE_T viewsize = ssh_entry->size;
  ULONG access = (shmflg & SHM_RDONLY) ? PAGE_READONLY : PAGE_READWRITE;
  status = NtMapViewOfSection (ssh_entry->hdl, NtCurrentProcess (), &ptr, 0,
			       ssh_entry->size, NULL, &viewsize, ViewShare,
			       MEM_TOP_DOWN, access);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      delete sph_entry;
      --ssh_entry->ref_count;
      return (void *) -1;
    }
  /* Use returned ptr address as is, so it's stored using the exact value
     in cygserver. */
  client_request_shm request (shmid, ptr, shmflg & ~SHM_RND);
  if (request.make_request () == -1 || request.ptrval () == NULL)
    {
      syscall_printf ("-1 [%d] = shmat ()", request.error_code ());
      UnmapViewOfFile (ptr);
      delete sph_entry;
      set_errno (request.error_code ());
      --ssh_entry->ref_count;
      return (void *) -1;
    }
  sph_entry->ptr = ptr;
  sph_entry->parent = ssh_entry;
  sph_entry->access = access;
  SLIST_LOCK ();
  SLIST_INSERT_HEAD (&sph_list, sph_entry, sph_next);
  SLIST_UNLOCK ();
  return ptr;
}

extern "C" int
shmctl (int shmid, int cmd, struct shmid_ds *buf)
{
  syscall_printf ("shmctl (shmid = %d, cmd = %d, buf = %p)",
		  shmid, cmd, buf);
  __try
    {
      client_request_shm request (shmid, cmd, buf);
      if (request.make_request () == -1 || request.retval () == -1)
	{
	  syscall_printf ("-1 [%d] = shmctl ()", request.error_code ());
	  set_errno (request.error_code ());
	  __leave;
	}
      if (cmd == IPC_RMID)
	{
	  /* Cleanup */
	  shm_shmid_list *ssh_entry, *ssh_next_entry;
	  SLIST_LOCK ();
	  SLIST_FOREACH_SAFE (ssh_entry, &ssh_list, ssh_next, ssh_next_entry)
	    {
	      if (ssh_entry->shmid == shmid)
		{
		  /* Remove this entry from the list and close the handle
		     only if it's not in use anymore. */
		  if (ssh_entry->ref_count <= 0)
		    {
		      SLIST_REMOVE (&ssh_list, ssh_entry, shm_shmid_list,
				    ssh_next);
		      CloseHandle (ssh_entry->hdl);
		      delete ssh_entry;
		    }
		  break;
		}
	    }
	  SLIST_UNLOCK ();
	}
      return request.retval ();
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

extern "C" int
shmdt (const void *shmaddr)
{
  syscall_printf ("shmdt (shmaddr = %p)", shmaddr);
  client_request_shm request (shmaddr);
  if (request.make_request () == -1 || request.retval () == -1)
    {
      syscall_printf ("-1 [%d] = shmdt ()", request.error_code ());
      set_errno (request.error_code ());
      return -1;
    }
  shm_attached_list *sph_entry, *sph_next_entry;
  /* Remove map from list... */
  SLIST_LOCK ();
  SLIST_FOREACH_SAFE (sph_entry, &sph_list, sph_next, sph_next_entry)
    {
      if (sph_entry->ptr == shmaddr)
	{
	  SLIST_REMOVE (&sph_list, sph_entry, shm_attached_list, sph_next);
	  /* ...unmap view... */
	  UnmapViewOfFile (sph_entry->ptr);
	  /* ...and, if this was the last reference to this shared section... */
	  shm_shmid_list *ssh_entry = sph_entry->parent;
	  if (--ssh_entry->ref_count <= 0)
	    {
	      /* ...delete parent entry and close handle. */
	      SLIST_REMOVE (&ssh_list, ssh_entry, shm_shmid_list, ssh_next);
	      CloseHandle (ssh_entry->hdl);
	      delete ssh_entry;
	    }
	  delete sph_entry;
	  break;
	}
    }
  SLIST_UNLOCK ();
  return request.retval ();
}

extern "C" int
shmget (key_t key, size_t size, int shmflg)
{
  syscall_printf ("shmget (key = %U, size = %d, shmflg = %y)",
		  key, size, shmflg);
  /* Try allocating memory before calling cygserver. */
  shm_shmid_list *ssh_new_entry = new (shm_shmid_list);
  if (!ssh_new_entry)
    {
      set_errno (ENOMEM);
      return -1;
    }
  client_request_shm request (key, size, shmflg);
  if (request.make_request () == -1 || request.retval () == -1)
    {
      syscall_printf ("-1 [%d] = shmget ()", request.error_code ());
      delete ssh_new_entry;
      set_errno (request.error_code ());
      return -1;
    }
  int shmid = request.retval ();	/* Shared mem ID */
  vm_object_t hdl = request.objval ();	/* HANDLE associated with it. */
  shm_shmid_list *ssh_entry;
  SLIST_LOCK ();
  SLIST_FOREACH (ssh_entry, &ssh_list, ssh_next)
    {
      if (ssh_entry->shmid == shmid)
	{
	  /* We already maintain an entry for this shmid.  That means,
	     the hdl returned by cygserver is a superfluous duplicate
	     of the original hdl maintained by cygserver.  We can safely
	     delete it. */
	  CloseHandle (hdl);
	  delete ssh_new_entry;
	  SLIST_UNLOCK ();
	  return shmid;
	}
    }
  /* We arrive here only if shmid is a new one for this process.  Add the
     shmid and hdl value to the list. */
  ssh_new_entry->shmid = shmid;
  ssh_new_entry->hdl = hdl;
  /* Fetch segment size from server.  If this is an already existing segment,
     the size value in this shmget call is supposed to be meaningless. */
  struct shmid_ds stat;
  client_request_shm stat_req (shmid, IPC_STAT, &stat);
  if (stat_req.make_request () == -1 || stat_req.retval () == -1)
    ssh_new_entry->size = size;
  else
    ssh_new_entry->size = stat.shm_segsz;
  ssh_new_entry->ref_count = 0;
  SLIST_INSERT_HEAD (&ssh_list, ssh_new_entry, ssh_next);
  SLIST_UNLOCK ();
  return shmid;
}
