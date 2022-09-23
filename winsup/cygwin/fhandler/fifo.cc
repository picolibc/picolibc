/* fhandler_fifo.cc - See fhandler.h for a description of the fhandler classes.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <w32api/winioctl.h>
#include "miscfuncs.h"

#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "cygtls.h"
#include "shared_info.h"
#include "ntdll.h"
#include "cygwait.h"
#include <sys/param.h>

/*
   Overview:

     FIFOs are implemented via Windows named pipes.  The server end of
     the pipe corresponds to an fhandler_fifo open for reading (a.k.a,
     a "reader"), and the client end corresponds to an fhandler_fifo
     open for writing (a.k.a, a "writer").

     The server can have multiple instances.  The reader (assuming for
     the moment that there is only one) creates a pipe instance for
     each writer that opens.  The reader maintains a list of
     "fifo_client_handler" structures, one for each writer.  A
     fifo_client_handler contains the handle for the pipe server
     instance and information about the state of the connection with
     the writer.  Access to the list is controlled by a
     "fifo_client_lock".

     The reader runs a "fifo_reader_thread" that creates new pipe
     instances as needed and listens for client connections.

     The connection state of a fifo_client_handler has one of the
     following values, in which order is important:

       fc_unknown
       fc_error
       fc_disconnected
       fc_closing
       fc_listening
       fc_connected
       fc_input_avail

     It can be changed in the following places:

       - It is set to fc_listening when the pipe instance is created.

       - It is set to fc_connected when the fifo_reader_thread detects
         a connection.

       - It is set to a value reported by the O/S when
         query_and_set_state is called.  This can happen in
         select.cc:peek_fifo and a couple other places.

       - It is set to fc_disconnected by raw_read when an attempt to
         read yields STATUS_PIPE_BROKEN.

       - It is set to fc_error in various places when unexpected
         things happen.

     State changes are always guarded by fifo_client_lock.

     If there are multiple readers open, only one of them, called the
     "owner", maintains the fifo_client_handler list.  The owner is
     therefore the only reader that can read at any given time.  If a
     different reader wants to read, it has to take ownership and
     duplicate the fifo_client_handler list.

     A reader that is not an owner also runs a fifo_reader_thread,
     which is mostly idle.  The thread wakes up if that reader might
     need to take ownership.

     There is a block of named shared memory, accessible to all
     fhandlers for a given FIFO.  It keeps track of the number of open
     readers and writers; it contains information needed for the owner
     change process; and it contains some locks to prevent races and
     deadlocks between the various threads.

     The shared memory is created by the first reader to open the
     FIFO.  It is opened by subsequent readers and by all writers.  It
     is destroyed by Windows when the last handle to it is closed.

     If a handle to it somehow remains open after all processes
     holding file descriptors to the FIFO have closed, the shared
     memory can persist and be reused with stale data by the next
     process that opens the FIFO.  So far I've seen this happen only
     as a result of a bug in the code, but there are some debug_printf
     statements in fhandler_fifo::open to help detect this if it
     happens again.

     At this writing, I know of only one application (Midnight
     Commander when running under tcsh) that *explicitly* opens two
     readers of a FIFO.  But many applications will have multiple
     readers open via dup/fork/exec.
*/


/* This is only to be used for writers.  When reading,
STATUS_PIPE_EMPTY simply means there's no data to be read. */
#define STATUS_PIPE_IS_CLOSED(status)	\
		({ NTSTATUS _s = (status); \
		   _s == STATUS_PIPE_CLOSING \
		   || _s == STATUS_PIPE_BROKEN \
		   || _s == STATUS_PIPE_EMPTY; })

#define STATUS_PIPE_NO_INSTANCE_AVAILABLE(status)	\
		({ NTSTATUS _s = (status); \
		   _s == STATUS_INSTANCE_NOT_AVAILABLE \
		   || _s == STATUS_PIPE_NOT_AVAILABLE \
		   || _s == STATUS_PIPE_BUSY; })

/* Number of pages reserved for shared_fc_handler. */
#define SH_FC_HANDLER_PAGES 100

static NO_COPY fifo_reader_id_t null_fr_id = { .winpid = 0, .fh = NULL };

fhandler_fifo::fhandler_fifo ():
  fhandler_pipe_fifo (),
  read_ready (NULL), write_ready (NULL), writer_opening (NULL),
  owner_needed_evt (NULL), owner_found_evt (NULL), update_needed_evt (NULL),
  cancel_evt (NULL), thr_sync_evt (NULL), pipe_name_buf (NULL),
  fc_handler (NULL), shandlers (0), nhandlers (0),
  reader (false), writer (false), duplexer (false),
  me (null_fr_id), shmem_handle (NULL), shmem (NULL),
  shared_fc_hdl (NULL), shared_fc_handler (NULL)
{
  need_fork_fixup (true);
}

PUNICODE_STRING
fhandler_fifo::get_pipe_name ()
{
  if (!pipe_name_buf)
    {
      pipe_name.Length = CYGWIN_FIFO_PIPE_NAME_LEN * sizeof (WCHAR);
      pipe_name.MaximumLength = pipe_name.Length + sizeof (WCHAR);
      pipe_name_buf = (PWCHAR) cmalloc_abort (HEAP_STR,
					      pipe_name.MaximumLength);
      pipe_name.Buffer = pipe_name_buf;
      __small_swprintf (pipe_name_buf, L"%S-fifo.%08x.%016X",
			&cygheap->installation_key, get_dev (), get_ino ());
    }
  return &pipe_name;
}

inline PSECURITY_ATTRIBUTES
sec_user_cloexec (bool cloexec, PSECURITY_ATTRIBUTES sa, PSID sid)
{
  return cloexec ? sec_user_nih (sa, sid) : sec_user (sa, sid);
}

static HANDLE
create_event ()
{
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  HANDLE evt = NULL;

  InitializeObjectAttributes (&attr, NULL, 0, NULL, NULL);
  status = NtCreateEvent (&evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return evt;
}


static void
set_pipe_non_blocking (HANDLE ph, bool nonblocking)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  FILE_PIPE_INFORMATION fpi;

  fpi.ReadMode = FILE_PIPE_MESSAGE_MODE;
  fpi.CompletionMode = nonblocking ? FILE_PIPE_COMPLETE_OPERATION
    : FILE_PIPE_QUEUE_OPERATION;
  status = NtSetInformationFile (ph, &io, &fpi, sizeof fpi,
				 FilePipeInformation);
  if (!NT_SUCCESS (status))
    debug_printf ("NtSetInformationFile(FilePipeInformation): %y", status);
}

/* Called when a FIFO is first opened for reading and again each time
   a new client handler is needed.  Each pipe instance is created in
   blocking mode so that we can easily wait for a connection.  After
   it is connected, it is put in nonblocking mode. */
