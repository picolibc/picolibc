/* mmap.cc

   Copyright 1996, 1997, 1998, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <stddef.h>
#include <sys/mman.h>
#include <errno.h>
#include "fhandler.h"
#include "dtable.h"
#include "cygerrno.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "security.h"

/*
 * Simple class used to keep a record of all current
 * mmap areas in a process. Needed so that
 * they can be duplicated after a fork().
 */

class mmap_record
{
  private:
    HANDLE mapping_handle_;
    DWORD access_mode_;
    DWORD offset_;
    DWORD size_to_map_;
    void *base_address_;

  public:
    mmap_record (HANDLE h, DWORD ac, DWORD o, DWORD s, void *b) :
       mapping_handle_ (h), access_mode_ (ac), offset_ (o),
       size_to_map_ (s), base_address_ (b) { ; }

    /* Default Copy constructor/operator=/destructor are ok */

    /* Simple accessors */
    HANDLE get_handle () const { return mapping_handle_; }
    DWORD get_access () const { return access_mode_; }
    DWORD get_offset () const { return offset_; }
    DWORD get_size () const { return size_to_map_; }
    void *get_address () const { return base_address_; }
};

class list {
public:
  mmap_record *recs;
  int nrecs, maxrecs;
  int fd;
  list ();
  ~list ();
  void add_record (mmap_record r);
  void erase (int i);
};

list::list ()
{
  recs = (mmap_record *) malloc (10 * sizeof(mmap_record));
  nrecs = 0;
  maxrecs = 10;
  fd = 0;
}

list::~list ()
{
  free (recs);
}

void
list::add_record (mmap_record r)
{
  if (nrecs == maxrecs)
    {
      maxrecs += 5;
      recs = (mmap_record *) realloc (recs, maxrecs * sizeof (mmap_record));
    }
  recs[nrecs++] = r;
}

void
list::erase (int i)
{
  for (; i < nrecs-1; i++)
    recs[i] = recs[i+1];
  nrecs--;
}

class map {
public:
  list **lists;
  int nlists, maxlists;
  map ();
  ~map ();
  list *get_list_by_fd (int fd);
  list *add_list (list *l, int fd);
  void erase (int i);
};

map::map ()
{
  lists = (list **) malloc (10 * sizeof(list *));
  nlists = 0;
  maxlists = 10;
}

map::~map ()
{
  free (lists);
}

list *
map::get_list_by_fd (int fd)
{
  int i;
  for (i=0; i<nlists; i++)
    if (lists[i]->fd == fd)
      return lists[i];
  return 0;
}

list *
map::add_list (list *l, int fd)
{
  l->fd = fd;
  if (nlists == maxlists)
    {
      maxlists += 5;
      lists = (list **) realloc (lists, maxlists * sizeof (list *));
    }
  lists[nlists++] = l;
  return lists[nlists-1];
}

void
map::erase (int i)
{
  for (; i < nlists-1; i++)
    lists[i] = lists[i+1];
  nlists--;
}

/*
 * Code to keep a record of all mmap'ed areas in a process.
 * Needed to duplicate tham in a child of fork().
 * mmap_record classes are kept in an STL list in an STL map, keyed
 * by file descriptor. This is *NOT* duplicated accross a fork(), it
 * needs to be specially handled by the fork code.
 */

static map *mmapped_areas;

