/* debug.cc

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#ifdef DEBUGGING
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#endif

#undef CloseHandle

#ifdef DEBUGGING
/* Here lies extra debugging routines which help track down internal
   Cygwin problems when compiled with -DDEBUGGING . */
#define NFREEH (sizeof (cygheap->debug.freeh) / sizeof (cygheap->debug.freeh[0]))

class lock_debug
{
  static NO_COPY SRWLOCK lock;
 public:
  lock_debug () { AcquireSRWLockExclusive (&lock); }
  ~lock_debug () { ReleaseSRWLockExclusive (&lock); }
};

SRWLOCK NO_COPY lock_debug::lock = SRWLOCK_INIT;

static bool mark_closed (const char *, int, HANDLE, const char *, bool);

/* Find a registered handle in the linked list of handles. */
static handle_list *
find_handle (HANDLE h)
{
  handle_list *hl;
  for (hl = &cygheap->debug.starth; hl->next != NULL; hl = hl->next)
    if (hl->next->h == h)
      goto out;
  hl = NULL;

out:
  return hl;
}

void
verify_handle (const char *func, int ln, HANDLE h)
{
  lock_debug here;
  handle_list *hl = find_handle (h);
  if (!hl)
    return;
  system_printf ("%s:%d - multiple attempts to add handle %p", func, ln, h);

  system_printf (" previously allocated by %s:%d(%s<%p>) winpid %d",
		 hl->func, hl->ln, hl->name, hl->h, hl->pid);
}

void
setclexec (HANDLE oh, HANDLE nh, bool not_inheriting)
{
  lock_debug here;
  handle_list *hl = find_handle (oh);
  if (hl)
    {
      hl = hl->next;
      hl->inherited = !not_inheriting;
      hl->h = nh;
    }
}

/* Create a new handle record */
static handle_list *
newh ()
{
  handle_list *hl;

  for (hl = cygheap->debug.freeh; hl < cygheap->debug.freeh + NFREEH; hl++)
    if (hl->name == NULL)
      return hl;

  return NULL;
}

void
modify_handle (const char *func, int ln, HANDLE h, const char *name, bool inh)
{
  lock_debug here;
  handle_list *hl = find_handle (h);
  if (!hl)
    {
      system_printf ("%s:%d handle %s<%p> not found", func, ln, name, h);
      return;
    }
  hl->next->inherited = inh;
  debug_printf ("%s:%d set handle %s<%p> inheritance flag to %d", func, ln,
		name, h, inh);
}

/* Add a handle to the linked list of known handles. */
void
add_handle (const char *func, int ln, HANDLE h, const char *name, bool inh)
{
  handle_list *hl;

  if (!cygheap)
    return;

  lock_debug here;
  if ((hl = find_handle (h)))
    {
      hl = hl->next;
      if (hl->name == name && hl->func == func && hl->ln == ln)
	return;
      system_printf ("%s:%d - multiple attempts to add handle %s<%p>", func,
		     ln, name, h);
      system_printf (" previously allocated by %s:%d(%s<%p>) winpid %d",
		     hl->func, hl->ln, hl->name, hl->h, hl->pid);
      return;
    }

  if ((hl = newh ()) == NULL)
    {
      debug_printf ("couldn't allocate memory for %s(%d): %s(%p)",
		    func, ln, name, h);
      return;
    }
  hl->h = h;
  hl->name = name;
  hl->func = func;
  hl->ln = ln;
  hl->inherited = inh;
  hl->pid = GetCurrentProcessId ();
  hl->next = cygheap->debug.starth.next;
  cygheap->debug.starth.next = hl;
  SetHandleInformation (h, HANDLE_FLAG_PROTECT_FROM_CLOSE, HANDLE_FLAG_PROTECT_FROM_CLOSE);
  debug_printf ("protecting handle '%s'(%p), inherited flag %d", hl->name, hl->h, hl->inherited);
}

static void
delete_handle (handle_list *hl)
{
  handle_list *hnuke = hl->next;
  debug_printf ("nuking handle '%s' (%p)", hnuke->name, hnuke->h);
  hl->next = hnuke->next;
  memset (hnuke, 0, sizeof (*hnuke));
}

void
debug_fixup_after_fork_exec ()
{
  /* No lock needed at this point */
  handle_list *hl;
  for (hl = &cygheap->debug.starth; hl->next != NULL; /* nothing */)
    if (hl->next->inherited)
      hl = hl->next;
    else
      delete_handle (hl);	// removes hl->next
}

static bool
mark_closed (const char *func, int ln, HANDLE h, const char *name, bool force)
{
  handle_list *hl;

  if (!cygheap)
    return true;

  if ((hl = find_handle (h)) && !force)
    {
      hl = hl->next;
      system_printf ("attempt to close protected handle %s:%d(%s<%p>) winpid %d",
		     hl->func, hl->ln, hl->name, hl->h, hl->pid);
      system_printf (" by %s:%d(%s<%p>)", func, ln, name, h);
      return false;
    }

  handle_list *hln;
  if (hl && (hln = hl->next) && strcmp (name, hln->name) != 0)
    {
      system_printf ("closing protected handle %s:%d(%s<%p>)",
		     hln->func, hln->ln, hln->name, hln->h);
      system_printf (" by %s:%d(%s<%p>)", func, ln, name, h);
    }

  if (hl)
    delete_handle (hl);

  return true;
}

/* Close a known handle.  Complain if !force and closing a known handle or
   if the name of the handle being closed does not match the registered name. */
bool
close_handle (const char *func, int ln, HANDLE h, const char *name, bool force)
{
  bool ret;

  lock_debug here;
  if (!mark_closed (func, ln, h, name, force))
    return false;

  SetHandleInformation (h, HANDLE_FLAG_PROTECT_FROM_CLOSE, 0);
  ret = CloseHandle (h);

  if (!ret)
    {
      system_printf ("CloseHandle(%s<%p>) failed %s:%d, %E", name, h, func, ln);
      try_to_debug ();
    }
  return ret;
}
#endif /*DEBUGGING*/