HANDLE
fhandler_fifo::create_pipe_instance ()
{
  NTSTATUS status;
  HANDLE npfsh;
  HANDLE ph = NULL;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG hattr;
  ULONG sharing;
  ULONG nonblocking = FILE_PIPE_QUEUE_OPERATION;
  ULONG max_instances = -1;
  LARGE_INTEGER timeout;

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  access = GENERIC_READ | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES
    | SYNCHRONIZE;
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  hattr = (openflags & O_CLOEXEC ? 0 : OBJ_INHERIT) | OBJ_CASE_INSENSITIVE;
  InitializeObjectAttributes (&attr, get_pipe_name (),
			      hattr, npfsh, NULL);
  timeout.QuadPart = -500000;
  status = NtCreateNamedPipeFile (&ph, access, &attr, &io, sharing,
				  FILE_OPEN_IF, 0,
				  FILE_PIPE_MESSAGE_TYPE
				    | FILE_PIPE_REJECT_REMOTE_CLIENTS,
				  FILE_PIPE_MESSAGE_MODE,
				  nonblocking, max_instances,
				  DEFAULT_PIPEBUFSIZE, DEFAULT_PIPEBUFSIZE,
				  &timeout);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return ph;
}

/* Connect to a pipe instance. */
NTSTATUS
fhandler_fifo::open_pipe (HANDLE& ph)
{
  NTSTATUS status;
  HANDLE npfsh;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG sharing;

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    return status;
  access = GENERIC_WRITE | FILE_READ_ATTRIBUTES | SYNCHRONIZE;
  InitializeObjectAttributes (&attr, get_pipe_name (),
			      openflags & O_CLOEXEC ? 0 : OBJ_INHERIT,
			      npfsh, NULL);
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  return NtOpenFile (&ph, access, &attr, &io, sharing, 0);
}

/* Wait up to 100ms for a pipe instance to be available, then connect. */
NTSTATUS
fhandler_fifo::wait_open_pipe (HANDLE& ph)
{
  HANDLE npfsh;
  HANDLE evt;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  ULONG pwbuf_size;
  PFILE_PIPE_WAIT_FOR_BUFFER pwbuf;
  LONGLONG stamp;
  LONGLONG orig_timeout = -100 * NS100PERSEC / MSPERSEC;   /* 100ms */

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    return status;
  if (!(evt = create_event ()))
    api_fatal ("Can't create event, %E");
  pwbuf_size
    = offsetof (FILE_PIPE_WAIT_FOR_BUFFER, Name) + get_pipe_name ()->Length;
  pwbuf = (PFILE_PIPE_WAIT_FOR_BUFFER) alloca (pwbuf_size);
  pwbuf->Timeout.QuadPart = orig_timeout;
  pwbuf->NameLength = get_pipe_name ()->Length;
  pwbuf->TimeoutSpecified = TRUE;
  memcpy (pwbuf->Name, get_pipe_name ()->Buffer, get_pipe_name ()->Length);
  stamp = get_clock (CLOCK_MONOTONIC)->n100secs ();
  bool retry;
  do
    {
      retry = false;
      status = NtFsControlFile (npfsh, evt, NULL, NULL, &io, FSCTL_PIPE_WAIT,
				pwbuf, pwbuf_size, NULL, 0);
      if (status == STATUS_PENDING)
	{
	  if (WaitForSingleObject (evt, INFINITE) == WAIT_OBJECT_0)
	    status = io.Status;
	  else
	    api_fatal ("WFSO failed, %E");
	}
      if (NT_SUCCESS (status))
	status = open_pipe (ph);
      if (STATUS_PIPE_NO_INSTANCE_AVAILABLE (status))
	{
	  /* Another writer has grabbed the pipe instance.  Adjust
	     the timeout and keep waiting if there's time left. */
	  pwbuf->Timeout.QuadPart = orig_timeout
	    + get_clock (CLOCK_MONOTONIC)->n100secs () - stamp;
	  if (pwbuf->Timeout.QuadPart < 0)
	    retry = true;
	  else
	    status = STATUS_IO_TIMEOUT;
	}
    }
  while (retry);
  NtClose (evt);
  return status;
}

/* Always called with fifo_client_lock in place. */
int
fhandler_fifo::add_client_handler (bool new_pipe_instance)
{
  fifo_client_handler fc;

  if (nhandlers >= shandlers)
    {
      void *temp = realloc (fc_handler,
			    (shandlers += 64) * sizeof (fc_handler[0]));
      if (!temp)
	{
	  shandlers -= 64;
	  set_errno (ENOMEM);
	  return -1;
	}
      fc_handler = (fifo_client_handler *) temp;
    }
  if (new_pipe_instance)
    {
      HANDLE ph = create_pipe_instance ();
      if (!ph)
	return -1;
      fc.h = ph;
      fc.set_state (fc_listening);
    }
  fc_handler[nhandlers++] = fc;
  return 0;
}

/* Always called with fifo_client_lock in place.  Delete a
   client_handler by swapping it with the last one in the list. */
void
fhandler_fifo::delete_client_handler (int i)
{
  fc_handler[i].close ();
  if (i < --nhandlers)
    fc_handler[i] = fc_handler[nhandlers];
}

/* Delete handlers that we will never read from.  Always called with
   fifo_client_lock in place. */
void
fhandler_fifo::cleanup_handlers ()
{
  /* Work from the top down to try to avoid copying. */
  for (int i = nhandlers - 1; i >= 0; --i)
    if (fc_handler[i].get_state () < fc_connected)
      delete_client_handler (i);
}

/* Always called with fifo_client_lock in place. */
void
fhandler_fifo::record_connection (fifo_client_handler& fc, bool set,
				  fifo_client_connect_state s)
{
  if (set)
    fc.set_state (s);
  set_pipe_non_blocking (fc.h, true);
}

/* Called from fifo_reader_thread_func with owner_lock in place. */
int
fhandler_fifo::update_my_handlers ()
{
  int ret = 0;

  close_all_handlers ();
  fifo_reader_id_t prev = get_prev_owner ();
  if (!prev)
    {
      debug_printf ("No previous owner to copy handles from");
      return 0;
    }
  HANDLE prev_proc;
  if (prev.winpid == me.winpid)
    prev_proc =  GetCurrentProcess ();
  else
    prev_proc = OpenProcess (PROCESS_DUP_HANDLE, false, prev.winpid);
  if (!prev_proc)
    api_fatal ("Can't open process of previous owner, %E");

  fifo_client_lock ();
  for (int i = 0; i < get_shared_nhandlers (); i++)
    {
      if (add_client_handler (false) < 0)
	api_fatal ("Can't add client handler, %E");
      fifo_client_handler &fc = fc_handler[nhandlers - 1];
      if (!DuplicateHandle (prev_proc, shared_fc_handler[i].h,
			    GetCurrentProcess (), &fc.h, 0,
			    !close_on_exec (), DUPLICATE_SAME_ACCESS))
	{
	  debug_printf ("Can't duplicate handle of previous owner, %E");
	  __seterrno ();
	  fc.set_state (fc_error);
	  fc.last_read = false;
	  ret = -1;
	}
      else
	{
	  fc.set_state (shared_fc_handler[i].get_state ());
	  fc.last_read = shared_fc_handler[i].last_read;
	}
    }
  fifo_client_unlock ();
  NtClose (prev_proc);
  set_prev_owner (null_fr_id);
  return ret;
}

/* Always called with fifo_client_lock and owner_lock in place. */
int
fhandler_fifo::update_shared_handlers ()
{
  cleanup_handlers ();
  if (nhandlers > get_shared_shandlers ())
    {
      if (remap_shared_fc_handler (nhandlers * sizeof (fc_handler[0])) < 0)
	return -1;
    }
  set_shared_nhandlers (nhandlers);
  memcpy (shared_fc_handler, fc_handler, nhandlers * sizeof (fc_handler[0]));
  shared_fc_handler_updated (true);
  set_prev_owner (me);
  return 0;
}