extern "C"
caddr_t
mmap (caddr_t addr, size_t len, int prot, int flags, int fd, off_t off)
{
  syscall_printf ("addr %x, len %d, prot %x, flags %x, fd %d, off %d",
		  addr, len, prot, flags, fd, off);

  DWORD access = (prot & PROT_WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ;
  if (flags & MAP_PRIVATE)
    access = FILE_MAP_COPY;

  SetResourceLock(LOCK_MMAP_LIST,READ_LOCK|WRITE_LOCK," mmap");

#if 0
  /* Windows 95 does not have fixed addresses */
  /*
   * CV: This assumption isn't correct. See Microsoft Platform SDK, Memory,
   * description of call `MapViewOfFileEx'.
   */
  if ((os_being_run != winNT) && (flags & MAP_FIXED))
    {
      set_errno (EINVAL);
      syscall_printf ("-1 = mmap(): win95 and MAP_FIXED");
      return (caddr_t) -1;
    }
#endif

  if (mmapped_areas == NULL)
    {
      /* First mmap call, create STL map */
      mmapped_areas = new map;
      if (mmapped_areas == NULL)
	{
	  set_errno (ENOMEM);
	  syscall_printf ("-1 = mmap(): ENOMEM");
	  return (caddr_t) -1;
	}
    }

  fhandler_disk_file fh_paging_file (NULL);
  fhandler_base *fh;
  caddr_t base = addr;
  HANDLE h;

  if ((flags & MAP_ANONYMOUS) || fd == -1)
    {
      fh_paging_file.set_io_handle (INVALID_HANDLE_VALUE);
      fh = &fh_paging_file;
    }
  else
    {
      /* Ensure that fd is open */
      if (fdtab.not_open (fd))
	{
	  set_errno (EBADF);
	  syscall_printf ("-1 = mmap(): EBADF");
	  ReleaseResourceLock(LOCK_MMAP_LIST,READ_LOCK|WRITE_LOCK," mmap");
	  return (caddr_t) -1;
	}
      fh = fdtab[fd];
    }

  h = fh->mmap (&base, len, access, flags, off);

  if (h == INVALID_HANDLE_VALUE)
    {
      ReleaseResourceLock(LOCK_MMAP_LIST,READ_LOCK|WRITE_LOCK," mmap");
      return MAP_FAILED;
    }

  /* Now we should have a successfully mmaped area.
     Need to save it so forked children can reproduce it.
  */
  mmap_record mmap_rec (h, access, off, len, base);

  /* Get list of mmapped areas for this fd, create a new one if
     one does not exist yet.
  */
  list *l = mmapped_areas->get_list_by_fd (fd);
  if (l == 0)
    {
      /* Create a new one */
      l = new list;
      if (l == 0)
        {
          fh->munmap (h, base, len);
          set_errno (ENOMEM);
          syscall_printf ("-1 = mmap(): ENOMEM");
          ReleaseResourceLock(LOCK_MMAP_LIST,READ_LOCK|WRITE_LOCK," mmap");
          return MAP_FAILED;
        }
      l = mmapped_areas->add_list (l, fd);
  }

  /* Insert into the list */
  l->add_record (mmap_rec);

  syscall_printf ("%x = mmap() succeeded", base);
  ReleaseResourceLock(LOCK_MMAP_LIST,READ_LOCK|WRITE_LOCK," mmap");
  return base;
}

/* munmap () removes an mmapped area.  It insists that base area
   requested is the same as that mmapped, error if not. */

extern "C"
int
munmap (caddr_t addr, size_t len)
{
  syscall_printf ("munmap (addr %x, len %d)", addr, len);

  SetResourceLock(LOCK_MMAP_LIST,WRITE_LOCK|READ_LOCK," munmap");
  /* Check if a mmap'ed area was ever created */
  if (mmapped_areas == NULL)
    {
      syscall_printf ("-1 = munmap(): mmapped_areas == NULL");
      set_errno (EINVAL);
      ReleaseResourceLock(LOCK_MMAP_LIST,WRITE_LOCK|READ_LOCK," munmap");
      return -1;
    }

  /* Iterate through the map, looking for the mmapped area.
     Error if not found. */

  int it;
  for (it = 0; it < mmapped_areas->nlists; ++it)
    {
      list *l = mmapped_areas->lists[it];
      if (l != 0)
	{
	  int li;
	  for (li = 0; li < l->nrecs; ++li)
	    {
	      mmap_record rec = l->recs[li];
	      if (rec.get_address () == addr)
		{
                  int fd = l->fd;
                  fhandler_disk_file fh_paging_file (NULL);
                  fhandler_base *fh;

                  if (fd == -1 || fdtab.not_open (fd))
                    {
                      fh_paging_file.set_io_handle (INVALID_HANDLE_VALUE);
                      fh = &fh_paging_file;
                    }
                  else
                    fh = fdtab[fd];
                  fh->munmap (rec.get_handle (), addr, len);

		  /* Delete the entry. */
		  l->erase (li);
		  syscall_printf ("0 = munmap(): %x", addr);
		  ReleaseResourceLock(LOCK_MMAP_LIST,WRITE_LOCK|READ_LOCK," munmap");
		  return 0;
		}
	     }
	 }
     }

  set_errno (EINVAL);
  syscall_printf ("-1 = munmap(): EINVAL");

  ReleaseResourceLock(LOCK_MMAP_LIST,WRITE_LOCK|READ_LOCK," munmap");
  return -1;
}

/* Sync file with memory. Ignore flags for now. */

extern "C"
int
msync (caddr_t addr, size_t len, int flags)
{
  syscall_printf ("addr = %x, len = %d, flags = %x",
		  addr, len, flags);

  /* However, check flags for validity. */
  if ((flags & ~(MS_ASYNC | MS_SYNC | MS_INVALIDATE))
      || ((flags & MS_ASYNC) && (flags & MS_SYNC)))
    {
      syscall_printf ("-1 = msync(): Invalid flags");
      set_errno (EINVAL);
      return -1;
    }

  SetResourceLock(LOCK_MMAP_LIST,WRITE_LOCK|READ_LOCK," msync");
  /* Check if a mmap'ed area was ever created */
  if (mmapped_areas == NULL)
    {
      syscall_printf ("-1 = msync(): mmapped_areas == NULL");
      set_errno (EINVAL);
      ReleaseResourceLock(LOCK_MMAP_LIST,WRITE_LOCK|READ_LOCK," msync");
      return -1;
    }

  /* Iterate through the map, looking for the mmapped area.
     Error if not found. */

  int it;
  for (it = 0; it < mmapped_areas->nlists; ++it)
    {
      list *l = mmapped_areas->lists[it];
      if (l != 0)
	{
	  int li;
	  for (li = 0; li < l->nrecs; ++li)
	    {
	      mmap_record rec = l->recs[li];
	      if (rec.get_address () == addr)
		{
                  int fd = l->fd;
                  fhandler_disk_file fh_paging_file (NULL);
                  fhandler_base *fh;

                  if (fd == -1 || fdtab.not_open (fd))
                    {
                      fh_paging_file.set_io_handle (INVALID_HANDLE_VALUE);
                      fh = &fh_paging_file;
                    }
                  else
                    fh = fdtab[fd];

                  int ret = fh->msync (rec.get_handle (), addr, len, flags);

                  if (ret)
		    syscall_printf ("%d = msync(): %E", ret);
                  else
		    syscall_printf ("0 = msync()");

		  ReleaseResourceLock(LOCK_MMAP_LIST,WRITE_LOCK|READ_LOCK," msync");
		  return 0;
		}
	     }
	 }
     }

  /* SUSv2: Return code if indicated memory was not mapped is ENOMEM. */
  set_errno (ENOMEM);
  syscall_printf ("-1 = msync(): ENOMEM");

  ReleaseResourceLock(LOCK_MMAP_LIST,WRITE_LOCK|READ_LOCK," msync");
  return -1;
}

/*
 * Base implementation:
 *
 * `mmap' returns ENODEV as documented in SUSv2.
 * In contrast to the global function implementation, the member function
 * `mmap' has to return the mapped base address in `addr' and the handle to
 * the mapping object as return value. In case of failure, the fhandler
 * mmap has to close that handle by itself and return INVALID_HANDLE_VALUE.
 *
 * `munmap' and `msync' get the handle to the mapping object as first parameter
 * additionally.
*/
HANDLE
fhandler_base::mmap (caddr_t *addr, size_t len, DWORD access,
                     int flags, off_t off)
{
  set_errno (ENODEV);
  return INVALID_HANDLE_VALUE;
}

int
fhandler_base::munmap (HANDLE h, caddr_t addr, size_t len)
{
  set_errno (ENODEV);
  return -1;
}

int
fhandler_base::msync (HANDLE h, caddr_t addr, size_t len, int flags)
{
  set_errno (ENODEV);
  return -1;
}

/* Implementation for disk files. */
HANDLE
fhandler_disk_file::mmap (caddr_t *addr, size_t len, DWORD access,
                          int flags, off_t off)
{
  DWORD protect;

  if (access & FILE_MAP_COPY)
    protect = PAGE_WRITECOPY;
  else if (access & FILE_MAP_WRITE)
    protect = PAGE_READWRITE;
  else
    protect = PAGE_READONLY;

  HANDLE h = CreateFileMapping (get_handle(), &sec_none, protect, 0, len, NULL);
  if (h == 0)
    {
      __seterrno ();
      syscall_printf ("-1 = mmap(): CreateFileMapping failed with %E");
      return INVALID_HANDLE_VALUE;
    }

  void *base = MapViewOfFileEx (h, access, 0, off, len,
                               (flags & MAP_FIXED) ? addr : NULL);

  if (!base || ((flags & MAP_FIXED) && base != addr))
    {
      if (!base)
        {
          __seterrno ();
          syscall_printf ("-1 = mmap(): MapViewOfFileEx failed with %E");
        }
      else
        {
	  set_errno (EINVAL);
          syscall_printf ("-1 = mmap(): address shift with MAP_FIXED given");
        }
      CloseHandle (h);
      return INVALID_HANDLE_VALUE;
    }

  *addr = (caddr_t) base;
  return h;
}

int
fhandler_disk_file::munmap (HANDLE h, caddr_t addr, size_t len)
{
  UnmapViewOfFile (addr);
  CloseHandle (h);
  return 0;
}

int
fhandler_disk_file::msync (HANDLE h, caddr_t addr, size_t len, int flags)
{
  if (FlushViewOfFile (addr, len) == 0)
    {
      __seterrno ();
      return -1;
    }
  return 0;
}

/* Set memory protection */

extern "C"
int
mprotect (caddr_t addr, size_t len, int prot)
{
  DWORD old_prot;
  DWORD new_prot = 0;

  syscall_printf ("mprotect (addr %x, len %d, prot %x)", addr, len, prot);

  if (prot == PROT_NONE)
    new_prot = PAGE_NOACCESS;
  else
    {
      switch (prot)
	{
	  case PROT_READ | PROT_WRITE | PROT_EXEC:
	    new_prot = PAGE_EXECUTE_READWRITE;
	    break;
	  case PROT_READ | PROT_WRITE:
	    new_prot = PAGE_READWRITE;
	    break;
	  case PROT_READ | PROT_EXEC:
	    new_prot = PAGE_EXECUTE_READ;
	    break;
	  case PROT_READ:
	    new_prot = PAGE_READONLY;
	    break;
	  default:
	    syscall_printf ("-1 = mprotect (): invalid prot value");
	    set_errno (EINVAL);
	    return -1;
	 }
     }

  if (VirtualProtect (addr, len, new_prot, &old_prot) == 0)
    {
      __seterrno ();
      syscall_printf ("-1 = mprotect (): %E");
      return -1;
    }

  syscall_printf ("0 = mprotect ()");
  return 0;
}

/*
 * Call to re-create all the file mappings in a forked
 * child. Called from the child in initialization. At this
 * point we are passed a valid mmapped_areas map, and all the
 * HANDLE's are valid for the child, but none of the
 * mapped areas are in our address space. We need to iterate
 * through the map, doing the MapViewOfFile calls.
 */

int __stdcall
fixup_mmaps_after_fork ()
{

  debug_printf ("recreate_mmaps_after_fork, mmapped_areas %p", mmapped_areas);

  /* Check if a mmapped area was ever created */
  if (mmapped_areas == NULL)
    return 0;

  /* Iterate through the map */
  for (int it = 0; it < mmapped_areas->nlists; ++it)
    {
      list *l = mmapped_areas->lists[it];
      if (l != 0)
	{
	  int li;
	  for (li = 0; li < l->nrecs; ++li)
	    {
	      mmap_record rec = l->recs[li];

	      debug_printf ("h %x, access %x, offset %d, size %d, address %p",
		  rec.get_handle (), rec.get_access (), rec.get_offset (),
		  rec.get_size (), rec.get_address ());

	      /* Now re-create the MapViewOfFileEx call */
	      void *base = MapViewOfFileEx (rec.get_handle (),
					    rec.get_access (), 0,
					    rec.get_offset (),
					    rec.get_size (),
					    rec.get_address ());
	      if (base != rec.get_address ())
		{
		  system_printf ("base address %p fails to match requested address %p",
				 rec.get_address ());
		  return -1;
		}
	    }
	}
    }

  debug_printf ("succeeded");
  return 0;
}