static DWORD
fifo_reader_thread (LPVOID param)
{
  fhandler_fifo *fh = (fhandler_fifo *) param;
  return fh->fifo_reader_thread_func ();
}

DWORD
fhandler_fifo::fifo_reader_thread_func ()
{
  HANDLE conn_evt;

  if (!(conn_evt = CreateEvent (NULL, false, false, NULL)))
    api_fatal ("Can't create connection event, %E");

  while (1)
    {
      fifo_reader_id_t cur_owner, pending_owner;
      bool idle = false, take_ownership = false;

      owner_lock ();
      cur_owner = get_owner ();
      pending_owner = get_pending_owner ();

      if (pending_owner)
	{
	  if (pending_owner == me)
	    take_ownership = true;
	  else if (cur_owner != me)
	    idle = true;
	  else
	    {
	      /* I'm the owner but someone else wants to be.  Have I
		 already seen and reacted to update_needed_evt? */
	      if (WaitForSingleObject (update_needed_evt, 0) == WAIT_OBJECT_0)
		{
		  /* No, I haven't. */
		  fifo_client_lock ();
		  if (update_shared_handlers () < 0)
		    api_fatal ("Can't update shared handlers, %E");
		  fifo_client_unlock ();
		}
	      owner_unlock ();
	      /* Yield to pending owner. */
	      Sleep (1);
	      continue;
	    }
	}
      else if (!cur_owner)
	take_ownership = true;
      else if (cur_owner != me)
	idle = true;
      else
	/* I'm the owner and there's no pending owner. */
	goto owner_listen;
      if (idle)
	{
	  owner_unlock ();
	  HANDLE w[2] = { owner_needed_evt, cancel_evt };
	  switch (WaitForMultipleObjects (2, w, false, INFINITE))
	    {
	    case WAIT_OBJECT_0:
	      continue;
	    case WAIT_OBJECT_0 + 1:
	      goto canceled;
	    default:
	      api_fatal ("WFMO failed, %E");
	    }
	}
      else if (take_ownership)
	{
	  if (!shared_fc_handler_updated ())
	    {
	      owner_unlock ();
	      if (IsEventSignalled (cancel_evt))
		goto canceled;
	      continue;
	    }
	  else
	    {
	      set_owner (me);
	      set_pending_owner (null_fr_id);
	      if (update_my_handlers () < 0)
		debug_printf ("error updating my handlers, %E");
	      owner_found ();
	      /* Fall through to owner_listen. */
	    }
	}

owner_listen:
      fifo_client_lock ();
      cleanup_handlers ();
      if (add_client_handler () < 0)
	api_fatal ("Can't add a client handler, %E");

      /* Listen for a writer to connect to the new client handler. */
      fifo_client_handler& fc = fc_handler[nhandlers - 1];
      fifo_client_unlock ();
      shared_fc_handler_updated (false);
      owner_unlock ();
      NTSTATUS status;
      IO_STATUS_BLOCK io;
      bool cancel = false;
      bool update = false;

      status = NtFsControlFile (fc.h, conn_evt, NULL, NULL, &io,
				FSCTL_PIPE_LISTEN, NULL, 0, NULL, 0);
      if (status == STATUS_PENDING)
	{
	  HANDLE w[3] = { conn_evt, update_needed_evt, cancel_evt };
	  switch (WaitForMultipleObjects (3, w, false, INFINITE))
	    {
	    case WAIT_OBJECT_0:
	      status = io.Status;
	      debug_printf ("NtFsControlFile STATUS_PENDING, then %y",
			    status);
	      break;
	    case WAIT_OBJECT_0 + 1:
	      status = STATUS_WAIT_1;
	      update = true;
	      break;
	    case WAIT_OBJECT_0 + 2:
	      status = STATUS_THREAD_IS_TERMINATING;
	      cancel = true;
	      update = true;
	      break;
	    default:
	      api_fatal ("WFMO failed, %E");
	    }
	}
      else
	debug_printf ("NtFsControlFile status %y, no STATUS_PENDING",
		      status);
      HANDLE ph = NULL;
      NTSTATUS status1;

      fifo_client_lock ();
      if (fc.get_state () != fc_listening)
	/* select.cc:peek_fifo has already recorded a connection. */
	;
      else
	{
	  switch (status)
	    {
	    case STATUS_SUCCESS:
	    case STATUS_PIPE_CONNECTED:
	      record_connection (fc);
	      break;
	    case STATUS_PIPE_CLOSING:
	      debug_printf ("NtFsControlFile got STATUS_PIPE_CLOSING...");
	      /* Maybe a writer already connected, wrote, and closed.
		 Just query the O/S. */
	      fc.query_and_set_state ();
	      debug_printf ("...O/S reports state %d", fc.get_state ());
	      record_connection (fc, false);
	      break;
	    case STATUS_THREAD_IS_TERMINATING:
	    case STATUS_WAIT_1:
	      /* Try to connect a bogus client.  Otherwise fc is still
		 listening, and the next connection might not get recorded. */
	      status1 = open_pipe (ph);
	      WaitForSingleObject (conn_evt, INFINITE);
	      if (NT_SUCCESS (status1))
		/* Bogus cilent connected. */
		delete_client_handler (nhandlers - 1);
	      else
		/* Did a real client connect? */
		switch (io.Status)
		  {
		  case STATUS_SUCCESS:
		  case STATUS_PIPE_CONNECTED:
		    record_connection (fc);
		    break;
		  case STATUS_PIPE_CLOSING:
		    debug_printf ("got STATUS_PIPE_CLOSING when trying to connect bogus client...");
		    fc.query_and_set_state ();
		    debug_printf ("...O/S reports state %d", fc.get_state ());
		    record_connection (fc, false);
		    break;
		  default:
		    debug_printf ("NtFsControlFile status %y after failing to connect bogus client or real client", io.Status);
		    fc.set_state (fc_error);
		    break;
		  }
	      break;
	    default:
	      debug_printf ("NtFsControlFile got unexpected status %y", status);
	      fc.set_state (fc_error);
	      break;
	    }
	}
      if (ph)
	NtClose (ph);
      if (update)
	{
	  owner_lock ();
	  if (get_owner () == me && update_shared_handlers () < 0)
	    api_fatal ("Can't update shared handlers, %E");
	  owner_unlock ();
	}
      fifo_client_unlock ();
      if (cancel)
	goto canceled;
    }
canceled:
  if (conn_evt)
    NtClose (conn_evt);
  /* automatically return the cygthread to the cygthread pool */
  _my_tls._ctinfo->auto_release ();
  return 0;
}

/* Return -1 on error and 0 or 1 on success.  If ONLY_OPEN is true, we
   expect the shared memory to exist, and we only try to open it.  In
   this case, we return 0 on success.

   Otherwise, we create the shared memory if it doesn't exist, and we
   return 1 if it already existed and we successfully open it. */
int
fhandler_fifo::create_shmem (bool only_open)
{
  HANDLE sect;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  LARGE_INTEGER size = { .QuadPart = sizeof (fifo_shmem_t) };
  SIZE_T viewsize = sizeof (fifo_shmem_t);
  PVOID addr = NULL;
  UNICODE_STRING uname;
  WCHAR shmem_name[MAX_PATH];
  bool already_exists = false;

  __small_swprintf (shmem_name, L"fifo-shmem.%08x.%016X", get_dev (),
		    get_ino ());
  RtlInitUnicodeString (&uname, shmem_name);
  InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT,
			      get_shared_parent_dir (), NULL);
  if (!only_open)
    {
      status = NtCreateSection (&sect, STANDARD_RIGHTS_REQUIRED | SECTION_QUERY
				| SECTION_MAP_READ | SECTION_MAP_WRITE,
				&attr, &size, PAGE_READWRITE, SEC_COMMIT, NULL);
      if (status == STATUS_OBJECT_NAME_COLLISION)
	already_exists = true;
    }
  if (only_open || already_exists)
    status = NtOpenSection (&sect, STANDARD_RIGHTS_REQUIRED | SECTION_QUERY
			    | SECTION_MAP_READ | SECTION_MAP_WRITE, &attr);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  status = NtMapViewOfSection (sect, NtCurrentProcess (), &addr, 0, viewsize,
			       NULL, &viewsize, ViewShare, 0, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    {
      NtClose (sect);
      __seterrno_from_nt_status (status);
      return -1;
    }
  shmem_handle = sect;
  shmem = (fifo_shmem_t *) addr;
  return already_exists ? 1 : 0;
}

/* shmem_handle must be valid when this is called. */
int
fhandler_fifo::reopen_shmem ()
{
  NTSTATUS status;
  SIZE_T viewsize = sizeof (fifo_shmem_t);
  PVOID addr = NULL;

  status = NtMapViewOfSection (shmem_handle, NtCurrentProcess (), &addr,
			       0, viewsize, NULL, &viewsize, ViewShare,
			       0, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  shmem = (fifo_shmem_t *) addr;
  return 0;
}

/* On first creation, map and commit one page of memory. */
int
fhandler_fifo::create_shared_fc_handler ()
{
  HANDLE sect;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  LARGE_INTEGER size
    = { .QuadPart = (LONGLONG) (SH_FC_HANDLER_PAGES * wincap.page_size ()) };
  SIZE_T viewsize = get_shared_fc_handler_committed () ?: wincap.page_size ();
  PVOID addr = NULL;
  UNICODE_STRING uname;
  WCHAR shared_fc_name[MAX_PATH];

  __small_swprintf (shared_fc_name, L"fifo-shared-fc.%08x.%016X", get_dev (),
		    get_ino ());
  RtlInitUnicodeString (&uname, shared_fc_name);
  InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT,
			      get_shared_parent_dir (), NULL);
  status = NtCreateSection (&sect, STANDARD_RIGHTS_REQUIRED | SECTION_QUERY
			    | SECTION_MAP_READ | SECTION_MAP_WRITE, &attr,
			    &size, PAGE_READWRITE, SEC_RESERVE, NULL);
  if (status == STATUS_OBJECT_NAME_COLLISION)
    status = NtOpenSection (&sect, STANDARD_RIGHTS_REQUIRED | SECTION_QUERY
			    | SECTION_MAP_READ | SECTION_MAP_WRITE, &attr);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  status = NtMapViewOfSection (sect, NtCurrentProcess (), &addr, 0, viewsize,
			       NULL, &viewsize, ViewShare, 0, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    {
      NtClose (sect);
      __seterrno_from_nt_status (status);
      return -1;
    }
  shared_fc_hdl = sect;
  shared_fc_handler = (fifo_client_handler *) addr;
  if (!get_shared_fc_handler_committed ())
    set_shared_fc_handler_committed (viewsize);
  set_shared_shandlers (viewsize / sizeof (fifo_client_handler));
  return 0;
}

/* shared_fc_hdl must be valid when this is called. */
int
fhandler_fifo::reopen_shared_fc_handler ()
{
  NTSTATUS status;
  SIZE_T viewsize = get_shared_fc_handler_committed ();
  PVOID addr = NULL;

  status = NtMapViewOfSection (shared_fc_hdl, NtCurrentProcess (),
			       &addr, 0, viewsize, NULL, &viewsize,
			       ViewShare, 0, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  shared_fc_handler = (fifo_client_handler *) addr;
  return 0;
}

int
fhandler_fifo::remap_shared_fc_handler (size_t nbytes)
{
  NTSTATUS status;
  SIZE_T viewsize = roundup2 (nbytes, wincap.page_size ());
  PVOID addr = NULL;

  if (viewsize > SH_FC_HANDLER_PAGES * wincap.page_size ())
    {
      set_errno (ENOMEM);
      return -1;
    }

  NtUnmapViewOfSection (NtCurrentProcess (), shared_fc_handler);
  status = NtMapViewOfSection (shared_fc_hdl, NtCurrentProcess (),
			       &addr, 0, viewsize, NULL, &viewsize,
			       ViewShare, 0, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  shared_fc_handler = (fifo_client_handler *) addr;
  set_shared_fc_handler_committed (viewsize);
  set_shared_shandlers (viewsize / sizeof (fc_handler[0]));
  return 0;
}

int
fhandler_fifo::open (int flags, mode_t)
{
  int saved_errno = 0, shmem_res = 0;

  if (flags & O_PATH)
    return open_fs (flags);

  /* Determine what we're doing with this fhandler: reading, writing, both */
  switch (flags & O_ACCMODE)
    {
    case O_RDONLY:
      reader = true;
      break;
    case O_WRONLY:
      writer = true;
      break;
    case O_RDWR:
      reader = writer = duplexer = true;
      break;
    default:
      set_errno (EINVAL);
      goto err;
    }

  debug_only_printf ("reader %d, writer %d, duplexer %d", reader, writer, duplexer);
  set_flags (flags);
  if (reader && !duplexer)
    nohandle (true);

  /* Create control events for this named pipe */
  char char_sa_buf[1024];
  LPSECURITY_ATTRIBUTES sa_buf;
  sa_buf = sec_user_cloexec (flags & O_CLOEXEC, (PSECURITY_ATTRIBUTES) char_sa_buf,
		      cygheap->user.sid());

  char npbuf[MAX_PATH];
  __small_sprintf (npbuf, "r-event.%08x.%016X", get_dev (), get_ino ());
  if (!(read_ready = CreateEvent (sa_buf, true, false, npbuf)))
    {
      debug_printf ("CreateEvent for %s failed, %E", npbuf);
      __seterrno ();
      goto err;
    }
  npbuf[0] = 'w';
  if (!(write_ready = CreateEvent (sa_buf, true, false, npbuf)))
    {
      debug_printf ("CreateEvent for %s failed, %E", npbuf);
      __seterrno ();
      goto err_close_read_ready;
    }
  npbuf[0] = 'o';
  if (!(writer_opening = CreateEvent (sa_buf, true, false, npbuf)))
    {
      debug_printf ("CreateEvent for %s failed, %E", npbuf);
      __seterrno ();
      goto err_close_write_ready;
    }

  /* If we're reading, create the shared memory and the shared
     fc_handler memory, create some events, start the
     fifo_reader_thread, signal read_ready, and wait for a writer. */
  if (reader)
    {
      /* Create/open shared memory. */
      if ((shmem_res = create_shmem ()) < 0)
	goto err_close_writer_opening;
      else if (shmem_res == 0)
	debug_printf ("shmem created");
      else
	debug_printf ("shmem existed; ok if we're not the first reader");
      if (create_shared_fc_handler () < 0)
	goto err_close_shmem;
      npbuf[0] = 'n';
      if (!(owner_needed_evt = CreateEvent (sa_buf, true, false, npbuf)))
	{
	  debug_printf ("CreateEvent for %s failed, %E", npbuf);
	  __seterrno ();
	  goto err_close_shared_fc_handler;
	}
      npbuf[0] = 'f';
      if (!(owner_found_evt = CreateEvent (sa_buf, true, false, npbuf)))
	{
	  debug_printf ("CreateEvent for %s failed, %E", npbuf);
	  __seterrno ();
	  goto err_close_owner_needed_evt;
	}
      npbuf[0] = 'u';
      if (!(update_needed_evt = CreateEvent (sa_buf, false, false, npbuf)))
	{
	  debug_printf ("CreateEvent for %s failed, %E", npbuf);
	  __seterrno ();
	  goto err_close_owner_found_evt;
	}
      if (!(cancel_evt = create_event ()))
	goto err_close_update_needed_evt;
      if (!(thr_sync_evt = create_event ()))
	goto err_close_cancel_evt;

      me.winpid = GetCurrentProcessId ();
      me.fh = this;
      nreaders_lock ();
      if (inc_nreaders () == 1)
	{
	  /* Reinitialize _sh_fc_handler_updated, which starts as 0. */
	  shared_fc_handler_updated (true);
	  set_owner (me);
	}
      new cygthread (fifo_reader_thread, this, "fifo_reader", thr_sync_evt);
      SetEvent (read_ready);
      nreaders_unlock ();

      /* If we're a duplexer, we need a handle for writing. */
      if (duplexer)
	{
	  HANDLE ph = NULL;
	  NTSTATUS status;

	  nwriters_lock ();
	  inc_nwriters ();
	  SetEvent (write_ready);
	  nwriters_unlock ();

	  while (1)
	    {
	      status = open_pipe (ph);
	      if (NT_SUCCESS (status))
		{
		  set_handle (ph);
		  set_pipe_non_blocking (ph, flags & O_NONBLOCK);
		  break;
		}
	      else if (status == STATUS_OBJECT_NAME_NOT_FOUND)
		{
		  /* The pipe hasn't been created yet. */
		  yield ();
		  continue;
		}
	      else
		{
		  __seterrno_from_nt_status (status);
		  nohandle (true);
		  goto err_close_reader;
		}
	    }
	}
      /* Not a duplexer; wait for a writer to connect if we're blocking. */
      else if (!wait (write_ready))
	goto err_close_reader;
      goto success;
    }

  /* If we're writing, wait for read_ready, connect to the pipe, open
     the shared memory, and signal write_ready.  */
  if (writer)
    {
      NTSTATUS status;

      /* Don't let a reader see EOF at this point. */
      SetEvent (writer_opening);
      while (1)
	{
	  if (!wait (read_ready))
	    {
	      ResetEvent (writer_opening);
	      goto err_close_writer_opening;
	    }
	  status = open_pipe (get_handle ());
	  if (NT_SUCCESS (status))
	    goto writer_shmem;
	  else if (status == STATUS_OBJECT_NAME_NOT_FOUND)
	    {
	      /* The pipe hasn't been created yet or there's no longer
		 a reader open. */
	      yield ();
	      continue;
	    }
	  else if (STATUS_PIPE_NO_INSTANCE_AVAILABLE (status))
	    break;
	  else
	    {
	      debug_printf ("create of writer failed");
	      __seterrno_from_nt_status (status);
	      ResetEvent (writer_opening);
	      goto err_close_writer_opening;
	    }
	}

      /* We should get here only if the system is heavily loaded
	 and/or many writers are trying to connect simultaneously */
      while (1)
	{
	  if (!wait (read_ready))
	    {
	      ResetEvent (writer_opening);
	      goto err_close_writer_opening;
	    }
	  status = wait_open_pipe (get_handle ());
	  if (NT_SUCCESS (status))
	    goto writer_shmem;
	  else if (status == STATUS_IO_TIMEOUT)
	    continue;
	  else
	    {
	      debug_printf ("create of writer failed");
	      __seterrno_from_nt_status (status);
	      ResetEvent (writer_opening);
	      goto err_close_writer_opening;
	    }
	}
    }
writer_shmem:
  if (create_shmem (true) < 0)
    goto err_close_writer_opening;
/* writer_success: */
  set_pipe_non_blocking (get_handle (), flags & O_NONBLOCK);
  nwriters_lock ();
  inc_nwriters ();
  set_writer_opened ();
  SetEvent (write_ready);
  ResetEvent (writer_opening);
  nwriters_unlock ();
success:
  if (!select_sem)
    {
      __small_sprintf (npbuf, "semaphore.%08x.%016X", get_dev (), get_ino ());
      select_sem = CreateSemaphore (sa_buf, 0, INT32_MAX, npbuf);
    }
  return 1;
err_close_reader:
  saved_errno = get_errno ();
  close ();
  set_errno (saved_errno);
  return 0;
/* err_close_thr_sync_evt: */
/*   NtClose (thr_sync_evt); */
err_close_cancel_evt:
  NtClose (cancel_evt);
err_close_update_needed_evt:
  NtClose (update_needed_evt);
err_close_owner_found_evt:
  NtClose (owner_found_evt);
err_close_owner_needed_evt:
  NtClose (owner_needed_evt);
err_close_shared_fc_handler:
  NtUnmapViewOfSection (NtCurrentProcess (), shared_fc_handler);
  NtClose (shared_fc_hdl);
err_close_shmem:
  NtUnmapViewOfSection (NtCurrentProcess (), shmem);
  NtClose (shmem_handle);
err_close_writer_opening:
  NtClose (writer_opening);
err_close_write_ready:
  NtClose (write_ready);
err_close_read_ready:
  NtClose (read_ready);
err:
  if (get_handle ())
    NtClose (get_handle ());
  return 0;
}

off_t
fhandler_fifo::lseek (off_t offset, int whence)
{
  debug_printf ("(%D, %d)", offset, whence);
  set_errno (ESPIPE);
  return -1;
}

bool
fhandler_fifo::wait (HANDLE h)
{
#ifdef DEBUGGING
  const char *what;
  if (h == read_ready)
    what = "reader";
  else
    what = "writer";
#endif
  /* Set the wait to zero for non-blocking I/O-related events. */
  DWORD wait = ((h == read_ready || h == write_ready)
		&& get_flags () & O_NONBLOCK) ? 0 : INFINITE;

  debug_only_printf ("waiting for %s", what);
  /* Wait for the event.  Set errno, as appropriate if something goes wrong. */
  switch (cygwait (h, wait))
    {
    case WAIT_OBJECT_0:
      debug_only_printf ("successfully waited for %s", what);
      return true;
    case WAIT_SIGNALED:
      debug_only_printf ("interrupted by signal while waiting for %s", what);
      set_errno (EINTR);
      return false;
    case WAIT_CANCELED:
      debug_only_printf ("cancellable interruption while waiting for %s", what);
      pthread::static_cancel_self ();	/* never returns */
      break;
    case WAIT_TIMEOUT:
      if (h == write_ready)
	{
	  debug_only_printf ("wait timed out waiting for write but will still open reader since non-blocking mode");
	  return true;
	}
      else
	{
	  set_errno (ENXIO);
	  return false;
	}
      break;
    default:
      debug_only_printf ("unknown error while waiting for %s", what);
      __seterrno ();
      return false;
   }
}

/* Called from raw_read and select.cc:peek_fifo. */
int
fhandler_fifo::take_ownership (DWORD timeout)
{
  int ret = 0;

  owner_lock ();
  if (get_owner () == me)
    {
      owner_unlock ();
      return 0;
    }
  set_pending_owner (me);
  /* Wake up my fifo_reader_thread. */
  owner_needed ();
  if (get_owner ())
    /* Wake up the owner and request an update of the shared fc_handlers. */
    SetEvent (update_needed_evt);
  owner_unlock ();
  /* The reader threads should now do the transfer. */
  switch (WaitForSingleObject (owner_found_evt, timeout))
    {
    case WAIT_OBJECT_0:
      owner_lock ();
      if (get_owner () != me)
	{
	  debug_printf ("owner_found_evt signaled, but I'm not the owner");
	  ret = -1;
	}
      owner_unlock ();
      break;
    case WAIT_TIMEOUT:
      debug_printf ("timed out");
      ret = -1;
      break;
    default:
      debug_printf ("WFSO failed, %E");
      ret = -1;
      break;
    }
  return ret;
}

void
fhandler_fifo::release_select_sem (const char *from)
{
  LONG n_release;
  if (reader) /* Number of select() call. */
    n_release = get_obj_handle_count (select_sem)
      - get_obj_handle_count (read_ready);
  else /* Number of select() and reader */
    n_release = get_obj_handle_count (select_sem)
      - get_obj_handle_count (get_handle ());
  debug_printf("%s(%s) release %d", from,
	       reader ? "reader" : "writer", n_release);
  if (n_release)
    ReleaseSemaphore (select_sem, n_release, NULL);
}

/* Read from a non-blocking pipe and wait for completion. */
static NTSTATUS
nt_read (HANDLE h, HANDLE evt, PIO_STATUS_BLOCK pio, void *in_ptr, size_t& len)
{
  NTSTATUS status;

  ResetEvent (evt);
  status = NtReadFile (h, evt, NULL, NULL, pio, in_ptr, len, NULL, NULL);
  if (status == STATUS_PENDING)
    {
      /* Very short-lived */
      status = NtWaitForSingleObject (evt, FALSE, NULL);
      if (NT_SUCCESS (status))
	status = pio->Status;
    }
  return status;
}

void
fhandler_fifo::raw_read (void *in_ptr, size_t& len)
{
  HANDLE evt;

  if (!len)
    return;

  if (!(evt = CreateEvent (NULL, false, false, NULL)))
    {
      __seterrno ();
      len = (size_t) -1;
      return;
    }

  while (1)
    {
      int nconnected = 0;

      /* No one else can take ownership while we hold the reading_lock. */
      reading_lock ();
      if (take_ownership (10) < 0)
	goto maybe_retry;

      fifo_client_lock ();
      /* Poll the connected clients for input.  Make three passes.

	 On the first pass, just try to read from the client from
	 which we last read successfully.  This should minimize
	 interleaving of writes from different clients.

	 On the second pass, just try to read from the clients in the
	 state fc_input_avail.  This should be more efficient if
	 select has been called and detected input available.

	 On the third pass, try to read from all connected clients. */

      /* First pass. */
      int j;
      for (j = 0; j < nhandlers; j++)
	if (fc_handler[j].last_read)
	  break;
      if (j < nhandlers && fc_handler[j].get_state () < fc_connected)
	{
	  fc_handler[j].last_read = false;
	  j = nhandlers;
	}
      if (j < nhandlers)
	{
	  NTSTATUS status;
	  IO_STATUS_BLOCK io;

	  status = nt_read (fc_handler[j].h, evt, &io, in_ptr, len);
	  switch (status)
	    {
	    case STATUS_SUCCESS:
	    case STATUS_BUFFER_OVERFLOW:
	      if (io.Information > 0)
		{
		  len = io.Information;
		  goto unlock_out;
		}
	      break;
	    case STATUS_PIPE_EMPTY:
	      /* Update state in case it's fc_input_avail. */
	      fc_handler[j].set_state (fc_connected);
	      break;
	    case STATUS_PIPE_BROKEN:
	      fc_handler[j].set_state (fc_disconnected);
	      break;
	    default:
	      debug_printf ("nt_read status %y", status);
	      fc_handler[j].set_state (fc_error);
	      break;
	    }
	}

      /* Second pass. */
      for (int i = 0; i < nhandlers; i++)
	if (fc_handler[i].get_state () == fc_input_avail)
	  {
	    NTSTATUS status;
	    IO_STATUS_BLOCK io;

	    status = nt_read (fc_handler[i].h, evt, &io, in_ptr, len);
	    switch (status)
	      {
	      case STATUS_SUCCESS:
	      case STATUS_BUFFER_OVERFLOW:
		if (io.Information > 0)
		  {
		    len = io.Information;
		    if (j < nhandlers)
		      fc_handler[j].last_read = false;
		    fc_handler[i].last_read = true;
		    goto unlock_out;
		  }
		break;
	      case STATUS_PIPE_EMPTY:
		/* No input available after all. */
		fc_handler[i].set_state (fc_connected);
		break;
	      case STATUS_PIPE_BROKEN:
		fc_handler[i].set_state (fc_disconnected);
		break;
	      default:
		debug_printf ("nt_read status %y", status);
		fc_handler[i].set_state (fc_error);
		break;
	      }
	  }

      /* Third pass. */
      for (int i = 0; i < nhandlers; i++)
	if (fc_handler[i].get_state () >= fc_connected)
	  {
	    NTSTATUS status;
	    IO_STATUS_BLOCK io;

	    nconnected++;
	    status = nt_read (fc_handler[i].h, evt, &io, in_ptr, len);
	    switch (status)
	      {
	      case STATUS_SUCCESS:
	      case STATUS_BUFFER_OVERFLOW:
		if (io.Information > 0)
		  {
		    len = io.Information;
		    if (j < nhandlers)
		      fc_handler[j].last_read = false;
		    fc_handler[i].last_read = true;
		    goto unlock_out;
		  }
		break;
	      case STATUS_PIPE_EMPTY:
		break;
	      case STATUS_PIPE_BROKEN:
		fc_handler[i].set_state (fc_disconnected);
		nconnected--;
		break;
	      default:
		debug_printf ("nt_read status %y", status);
		fc_handler[i].set_state (fc_error);
		nconnected--;
		break;
	      }
	  }
      if (!nconnected && hit_eof ())
	{
	  len = 0;
	  goto unlock_out;
	}
      fifo_client_unlock ();
maybe_retry:
      reading_unlock ();
      if (is_nonblocking ())
	{
	  set_errno (EAGAIN);
	  len = (size_t) -1;
	  goto out;
	}
      else
	{
	  /* Allow interruption and don't hog the CPU. */
	  DWORD waitret = cygwait (select_sem, 1, cw_cancel | cw_sig_eintr);
	  if (waitret == WAIT_CANCELED)
	    pthread::static_cancel_self ();
	  else if (waitret == WAIT_SIGNALED)
	    {
	      if (_my_tls.call_signal_handler ())
		continue;
	      else
		{
		  set_errno (EINTR);
		  len = (size_t) -1;
		  goto out;
		}
	    }
	}
      /* We might have been closed by a signal handler or another thread. */
      if (isclosed ())
	{
	  set_errno (EBADF);
	  len = (size_t) -1;
	  goto out;
	}
    }
unlock_out:
  fifo_client_unlock ();
  reading_unlock ();
out:
  if (select_sem)
    release_select_sem ("raw_read");
  CloseHandle (evt);
}

int
fhandler_fifo::fstat (struct stat *buf)
{
  if (reader || writer || duplexer)
    {
      /* fhandler_fifo::open has been called, and O_PATH is not set.
	 We don't want to call fhandler_base::fstat.  In the writer
	 and duplexer cases we have a handle, but it's a pipe handle
	 rather than a file handle, so it's not suitable for stat.  In
	 the reader case we don't have a handle, but
	 fhandler_base::fstat would call fhandler_base::open, which
	 would modify the flags and status_flags. */
      fhandler_disk_file fh (pc);
      fh.get_device () = FH_FS;
      int res = fh.fstat (buf);
      buf->st_dev = buf->st_rdev = dev ();
      buf->st_mode = dev ().mode ();
      buf->st_size = 0;
      return res;
    }
  return fhandler_base::fstat (buf);
}

int
fhandler_fifo::fstatvfs (struct statvfs *sfs)
{
  if (get_flags () & O_PATH)
    /* We already have a handle. */
    {
      HANDLE h = get_handle ();
      if (h)
	return fstatvfs_by_handle (h, sfs);
    }

  fhandler_disk_file fh (pc);
  fh.get_device () = FH_FS;
  return fh.fstatvfs (sfs);
}

void
fhandler_fifo::close_all_handlers ()
{
  fifo_client_lock ();
  for (int i = 0; i < nhandlers; i++)
    fc_handler[i].close ();
  nhandlers = 0;
  fifo_client_unlock ();
}

/* Return previous state. */
fifo_client_connect_state
fifo_client_handler::query_and_set_state ()
{
  IO_STATUS_BLOCK io;
  FILE_PIPE_LOCAL_INFORMATION fpli;
  NTSTATUS status;
  fifo_client_connect_state prev_state = get_state ();

  if (!h)
    {
      set_state (fc_unknown);
      goto out;
    }

  status = NtQueryInformationFile (h, &io, &fpli,
				   sizeof (fpli), FilePipeLocalInformation);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQueryInformationFile status %y", status);
      set_state (fc_error);
    }
  else if (fpli.ReadDataAvailable > 0)
    set_state (fc_input_avail);
  else
    switch (fpli.NamedPipeState)
      {
      case FILE_PIPE_DISCONNECTED_STATE:
	set_state (fc_disconnected);
	break;
      case FILE_PIPE_LISTENING_STATE:
	set_state (fc_listening);
	break;
      case FILE_PIPE_CONNECTED_STATE:
	set_state (fc_connected);
	break;
      case FILE_PIPE_CLOSING_STATE:
	set_state (fc_closing);
	break;
      default:
	set_state (fc_error);
	break;
      }
out:
  return prev_state;
}

void
fhandler_fifo::cancel_reader_thread ()
{
  if (cancel_evt)
    SetEvent (cancel_evt);
  if (thr_sync_evt)
    WaitForSingleObject (thr_sync_evt, INFINITE);
}

int
fhandler_fifo::close ()
{
  if (select_sem)
    {
      release_select_sem ("close");
      NtClose (select_sem);
    }
  if (writer)
    {
      nwriters_lock ();
      if (dec_nwriters () == 0)
	ResetEvent (write_ready);
      nwriters_unlock ();
    }
  if (reader)
    {
      /* If we're the owner, we can't close our fc_handlers if a new
	 owner might need to duplicate them. */
      bool close_fc_ok = false;

      cancel_reader_thread ();
      nreaders_lock ();
      if (dec_nreaders () == 0)
	{
	  close_fc_ok = true;
	  ResetEvent (read_ready);
	  ResetEvent (owner_needed_evt);
	  ResetEvent (owner_found_evt);
	  set_owner (null_fr_id);
	  set_prev_owner (null_fr_id);
	  set_pending_owner (null_fr_id);
	  set_shared_nhandlers (0);
	}
      else
	{
	  owner_lock ();
	  if (get_owner () != me)
	    close_fc_ok = true;
	  else
	    {
	      set_owner (null_fr_id);
	      set_prev_owner (me);
	      if (!get_pending_owner ())
		owner_needed ();
	    }
	  owner_unlock ();
	}
      nreaders_unlock ();
      while (!close_fc_ok)
	{
	  if (WaitForSingleObject (owner_found_evt, 1) == WAIT_OBJECT_0)
	    close_fc_ok = true;
	  else
	    {
	      nreaders_lock ();
	      if (!nreaders ())
		{
		  close_fc_ok = true;
		  nreaders_unlock ();
		}
	      else
		{
		  nreaders_unlock ();
		  owner_lock ();
		  if (get_owner () || get_prev_owner () != me)
		    close_fc_ok = true;
		  owner_unlock ();
		}
	    }
	}
      close_all_handlers ();
      if (fc_handler)
	free (fc_handler);
      if (owner_needed_evt)
	NtClose (owner_needed_evt);
      if (owner_found_evt)
	NtClose (owner_found_evt);
      if (update_needed_evt)
	NtClose (update_needed_evt);
      if (cancel_evt)
	NtClose (cancel_evt);
      if (thr_sync_evt)
	NtClose (thr_sync_evt);
      if (shared_fc_handler)
	NtUnmapViewOfSection (NtCurrentProcess (), shared_fc_handler);
      if (shared_fc_hdl)
	NtClose (shared_fc_hdl);
    }
  if (shmem)
    NtUnmapViewOfSection (NtCurrentProcess (), shmem);
  if (shmem_handle)
    NtClose (shmem_handle);
  if (read_ready)
    NtClose (read_ready);
  if (write_ready)
    NtClose (write_ready);
  if (writer_opening)
    NtClose (writer_opening);
  if (nohandle ())
    return 0;
  else
    return fhandler_base::close ();
}

/* If we have a write handle (i.e., we're a duplexer or a writer),
   keep the nonblocking state of the windows pipe in sync with our
   nonblocking state. */
int
fhandler_fifo::fcntl (int cmd, intptr_t arg)
{
  if (cmd != F_SETFL || nohandle () || (get_flags () & O_PATH))
    return fhandler_base::fcntl (cmd, arg);

  const bool was_nonblocking = is_nonblocking ();
  int res = fhandler_base::fcntl (cmd, arg);
  const bool now_nonblocking = is_nonblocking ();
  if (now_nonblocking != was_nonblocking)
    set_pipe_non_blocking (get_handle (), now_nonblocking);
  return res;
}

int
fhandler_fifo::dup (fhandler_base *child, int flags)
{
  fhandler_fifo *fhf = NULL;

  if (get_flags () & O_PATH)
    return fhandler_base::dup (child, flags);

  if (fhandler_base::dup (child, flags))
    goto err;

  fhf = (fhandler_fifo *) child;
  if (!DuplicateHandle (GetCurrentProcess (), read_ready,
			GetCurrentProcess (), &fhf->read_ready,
			0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      goto err;
    }
  if (!DuplicateHandle (GetCurrentProcess (), write_ready,
			GetCurrentProcess (), &fhf->write_ready,
			0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      goto err_close_read_ready;
    }
  if (!DuplicateHandle (GetCurrentProcess (), writer_opening,
			GetCurrentProcess (), &fhf->writer_opening,
			0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      goto err_close_write_ready;
    }
  if (!DuplicateHandle (GetCurrentProcess (), shmem_handle,
			GetCurrentProcess (), &fhf->shmem_handle,
			0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      goto err_close_writer_opening;
    }
  if (fhf->reopen_shmem () < 0)
    goto err_close_shmem_handle;
  if (select_sem &&
      !DuplicateHandle (GetCurrentProcess (), select_sem,
			GetCurrentProcess (), &fhf->select_sem,
			0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      goto err_close_shmem;
    }
  if (reader)
    {
      /* Make sure the child starts unlocked. */
      fhf->fifo_client_unlock ();

      /* Clear fc_handler list; the child never starts as owner. */
      fhf->nhandlers = fhf->shandlers = 0;
      fhf->fc_handler = NULL;

      if (!DuplicateHandle (GetCurrentProcess (), shared_fc_hdl,
			    GetCurrentProcess (), &fhf->shared_fc_hdl,
			    0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
	{
	  __seterrno ();
	  goto err_close_select_sem;
	}
      if (fhf->reopen_shared_fc_handler () < 0)
	goto err_close_shared_fc_hdl;
      if (!DuplicateHandle (GetCurrentProcess (), owner_needed_evt,
			    GetCurrentProcess (), &fhf->owner_needed_evt,
			    0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
	{
	  __seterrno ();
	  goto err_close_shared_fc_handler;
	}
      if (!DuplicateHandle (GetCurrentProcess (), owner_found_evt,
			    GetCurrentProcess (), &fhf->owner_found_evt,
			    0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
	{
	  __seterrno ();
	  goto err_close_owner_needed_evt;
	}
      if (!DuplicateHandle (GetCurrentProcess (), update_needed_evt,
			    GetCurrentProcess (), &fhf->update_needed_evt,
			    0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
	{
	  __seterrno ();
	  goto err_close_owner_found_evt;
	}
      if (!(fhf->cancel_evt = create_event ()))
	goto err_close_update_needed_evt;
      if (!(fhf->thr_sync_evt = create_event ()))
	goto err_close_cancel_evt;
      inc_nreaders ();
      fhf->me.fh = fhf;
      new cygthread (fifo_reader_thread, fhf, "fifo_reader", fhf->thr_sync_evt);
    }
  if (writer)
    inc_nwriters ();
  return 0;
err_close_cancel_evt:
  NtClose (fhf->cancel_evt);
err_close_update_needed_evt:
  NtClose (fhf->update_needed_evt);
err_close_owner_found_evt:
  NtClose (fhf->owner_found_evt);
err_close_owner_needed_evt:
  NtClose (fhf->owner_needed_evt);
err_close_shared_fc_handler:
  NtUnmapViewOfSection (GetCurrentProcess (), fhf->shared_fc_handler);
err_close_shared_fc_hdl:
  NtClose (fhf->shared_fc_hdl);
err_close_select_sem:
  NtClose (fhf->select_sem);
err_close_shmem:
  NtUnmapViewOfSection (GetCurrentProcess (), fhf->shmem);
err_close_shmem_handle:
  NtClose (fhf->shmem_handle);
err_close_writer_opening:
  NtClose (fhf->writer_opening);
err_close_write_ready:
  NtClose (fhf->write_ready);
err_close_read_ready:
  NtClose (fhf->read_ready);
err:
  return -1;
}

void
fhandler_fifo::fixup_after_fork (HANDLE parent)
{
  fhandler_base::fixup_after_fork (parent);
  fork_fixup (parent, read_ready, "read_ready");
  fork_fixup (parent, write_ready, "write_ready");
  fork_fixup (parent, writer_opening, "writer_opening");
  fork_fixup (parent, shmem_handle, "shmem_handle");
  if (reopen_shmem () < 0)
    api_fatal ("Can't reopen shared memory during fork, %E");
  if (select_sem)
    fork_fixup (parent, select_sem, "select_sem");
  if (reader)
    {
      /* Make sure the child starts unlocked. */
      fifo_client_unlock ();

      fork_fixup (parent, shared_fc_hdl, "shared_fc_hdl");
      if (reopen_shared_fc_handler () < 0)
	api_fatal ("Can't reopen shared fc_handler memory during fork, %E");
      fork_fixup (parent, owner_needed_evt, "owner_needed_evt");
      fork_fixup (parent, owner_found_evt, "owner_found_evt");
      fork_fixup (parent, update_needed_evt, "update_needed_evt");
      if (close_on_exec ())
	/* Prevent a later attempt to close the non-inherited
	   pipe-instance handles copied from the parent. */
	nhandlers = 0;
      if (!(cancel_evt = create_event ()))
	api_fatal ("Can't create reader thread cancel event during fork, %E");
      if (!(thr_sync_evt = create_event ()))
	api_fatal ("Can't create reader thread sync event during fork, %E");
      inc_nreaders ();
      me.winpid = GetCurrentProcessId ();
      new cygthread (fifo_reader_thread, this, "fifo_reader", thr_sync_evt);
    }
  if (writer)
    inc_nwriters ();
}

void
fhandler_fifo::fixup_after_exec ()
{
  fhandler_base::fixup_after_exec ();
  if (close_on_exec ())
    return;
  if (reopen_shmem () < 0)
    api_fatal ("Can't reopen shared memory during exec, %E");
  if (reader)
    {
      /* Make sure the child starts unlocked. */
      fifo_client_unlock ();

      if (reopen_shared_fc_handler () < 0)
	api_fatal ("Can't reopen shared fc_handler memory during exec, %E");
      fc_handler = NULL;
      nhandlers = shandlers = 0;
      if (!(cancel_evt = create_event ()))
	api_fatal ("Can't create reader thread cancel event during exec, %E");
      if (!(thr_sync_evt = create_event ()))
	api_fatal ("Can't create reader thread sync event during exec, %E");
      /* At this moment we're a new reader.  The count will be
	 decremented when the parent closes. */
      inc_nreaders ();
      me.winpid = GetCurrentProcessId ();
      new cygthread (fifo_reader_thread, this, "fifo_reader", thr_sync_evt);
    }
  if (writer)
    inc_nwriters ();
}

void
fhandler_fifo::set_close_on_exec (bool val)
{
  fhandler_base::set_close_on_exec (val);
  set_no_inheritance (read_ready, val);
  set_no_inheritance (write_ready, val);
  set_no_inheritance (writer_opening, val);
  if (reader)
    {
      set_no_inheritance (owner_needed_evt, val);
      set_no_inheritance (owner_found_evt, val);
      set_no_inheritance (update_needed_evt, val);
      fifo_client_lock ();
      for (int i = 0; i < nhandlers; i++)
	set_no_inheritance (fc_handler[i].h, val);
      fifo_client_unlock ();
    }
  if (select_sem)
    set_no_inheritance (select_sem, val);
}
