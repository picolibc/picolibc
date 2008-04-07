/* mmap.cc

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/param.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "sys/cygwin.h"
#include "ntdll.h"
#include <sys/queue.h>

/* __PROT_ATTACH indicates an anonymous mapping which is supposed to be
   attached to a file mapping for pages beyond the file's EOF.  The idea
   is to support mappings longer than the file, without the file growing
   to mapping length (POSIX semantics). */
#define __PROT_ATTACH   0x8000000
/* Filler pages are the pages from the last file backed page to the next
   64K boundary.  These pages are created as anonymous pages, but with
   the same page protection as the file's pages, since POSIX applications
   expect to be able to access this part the same way as the file pages. */
#define __PROT_FILLER   0x4000000

/* Stick with 4K pages for bookkeeping, otherwise we just get confused
   when trying to do file mappings with trailing filler pages correctly. */
#define PAGE_CNT(bytes) howmany((bytes),getsystempagesize())

#define PGBITS		(sizeof (DWORD)*8)
#define MAPSIZE(pages)	howmany ((pages), PGBITS)

#define MAP_SET(n)	(page_map[(n)/PGBITS] |= (1L << ((n) % PGBITS)))
#define MAP_CLR(n)	(page_map[(n)/PGBITS] &= ~(1L << ((n) % PGBITS)))
#define MAP_ISSET(n)	(page_map[(n)/PGBITS] & (1L << ((n) % PGBITS)))

/* Used for anonymous mappings. */
static fhandler_dev_zero fh_anonymous;

/* Used for thread synchronization while accessing mmap bookkeeping lists. */
static NO_COPY muto mmap_guard;
#define LIST_LOCK()    (mmap_guard.init ("mmap_guard")->acquire ())
#define LIST_UNLOCK()  (mmap_guard.release ())

/* Small helpers to avoid having lots of flag bit tests in the code. */
static inline bool
priv (int flags)
{
  return (flags & MAP_PRIVATE) == MAP_PRIVATE;
}

static inline bool
fixed (int flags)
{
  return (flags & MAP_FIXED) == MAP_FIXED;
}

static inline bool
anonymous (int flags)
{
  return (flags & MAP_ANONYMOUS) == MAP_ANONYMOUS;
}

static inline bool
noreserve (int flags)
{
  return (flags & MAP_NORESERVE) == MAP_NORESERVE;
}

static inline bool
autogrow (int flags)
{
  return (flags & MAP_AUTOGROW) == MAP_AUTOGROW;
}

static inline bool
attached (int prot)
{
  return (prot & __PROT_ATTACH) == __PROT_ATTACH;
}

static inline bool
filler (int prot)
{
  return (prot & __PROT_FILLER) == __PROT_FILLER;
}

static inline DWORD
gen_create_protect (DWORD openflags, int flags)
{
  DWORD ret = PAGE_READONLY;

  if (priv (flags))
    ret = PAGE_WRITECOPY;
  else if (openflags & GENERIC_WRITE)
    ret = PAGE_READWRITE;

  if (openflags & GENERIC_EXECUTE)
    ret <<= 4;

  return ret;
}

/* Generate Windows protection flags from mmap prot and flag values. */
static inline DWORD
gen_protect (int prot, int flags)
{
  DWORD ret = PAGE_NOACCESS;

  /* Attached pages are only reserved, but the protection must be a
     valid value, so we just return PAGE_READWRITE. */
  if (attached (prot))
    return PAGE_EXECUTE_READWRITE;

  if (prot & PROT_WRITE)
    ret = (priv (flags) && (!anonymous (flags) || filler (prot)))
	  ? PAGE_WRITECOPY : PAGE_READWRITE;
  else if (prot & PROT_READ)
    ret = PAGE_READONLY;

  if (prot & PROT_EXEC)
    ret <<= 4;

  return ret;
}

static HANDLE
CreateMapping (HANDLE fhdl, size_t len, _off64_t off, DWORD openflags,
	       int prot, int flags)
{
  HANDLE h;
  NTSTATUS ret;

  LARGE_INTEGER sectionsize = { QuadPart: len };
  ULONG protect = gen_create_protect (openflags, flags);
  ULONG attributes = attached (prot) ? SEC_RESERVE : SEC_COMMIT;

  OBJECT_ATTRIBUTES oa;
  InitializeObjectAttributes (&oa, NULL, OBJ_INHERIT, NULL,
			      sec_none.lpSecurityDescriptor);

  if (fhdl == INVALID_HANDLE_VALUE)
    {
      /* Standard anonymous mapping needs non-zero len. */
      ret = NtCreateSection (&h, SECTION_ALL_ACCESS, &oa,
			     &sectionsize, protect, attributes, NULL);
    }
  else if (autogrow (flags))
    {
      /* Auto-grow only works if the protection is PAGE_READWRITE.  So,
	 first we call NtCreateSection with PAGE_READWRITE, then, if the
	 requested protection is different, we close the mapping and
	 reopen it again with the correct protection, if auto-grow worked. */
      sectionsize.QuadPart += off;
      ret = NtCreateSection (&h, SECTION_ALL_ACCESS, &oa,
			     &sectionsize, PAGE_READWRITE, attributes, fhdl);
      if (NT_SUCCESS (ret) && protect != PAGE_READWRITE)
	{
	  NtClose (h);
	  ret = NtCreateSection (&h, SECTION_ALL_ACCESS, &oa,
				 &sectionsize, protect, attributes, fhdl);
	}
    }
  else
    {
      /* Zero len creates mapping for whole file and allows
	 AT_EXTENDABLE_FILE mapping, if we ever use it... */
      sectionsize.QuadPart = 0;
      ret = NtCreateSection (&h, SECTION_ALL_ACCESS, &oa,
			     &sectionsize, protect, attributes, fhdl);
    }
  if (!NT_SUCCESS (ret))
    {
      h = NULL;
      SetLastError (RtlNtStatusToDosError (ret));
    }
  return h;
}

void *
MapView (HANDLE h, void *addr, size_t len, DWORD openflags,
	 int prot, int flags, _off64_t off)
{
  NTSTATUS ret;
  LARGE_INTEGER offset = { QuadPart:off };
  DWORD protect = gen_create_protect (openflags, flags);
  void *base = addr;
  ULONG commitsize = attached (prot) ? 0 : len;
  ULONG viewsize = len;
  ULONG alloc_type = (base && !wincap.is_wow64 () ? AT_ROUND_TO_PAGE : 0)
		     | MEM_TOP_DOWN;

  /* Try mapping using the given address first, even if it's NULL.
     If it failed, and addr was not NULL and flags is not MAP_FIXED,
     try again with NULL address.

     Note: Retrying the mapping might be unnecessary, now that mmap64 checks
	   for a valid memory area first. */
  ret = NtMapViewOfSection (h, GetCurrentProcess (), &base, 0, commitsize,
			    &offset, &viewsize, ViewShare, alloc_type, protect);
  if (!NT_SUCCESS (ret) && addr && !fixed (flags))
    {
      base = NULL;
      ret = NtMapViewOfSection (h, GetCurrentProcess (), &base, 0, commitsize,
				&offset, &viewsize, ViewShare, 0, protect);
    }
  if (!NT_SUCCESS (ret))
    {
      base = NULL;
      SetLastError (RtlNtStatusToDosError (ret));
    }
  debug_printf ("%x = NtMapViewOfSection (h:%x, addr:%x, len:%u, off:%D, "
		"protect:%x, type:%x)", base, h, addr, len, off, protect, 0);
  return base;
}

/* Class structure used to keep a record of all current mmap areas
   in a process.  Needed for bookkeeping all mmaps in a process and
   for duplicating all mmaps after fork() since mmaps are not propagated
   to child processes by Windows.  All information must be duplicated
   by hand, see fixup_mmaps_after_fork().

   The class structure:

   One member of class map per process, global variable mmapped_areas.
   Contains a singly-linked list of type class mmap_list.  Each mmap_list
   entry represents all mapping to a file, keyed by file descriptor and
   file name hash.
   Each list entry contains a singly-linked list of type class mmap_record.
   Each mmap_record represents exactly one mapping.  For each mapping, there's
   an additional so called `page_map'.  It's an array of bits, one bit
   per mapped memory page.  The bit is set if the page is accessible,
   unset otherwise. */

class mmap_record
{
  public:
    LIST_ENTRY (mmap_record) mr_next;

  private:
    int fd;
    HANDLE mapping_hdl;
    DWORD openflags;
    int prot;
    int flags;
    _off64_t offset;
    DWORD len;
    caddr_t base_address;
    DWORD *page_map;
    device dev;

  public:
    mmap_record (int nfd, HANDLE h, DWORD of, int p, int f, _off64_t o, DWORD l,
		 caddr_t b) :
       fd (nfd),
       mapping_hdl (h),
       openflags (of),
       prot (p),
       flags (f),
       offset (o),
       len (l),
       base_address (b),
       page_map (NULL)
      {
	dev.devn = 0;
	if (fd >= 0 && !cygheap->fdtab.not_open (fd))
	  dev = cygheap->fdtab[fd]->dev ();
	else if (fd == -1)
	  dev.parse (FH_ZERO);
      }

    int get_fd () const { return fd; }
    HANDLE get_handle () const { return mapping_hdl; }
    device& get_device () { return dev; }
    int get_prot () const { return prot; }
    int get_openflags () const { return openflags; }
    int get_flags () const { return flags; }
    bool priv () const { return ::priv (flags); }
    bool fixed () const { return ::fixed (flags); }
    bool anonymous () const { return ::anonymous (flags); }
    bool noreserve () const { return ::noreserve (flags); }
    bool autogrow () const { return ::autogrow (flags); }
    bool attached () const { return ::attached (prot); }
    bool filler () const { return ::filler (prot); }
    _off64_t get_offset () const { return offset; }
    DWORD get_len () const { return len; }
    caddr_t get_address () const { return base_address; }

    bool alloc_page_map ();
    void free_page_map () { if (page_map) cfree (page_map); }

    DWORD find_unused_pages (DWORD pages) const;
    bool match (caddr_t addr, DWORD len, caddr_t &m_addr, DWORD &m_len);
    _off64_t map_pages (_off64_t off, DWORD len);
    bool map_pages (caddr_t addr, DWORD len);
    bool unmap_pages (caddr_t addr, DWORD len);
    int access (caddr_t address);

    fhandler_base *alloc_fh ();
    void free_fh (fhandler_base *fh);

    DWORD gen_create_protect () const
      { return ::gen_create_protect (get_openflags (), get_flags ()); }
    DWORD gen_protect () const
      { return ::gen_protect (get_prot (), get_flags ()); }
    bool compatible_flags (int fl) const;
};

class mmap_list
{
  public:
    LIST_ENTRY (mmap_list) ml_next;
    LIST_HEAD (, mmap_record) recs;

  private:
    int fd;
    __ino64_t hash;

  public:
    int get_fd () const { return fd; }
    __ino64_t get_hash () const { return hash; }

    bool anonymous () const { return fd == -1; }
    void set (int nfd, struct __stat64 *st);
    mmap_record *add_record (mmap_record r);
    bool del_record (mmap_record *rec);
    caddr_t try_map (void *addr, size_t len, int flags, _off64_t off);
};

class mmap_areas
{
  public:
    LIST_HEAD (, mmap_list) lists;

    mmap_list *get_list_by_fd (int fd, struct __stat64 *st);
    mmap_list *add_list (int fd, struct __stat64 *st);
    void del_list (mmap_list *ml);
};

/* This is the global map structure pointer. */
static mmap_areas mmapped_areas;

bool
mmap_record::compatible_flags (int fl) const
{
#define MAP_COMPATMASK	(MAP_TYPE | MAP_NORESERVE)
  return (get_flags () & MAP_COMPATMASK) == (fl & MAP_COMPATMASK);
}

DWORD
mmap_record::find_unused_pages (DWORD pages) const
{
  DWORD mapped_pages = PAGE_CNT (get_len ());
  DWORD start;

  if (pages > mapped_pages)
    return (DWORD)-1;
  for (start = 0; start <= mapped_pages - pages; ++start)
    if (!MAP_ISSET (start))
      {
	DWORD cnt;
	for (cnt = 0; cnt < pages; ++cnt)
	  if (MAP_ISSET (start + cnt))
	    break;
	if (cnt >= pages)
	  return start;
      }
  return (DWORD)-1;
}

bool
mmap_record::match (caddr_t addr, DWORD len, caddr_t &m_addr, DWORD &m_len)
{
  caddr_t low = (addr >= get_address ()) ? addr : get_address ();
  caddr_t high = get_address ();
  if (filler ())
    high += get_len ();
  else
    high += (PAGE_CNT (get_len ()) * getsystempagesize ());
  high = (addr + len < high) ? addr + len : high;
  if (low < high)
    {
      m_addr = low;
      m_len = high - low;
      return true;
    }
  return false;
}

bool
mmap_record::alloc_page_map ()
{
  /* Allocate one bit per page */
  if (!(page_map = (DWORD *) ccalloc (HEAP_MMAP,
				      MAPSIZE (PAGE_CNT (get_len ())),
				      sizeof (DWORD))))
    return false;

  DWORD start_protect = gen_create_protect ();
  DWORD real_protect = gen_protect ();
  if (real_protect != start_protect && !noreserve ()
      && !VirtualProtect (get_address (), get_len (),
			  real_protect, &start_protect))
    system_printf ("Warning: VirtualProtect (addr: %p, len: 0x%x, "
		   "new_prot: 0x%x, old_prot: 0x%x), %E",
		   get_address (), get_len (),
		   real_protect, start_protect);
  DWORD len = PAGE_CNT (get_len ());
  while (len-- > 0)
    MAP_SET (len);
  return true;
}

_off64_t
mmap_record::map_pages (_off64_t off, DWORD len)
{
  /* Used ONLY if this mapping matches into the chunk of another already
     performed mapping in a special case of MAP_ANON|MAP_PRIVATE.

     Otherwise it's job is now done by alloc_page_map(). */
  DWORD old_prot;
  debug_printf ("map_pages (fd=%d, off=%D, len=%u)", get_fd (), off, len);
  len = PAGE_CNT (len);

  if ((off = find_unused_pages (len)) == (DWORD)-1)
    return 0L;
  if (!noreserve ()
      && !VirtualProtect (get_address () + off * getsystempagesize (),
			  len * getsystempagesize (), gen_protect (),
			  &old_prot))
    {
      __seterrno ();
      return (_off64_t)-1;
    }

  while (len-- > 0)
    MAP_SET (off + len);
  return off * getsystempagesize ();
}

bool
mmap_record::map_pages (caddr_t addr, DWORD len)
{
  debug_printf ("map_pages (addr=%x, len=%u)", addr, len);
  DWORD old_prot;
  DWORD off = addr - get_address ();
  off /= getsystempagesize ();
  len = PAGE_CNT (len);
  /* First check if the area is unused right now. */
  for (DWORD l = 0; l < len; ++l)
    if (MAP_ISSET (off + l))
      {
	set_errno (EINVAL);
	return false;
      }
  if (!noreserve ()
      && !VirtualProtect (get_address () + off * getsystempagesize (),
			  len * getsystempagesize (), gen_protect (),
			  &old_prot))
    {
      __seterrno ();
      return false;
    }
  for (; len-- > 0; ++off)
    MAP_SET (off);
  return true;
}

bool
mmap_record::unmap_pages (caddr_t addr, DWORD len)
{
  DWORD old_prot;
  DWORD off = addr - get_address ();
  if (noreserve ()
      && !VirtualFree (get_address () + off, len, MEM_DECOMMIT))
    debug_printf ("VirtualFree in unmap_pages () failed, %E");
  else if (!VirtualProtect (get_address () + off, len, PAGE_NOACCESS,
			    &old_prot))
    debug_printf ("VirtualProtect in unmap_pages () failed, %E");

  off /= getsystempagesize ();
  len = PAGE_CNT (len);
  for (; len-- > 0; ++off)
    MAP_CLR (off);
  /* Return TRUE if all pages are free'd which may result in unmapping
     the whole chunk. */
  for (len = MAPSIZE (PAGE_CNT (get_len ())); len > 0; )
    if (page_map[--len])
      return false;
  return true;
}

int
mmap_record::access (caddr_t address)
{
  if (address < get_address () || address >= get_address () + get_len ())
    return 0;
  DWORD off = (address - get_address ()) / getsystempagesize ();
  return MAP_ISSET (off);
}

fhandler_base *
mmap_record::alloc_fh ()
{
  if (anonymous ())
    {
      fh_anonymous.set_io_handle (INVALID_HANDLE_VALUE);
      fh_anonymous.set_access (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
      return &fh_anonymous;
    }

  /* The file descriptor could have been closed or, even
     worse, could have been reused for another file before
     the call to fork(). This requires creating a fhandler
     of the correct type to be sure to call the method of the
     correct class. */
  fhandler_base *fh = build_fh_dev (get_device ());
  fh->set_access (get_openflags ());
  return fh;
}

void
mmap_record::free_fh (fhandler_base *fh)
{
  if (!anonymous ())
    cfree (fh);
}

mmap_record *
mmap_list::add_record (mmap_record r)
{
  mmap_record *rec = (mmap_record *) cmalloc (HEAP_MMAP, sizeof (mmap_record));
  if (!rec)
    return NULL;
  *rec = r;
  if (!rec->alloc_page_map ())
    {
      cfree (rec);
      return NULL;
    }
  LIST_INSERT_HEAD (&recs, rec, mr_next);
  return rec;
}

void
mmap_list::set (int nfd, struct __stat64 *st)
{
  fd = nfd;
  if (!anonymous ())
    {
      /* The fd isn't sufficient since it could already be the fd of another
	 file.  So we use the inode number as evaluated by fstat to identify
	 the file. */
      hash = st ? st->st_ino : (__ino64_t) 0;
    }
  LIST_INIT (&recs);
}

bool
mmap_list::del_record (mmap_record *rec)
{
  rec->free_page_map ();
  LIST_REMOVE (rec, mr_next);
  cfree (rec);
  /* Return true if the list is empty which allows the caller to remove
     this list from the list of lists. */
  return !LIST_FIRST(&recs);
}

caddr_t
mmap_list::try_map (void *addr, size_t len, int flags, _off64_t off)
{
  mmap_record *rec;

  if (off == 0 && !fixed (flags))
    {
      /* If MAP_FIXED isn't given, check if this mapping matches into the
	 chunk of another already performed mapping. */
      DWORD plen = PAGE_CNT (len);
      LIST_FOREACH (rec, &recs, mr_next)
	if (rec->find_unused_pages (plen) != (DWORD) -1)
	  break;
      if (rec && rec->compatible_flags (flags))
	{
	  if ((off = rec->map_pages (off, len)) == (_off64_t) -1)
	    return (caddr_t) MAP_FAILED;
	  return (caddr_t) rec->get_address () + off;
	}
    }
  else if (fixed (flags))
    {
      /* If MAP_FIXED is given, test if the requested area is in an
	 unmapped part of an still active mapping.  This can happen
	 if a memory region is unmapped and remapped with MAP_FIXED. */
      caddr_t u_addr;
      DWORD u_len;

      LIST_FOREACH (rec, &recs, mr_next)
	if (rec->match ((caddr_t) addr, len, u_addr, u_len))
	  break;
      if (rec)
	{
	  if (u_addr > (caddr_t) addr || u_addr + len < (caddr_t) addr + len
	      || !rec->compatible_flags (flags))
	    {
	      /* Partial match only, or access mode doesn't match. */
	      /* FIXME: Handle partial mappings gracefully if adjacent
		 memory is available. */
	      set_errno (EINVAL);
	      return (caddr_t) MAP_FAILED;
	    }
	  if (!rec->map_pages ((caddr_t) addr, len))
	    return (caddr_t) MAP_FAILED;
	  return (caddr_t) addr;
	}
    }
  return NULL;
}

mmap_list *
mmap_areas::get_list_by_fd (int fd, struct __stat64 *st)
{
  mmap_list *ml;
  LIST_FOREACH (ml, &lists, ml_next)
    {
      if (fd == -1 && ml->anonymous ())
	return ml;
      /* The fd isn't sufficient since it could already be the fd of another
	 file.  So we use the inode number as evaluated by fstat to identify
	 the file. */
      if (fd != -1 && st && ml->get_hash () == st->st_ino)
	return ml;
    }
  return 0;
}

mmap_list *
mmap_areas::add_list (int fd, struct __stat64 *st)
{
  mmap_list *ml = (mmap_list *) cmalloc (HEAP_MMAP, sizeof (mmap_list));
  if (!ml)
    return NULL;
  ml->set (fd, st);
  LIST_INSERT_HEAD (&lists, ml, ml_next);
  return ml;
}

void
mmap_areas::del_list (mmap_list *ml)
{
  LIST_REMOVE (ml, ml_next);
  cfree (ml);
}

/* This function is called from exception_handler when a segmentation
   violation has occurred.  It should also be called from all Cygwin
   functions that want to support passing noreserve mmap page addresses
   to Windows system calls.  In that case, it should be called only after
   a system call indicates that the application buffer passed had an
   invalid virtual address to avoid any performance impact in non-noreserve
   cases.

   Check if the address range is all within noreserve mmap regions.  If so,
   call VirtualAlloc to commit the pages and return MMAP_NORESERVE_COMMITED
   on success.  If the page has __PROT_ATTACH (SUSv3 memory protection
   extension), or if VirutalAlloc fails, return MMAP_RAISE_SIGBUS.
   Otherwise, return MMAP_NONE if the address range is not covered by an
   attached or noreserve map.

   On MAP_NORESERVE_COMMITED, the exeception handler should return 0 to
   allow the application to retry the memory access, or the calling Cygwin
   function should retry the Windows system call. */
mmap_region_status
mmap_is_attached_or_noreserve (void *addr, size_t len)
{
  mmap_region_status ret = MMAP_NONE;

  LIST_LOCK ();
  mmap_list *map_list = mmapped_areas.get_list_by_fd (-1, NULL);

  size_t pagesize = getpagesize ();
  caddr_t start_addr = (caddr_t) rounddown ((uintptr_t) addr, pagesize);
  len += ((caddr_t) addr - start_addr);
  len = roundup2 (len, pagesize);

  if (map_list == NULL)
    goto out;

  mmap_record *rec;
  caddr_t u_addr;
  DWORD u_len;

  LIST_FOREACH (rec, &map_list->recs, mr_next)
    {
      if (!rec->match (start_addr, len, u_addr, u_len))
	continue;
      if (rec->attached ())
	{
	  ret = MMAP_RAISE_SIGBUS;
	  break;
	}
      if (!rec->noreserve ())
	break;

      size_t commit_len = u_len - (start_addr - u_addr);
      if (commit_len > len)
	commit_len = len;

      if (!VirtualAlloc (start_addr, commit_len, MEM_COMMIT,
			 rec->gen_protect ()))
	{
	  ret = MMAP_RAISE_SIGBUS;
	  break;
	}

      start_addr += commit_len;
      len -= commit_len;
      if (!len)
	{
	  ret = MMAP_NORESERVE_COMMITED;
	  break;
	}
    }
out:
  LIST_UNLOCK ();
  return ret;
}

static caddr_t
mmap_worker (mmap_list *map_list, fhandler_base *fh, caddr_t base, size_t len,
	     int prot, int flags, int fd, _off64_t off, struct __stat64 *st)
{
  HANDLE h = fh->mmap (&base, len, prot, flags, off);
  if (h == INVALID_HANDLE_VALUE)
    return NULL;
  if (!map_list
      && !(map_list = mmapped_areas.get_list_by_fd (fd, st))
      && !(map_list = mmapped_areas.add_list (fd, st)))
    {
      fh->munmap (h, base, len);
      return NULL;
    }
  mmap_record mmap_rec (fd, h, fh->get_access (), prot, flags, off, len, base);
  mmap_record *rec = map_list->add_record (mmap_rec);
  if (!rec)
    {
      fh->munmap (h, base, len);
      return NULL;
    }
  return base;
}

extern "C" void *
mmap64 (void *addr, size_t len, int prot, int flags, int fd, _off64_t off)
{
  syscall_printf ("addr %x, len %u, prot %x, flags %x, fd %d, off %D",
		  addr, len, prot, flags, fd, off);

  caddr_t ret = (caddr_t) MAP_FAILED;
  fhandler_base *fh = NULL;
  fhandler_disk_file *fh_disk_file = NULL; /* Used for reopening a disk file
					      when necessary. */
  mmap_list *map_list = NULL;
  size_t orig_len = 0;
  caddr_t base = NULL;
  struct __stat64 st;

  DWORD pagesize = getpagesize ();

  fh_anonymous.set_io_handle (INVALID_HANDLE_VALUE);
  fh_anonymous.set_access (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);

  /* EINVAL error conditions. */
  if (off % pagesize
      || ((prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC)))
      || ((flags & MAP_TYPE) != MAP_SHARED
	  && (flags & MAP_TYPE) != MAP_PRIVATE)
      || (fixed (flags) && ((uintptr_t) addr % pagesize))
      || !len)
    {
      set_errno (EINVAL);
      goto out;
    }

  if (!anonymous (flags) && fd != -1)
    {
      /* Ensure that fd is open */
      cygheap_fdget cfd (fd);
      if (cfd < 0)
	goto out;

      fh = cfd;

      /* mmap /dev/zero is like MAP_ANONYMOUS. */
      if (fh->get_device () == FH_ZERO)
	flags |= MAP_ANONYMOUS;
    }

  if (anonymous (flags) || fd == -1)
    {
      fh = &fh_anonymous;
      fd = -1;
      flags |= MAP_ANONYMOUS;
      /* Anonymous mappings are always forced to pagesize length with
	 no offset. */
      len = roundup2 (len, pagesize);
      off = 0;
    }
  else if (fh->get_device () == FH_FS)
    {
      /* EACCES error conditions according to SUSv3.  File must be opened
	 for reading, regardless of the requested protection, and file must
	 be opened for writing when PROT_WRITE together with MAP_SHARED
	 is requested. */
      if (!(fh->get_access () & GENERIC_READ)
	  || (!(fh->get_access () & GENERIC_WRITE)
	      && (prot & PROT_WRITE) && !priv (flags)))
	{
	  set_errno (EACCES);
	  goto out;
	}

      /* You can't create mappings with PAGE_EXECUTE protection if
	 the file isn't explicitely opened with EXECUTE access. */
      UNICODE_STRING fname;
      OBJECT_ATTRIBUTES attr;
      NTSTATUS status;
      HANDLE h;
      IO_STATUS_BLOCK io;

      RtlInitUnicodeString (&fname, L"");
      InitializeObjectAttributes (&attr, &fname, OBJ_CASE_INSENSITIVE,
				  fh->get_handle (), NULL);
      status = NtOpenFile (&h,
			   fh->get_access () | GENERIC_EXECUTE | SYNCHRONIZE,
			   &attr, &io, FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT
			   | FILE_OPEN_FOR_BACKUP_INTENT);
      if (NT_SUCCESS (status))
	{
	  fh_disk_file = new (alloca (sizeof *fh_disk_file)) fhandler_disk_file;
	  fh_disk_file->set_name (fh->pc);
	  fh_disk_file->set_io_handle (h);
	  fh_disk_file->set_access (fh->get_access () | GENERIC_EXECUTE);
	  fh = fh_disk_file;
	}
      else if (prot & PROT_EXEC)
	{
	  /* TODO: To be or not to be... I'm opting for refusing this
	     mmap request rather than faking it, but that might break
	     some non-portable code. */
	  set_errno (EACCES);
	  goto out;
	}

      if (fh->fstat_by_handle (&st))
	{
	  __seterrno ();
	  goto out;
	}
      _off64_t fsiz = st.st_size;

      /* Don't allow file mappings beginning beyond EOF since Windows can't
	 handle that POSIX like, unless MAP_AUTOGROW flag is set, which
	 mimics Windows behaviour. */
      if (off >= fsiz && !autogrow (flags))
	{
	  /* Instead, it seems suitable to return an anonymous mapping of
	     the given size instead.  Mapped addresses beyond EOF aren't
	     written back to the file anyway, so the handling is identical
	     to other pages beyond EOF. */
	  fh = &fh_anonymous;
	  len = roundup2 (len, pagesize);
	  prot = PROT_READ | PROT_WRITE | __PROT_ATTACH;
	  flags &= MAP_FIXED;
	  flags |= MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
	  fd = -1;
	  off = 0;
	  goto go_ahead;
	}
      fsiz -= off;
      /* We're creating the pages beyond EOF as reserved, anonymous pages.
	 Note that this isn't done in WOW64 environments since apparently
	 WOW64 does not support the AT_ROUND_TO_PAGE flag which is required
	 to get this right.  Too bad. */
      if (!wincap.is_wow64 ()
	  && ((len > fsiz && !autogrow (flags)) || len < pagesize))
	orig_len = len;
      if (len > fsiz)
	{
	  if (autogrow (flags))
	    {
	      /* Allow mapping beyond EOF if MAP_AUTOGROW flag is set.
		 Check if file has been opened for writing, otherwise
		 MAP_AUTOGROW is invalid. */
	      if (!(fh->get_access () & GENERIC_WRITE))
		{
		  set_errno (EINVAL);
		  goto out;
		}
	    }
	  else
	    /* Otherwise, don't map beyond EOF, since Windows would change
	       the file to the new length, in contrast to POSIX. */
	    len = fsiz;
	}

      /* If the requested offset + len is <= file size, drop MAP_AUTOGROW.
	 This simplifes fhandler::mmap's job. */
      if (autogrow (flags) && (off + len) <= fsiz)
	flags &= ~MAP_AUTOGROW;
    }

go_ahead:

  /* MAP_NORESERVE is only supported on private anonymous mappings.
     Remove that bit from flags so that later code doesn't have to
     test all bits. */
  if (noreserve (flags) && (!anonymous (flags) || !priv (flags)))
    flags &= ~MAP_NORESERVE;

  LIST_LOCK ();
  map_list = mmapped_areas.get_list_by_fd (fd, &st);

  /* Test if an existing anonymous mapping can be recycled. */
  if (map_list && anonymous (flags))
    {
      caddr_t tried = map_list->try_map (addr, len, flags, off);
      /* try_map returns NULL if no map matched, otherwise it returns
	 a valid address, or MAP_FAILED in case of a fatal error. */
      if (tried)
	{
	  ret = tried;
	  goto out_with_unlock;
	}
    }

  if (orig_len)
    {
      /* If the requested length is bigger than the file size, we try to
	 allocate an area of the full size first.  This area is immediately
	 deallocated and the address we got is used as base address for the
	 subsequent real mappings.  This ensures that we have enough space
	 for the whole thing. */
      orig_len = roundup2 (orig_len, pagesize);
      PVOID newaddr = VirtualAlloc (addr, orig_len, MEM_TOP_DOWN | MEM_RESERVE,
				    PAGE_READWRITE);
      if (!newaddr)
	{
	  /* If addr is not NULL, but MAP_FIXED isn't given, allow the OS
	     to choose. */
	  if (addr && !fixed (flags))
	    newaddr = VirtualAlloc (NULL, orig_len, MEM_TOP_DOWN | MEM_RESERVE,
				    PAGE_READWRITE);
	  if (!newaddr)
	    {
	      __seterrno ();
	      goto out_with_unlock;
	    }
	}
      if (!VirtualFree (newaddr, 0, MEM_RELEASE))
	{
	  __seterrno ();
	  goto out_with_unlock;
	}
      addr = newaddr;
    }

  base = mmap_worker (map_list, fh, (caddr_t) addr, len, prot, flags, fd, off,
		      &st);
  if (!base)
    goto out_with_unlock;

  if (orig_len)
    {
      /* If the requested length is bigger than the file size, the
	 remainder is created as anonymous mapping.  Actually two
	 mappings are created, first the remainder from the file end to
	 the next 64K boundary as accessible pages with the same
	 protection as the file's pages, then as much pages as necessary
	 to accomodate the requested length, but as reserved pages which
	 raise a SIGBUS when trying to access them.  AT_ROUND_TO_PAGE
	 and page protection on shared pages is only supported by 32 bit NT,
	 so don't even try on WOW64.  This is accomplished by not setting
	 orig_len on WOW64 above. */
#if 0
      orig_len = roundup2 (orig_len, pagesize);
#endif
      len = roundup2 (len, getsystempagesize ());
      if (orig_len - len)
	{
	  orig_len -= len;
	  size_t valid_page_len = orig_len % pagesize;
	  size_t sigbus_page_len = orig_len - valid_page_len;

	  caddr_t at_base = base + len;
	  if (valid_page_len)
	    {
	      prot |= __PROT_FILLER;
	      flags &= MAP_SHARED | MAP_PRIVATE;
	      flags |= MAP_ANONYMOUS | MAP_FIXED;
	      at_base = mmap_worker (NULL, &fh_anonymous, at_base,
				     valid_page_len, prot, flags, -1, 0, NULL);
	      if (!at_base)
		{
		  fh->munmap (fh->get_handle (), base, len);
		  set_errno (ENOMEM);
		  goto out_with_unlock;
		}
	      at_base += valid_page_len;
	    }
	  if (sigbus_page_len)
	    {
	      prot = PROT_READ | PROT_WRITE | __PROT_ATTACH;
	      flags = MAP_ANONYMOUS | MAP_NORESERVE | MAP_FIXED;
	      at_base = mmap_worker (NULL, &fh_anonymous, at_base,
				     sigbus_page_len, prot, flags, -1, 0, NULL);
	      if (!at_base)
		debug_printf ("Warning: Mapping beyond EOF failed, %E");
	    }
	}
    }

  ret = base;

out_with_unlock:
  LIST_UNLOCK ();

out:

  if (fh_disk_file)
    NtClose (fh_disk_file->get_handle ());

  syscall_printf ("%p = mmap() ", ret);
  return ret;
}

extern "C" void *
mmap (void *addr, size_t len, int prot, int flags, int fd, _off_t off)
{
  return mmap64 (addr, len, prot, flags, fd, (_off64_t)off);
}

/* munmap () removes all mmapped pages between addr and addr+len. */

extern "C" int
munmap (void *addr, size_t len)
{
  syscall_printf ("munmap (addr %x, len %u)", addr, len);

  /* Error conditions according to SUSv3 */
  if (!addr || !len || check_invalid_virtual_addr (addr, len))
    {
      set_errno (EINVAL);
      return -1;
    }
  size_t pagesize = getpagesize ();
  if (((uintptr_t) addr % pagesize) || !len)
    {
      set_errno (EINVAL);
      return -1;
    }
  len = roundup2 (len, pagesize);

  LIST_LOCK ();

  /* Iterate through the map, unmap pages between addr and addr+len
     in all maps. */
  mmap_list *map_list, *next_map_list;
  LIST_FOREACH_SAFE (map_list, &mmapped_areas.lists, ml_next, next_map_list)
    {
      mmap_record *rec, *next_rec;
      caddr_t u_addr;
      DWORD u_len;

      LIST_FOREACH_SAFE (rec, &map_list->recs, mr_next, next_rec)
	{
	  if (!rec->match ((caddr_t) addr, len, u_addr, u_len))
	    continue;
	  if (rec->unmap_pages (u_addr, u_len))
	    {
	      /* The whole record has been unmapped, so we now actually
		 unmap it from the system in full length... */
	      fhandler_base *fh = rec->alloc_fh ();
	      fh->munmap (rec->get_handle (),
			  rec->get_address (),
			  rec->get_len ());
	      rec->free_fh (fh);

	      /* ...and delete the record. */
	      if (map_list->del_record (rec))
		{
		  /* Yay, the last record has been removed from the list,
		     we can remove the list now, too. */
		  mmapped_areas.del_list (map_list);
		  break;
		}
	    }
	}
    }

  LIST_UNLOCK ();
  syscall_printf ("0 = munmap(): %x", addr);
  return 0;
}

/* Sync file with memory. Ignore flags for now. */

extern "C" int
msync (void *addr, size_t len, int flags)
{
  int ret = -1;
  mmap_list *map_list;

  syscall_printf ("msync (addr: %p, len %u, flags %x)", addr, len, flags);

  LIST_LOCK ();

  if (((uintptr_t) addr % getpagesize ())
      || (flags & ~(MS_ASYNC | MS_SYNC | MS_INVALIDATE))
      || (flags & (MS_ASYNC | MS_SYNC) == (MS_ASYNC | MS_SYNC)))
    {
      set_errno (EINVAL);
      goto out;
    }
#if 0 /* If I only knew why I did that... */
  len = roundup2 (len, getpagesize ());
#endif

  /* Iterate through the map, looking for the mmapped area.
     Error if not found. */
  LIST_FOREACH (map_list, &mmapped_areas.lists, ml_next)
    {
      mmap_record *rec;
      LIST_FOREACH (rec, &map_list->recs, mr_next)
	{
	  if (rec->access ((caddr_t) addr))
	    {
	      /* Check whole area given by len. */
	      for (DWORD i = getpagesize (); i < len; i += getpagesize ())
		if (!rec->access ((caddr_t) addr + i))
		  {
		    set_errno (ENOMEM);
		    goto out;
		  }
	      fhandler_base *fh = rec->alloc_fh ();
	      ret = fh->msync (rec->get_handle (), (caddr_t) addr, len, flags);
	      rec->free_fh (fh);
	      goto out;
	    }
	}
    }

  /* No matching mapping exists. */
  set_errno (ENOMEM);

out:
  LIST_UNLOCK ();
  syscall_printf ("%d = msync()", ret);
  return ret;
}

/* Set memory protection */

extern "C" int
mprotect (void *addr, size_t len, int prot)
{
  bool in_mapped = false;
  bool ret = false;
  DWORD old_prot;
  DWORD new_prot = 0;

  syscall_printf ("mprotect (addr: %p, len %u, prot %x)", addr, len, prot);

  /* See comment in mmap64 for a description. */
  size_t pagesize = getpagesize ();
  if ((uintptr_t) addr % pagesize)
    {
      set_errno (EINVAL);
      goto out;
    }
  len = roundup2 (len, pagesize);

  LIST_LOCK ();

  /* Iterate through the map, protect pages between addr and addr+len
     in all maps. */
  mmap_list *map_list;
  LIST_FOREACH (map_list, &mmapped_areas.lists, ml_next)
    {
      mmap_record *rec;
      caddr_t u_addr;
      DWORD u_len;

      LIST_FOREACH (rec, &map_list->recs, mr_next)
	{
	  if (!rec->match ((caddr_t) addr, len, u_addr, u_len))
	    continue;
	  in_mapped = true;
	  if (rec->attached ())
	    continue;
	  new_prot = gen_protect (prot, rec->get_flags ());
	  if (rec->noreserve ())
	    {
	      if (new_prot == PAGE_NOACCESS)
		ret = VirtualFree (u_addr, u_len, MEM_DECOMMIT);
	      else
		ret = !!VirtualAlloc (u_addr, u_len, MEM_COMMIT, new_prot);
	    }
	  else
	    ret = VirtualProtect (u_addr, u_len, new_prot, &old_prot);
	  if (!ret)
	    {
	      __seterrno ();
	      break;
	    }
	}
    }

  LIST_UNLOCK ();

  if (!in_mapped)
    {
      int flags = 0;
      MEMORY_BASIC_INFORMATION mbi;

      ret = VirtualQuery (addr, &mbi, sizeof mbi);
      if (ret)
	{
	  /* If write protection is requested, check if the page was
	     originally protected writecopy.  In this case call VirtualProtect
	     requesting PAGE_WRITECOPY, otherwise the VirtualProtect will fail
	     on NT version >= 5.0 */
	  if (prot & PROT_WRITE)
	    {
	      if (mbi.AllocationProtect == PAGE_WRITECOPY
		  || mbi.AllocationProtect == PAGE_EXECUTE_WRITECOPY)
		flags = MAP_PRIVATE;
	    }
	  new_prot = gen_protect (prot, flags);
	  if (new_prot != PAGE_NOACCESS && mbi.State == MEM_RESERVE)
	    ret = VirtualAlloc (addr, len, MEM_COMMIT, new_prot);
	  else
	    ret = VirtualProtect (addr, len, new_prot, &old_prot);
	}
      if (!ret)
	__seterrno ();
    }

out:

  syscall_printf ("%d = mprotect ()", ret ? 0 : -1);
  return ret ? 0 : -1;
}

extern "C" int
mlock (const void *addr, size_t len)
{
  int ret = -1;

  /* Instead of using VirtualLock, which does not guarantee that the pages
     aren't swapped out when the process is inactive, we're using
     ZwLockVirtualMemory with the LOCK_VM_IN_RAM flag to do what mlock on
     POSIX systems does.  On NT, this requires SeLockMemoryPrivilege,
     which is given only to SYSTEM by default. */

  push_thread_privilege (SE_LOCK_MEMORY_PRIVILEGE, true);

  /* Align address and length values to page size. */
  size_t pagesize = getpagesize ();
  PVOID base = (PVOID) rounddown((uintptr_t) addr, pagesize);
  ULONG size = roundup2 (((uintptr_t) addr - (uintptr_t) base) + len, pagesize);
  NTSTATUS status = 0;
  do
    {
      status = NtLockVirtualMemory (hMainProc, &base, &size, LOCK_VM_IN_RAM);
      if (status == STATUS_WORKING_SET_QUOTA)
	{
	  /* The working set is too small, try to increase it so that the
	     requested locking region fits in.  Unfortunately I don't know
	     any function which would return the currently locked pages of
	     a process (no go with NtQueryVirtualMemory).

	     So, except for the border cases, what we do here is something
	     really embarrassing.  We raise the working set by 64K at a time
	     and retry, until either we fail to raise the working set size
	     further, or until NtLockVirtualMemory returns successfully (or
	     with another error).  */
	  ULONG min, max;
	  if (!GetProcessWorkingSetSize (hMainProc, &min, &max))
	    {
	      set_errno (ENOMEM);
	      break;
	    }
	  if (min < size)
	    min = size + pagesize;
	  else if (size < pagesize)
	    min += size;
	  else
	    min += pagesize;
	  if (max < min)
	    max = min;
	  if (!SetProcessWorkingSetSize (hMainProc, min, max))
	    {
	      set_errno (ENOMEM);
	      break;
	    }
	}
      else if (!NT_SUCCESS (status))
	__seterrno_from_nt_status (status);
      else
	ret = 0;
    }
  while (status == STATUS_WORKING_SET_QUOTA);

  pop_thread_privilege ();

  return ret;
}

extern "C" int
munlock (const void *addr, size_t len)
{
  int ret = -1;

  push_thread_privilege (SE_LOCK_MEMORY_PRIVILEGE, true);

  /* Align address and length values to page size. */
  size_t pagesize = getpagesize ();
  PVOID base = (PVOID) rounddown((uintptr_t) addr, pagesize);
  ULONG size = roundup2 (((uintptr_t) addr - (uintptr_t) base) + len, pagesize);
  NTSTATUS status = NtUnlockVirtualMemory (hMainProc, &base, &size,
					   LOCK_VM_IN_RAM);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  else
    ret = 0;

  pop_thread_privilege ();

  return ret;
}

extern "C" int
posix_madvise (void *addr, size_t len, int advice)
{
  /* Check parameters. */
  if (advice < POSIX_MADV_NORMAL || advice > POSIX_MADV_DONTNEED
      || !len)
    return EINVAL;

  /* Check requested memory area. */
  MEMORY_BASIC_INFORMATION m;
  char *p = (char *) addr;
  char *endp = p + len;
  while (p < endp)
    {
      if (!VirtualQuery (p, &m, sizeof m) || m.State == MEM_FREE)
	return ENOMEM;
      p = (char *) m.BaseAddress + m.RegionSize;
    }

  /* Eventually do nothing. */
  return 0;
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
fhandler_base::mmap (caddr_t *addr, size_t len, int prot,
		     int flags, _off64_t off)
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

bool
fhandler_base::fixup_mmap_after_fork (HANDLE h, int prot, int flags,
				      _off64_t offset, DWORD size,
				      void *address)
{
  set_errno (ENODEV);
  return -1;
}

/* Implementation for anonymous maps.  Using fhandler_dev_zero looks
   quite the natural way. */
HANDLE
fhandler_dev_zero::mmap (caddr_t *addr, size_t len, int prot,
			 int flags, _off64_t off)
{
  HANDLE h;
  void *base;

  if (priv (flags) && !filler (prot))
    {
      /* Private anonymous maps are now implemented using VirtualAlloc.
	 This has two advantages:

	 - VirtualAlloc has a smaller footprint than a copy-on-write
	   anonymous map.

	 - It supports decommitting using VirtualFree, in contrast to
	   section maps.  This allows minimum footprint private maps,
	   when using the (non-POSIX, yay-Linux) MAP_NORESERVE flag.
      */
      DWORD protect = gen_protect (prot, flags);
      DWORD alloc_type = MEM_TOP_DOWN | MEM_RESERVE
			 | (noreserve (flags) ? 0 : MEM_COMMIT);
      base = VirtualAlloc (*addr, len, alloc_type, protect);
      if (!base && addr && !fixed (flags))
	base = VirtualAlloc (NULL, len, alloc_type, protect);
      if (!base || (fixed (flags) && base != *addr))
	{
	  if (!base)
	    __seterrno ();
	  else
	    {
	      VirtualFree (base, 0, MEM_RELEASE);
	      set_errno (EINVAL);
	      debug_printf ("VirtualAlloc: address shift with MAP_FIXED given");
	    }
	  return INVALID_HANDLE_VALUE;
	}
      h = (HANDLE) 1; /* Fake handle to indicate success. */
    }
  else
    {
      h = CreateMapping (get_handle (), len, off, get_access (), prot, flags);
      if (!h)
	{
	  __seterrno ();
	  debug_printf ("CreateMapping failed with %E");
	  return INVALID_HANDLE_VALUE;
	}

      base = MapView (h, *addr, len, get_access(), prot, flags, off);
      if (!base || (fixed (flags) && base != *addr))
	{
	  if (!base)
	    __seterrno ();
	  else
	    {
	      UnmapViewOfFile (base);
	      set_errno (EINVAL);
	      debug_printf ("MapView: address shift with MAP_FIXED given");
	    }
	  NtClose (h);
	  return INVALID_HANDLE_VALUE;
	}
    }
  *addr = (caddr_t) base;
  return h;
}

int
fhandler_dev_zero::munmap (HANDLE h, caddr_t addr, size_t len)
{
  if (h == (HANDLE) 1)	/* See fhandler_dev_zero::mmap. */
    VirtualFree (addr, 0, MEM_RELEASE);
  else
    {
      UnmapViewOfFile (addr);
      NtClose (h);
    }
  return 0;
}

int
fhandler_dev_zero::msync (HANDLE h, caddr_t addr, size_t len, int flags)
{
  return 0;
}

bool
fhandler_dev_zero::fixup_mmap_after_fork (HANDLE h, int prot, int flags,
				      _off64_t offset, DWORD size,
				      void *address)
{
  /* Re-create the map */
  void *base;
  if (priv (flags) && !filler (prot))
    {
      DWORD alloc_type = MEM_RESERVE | (noreserve (flags) ? 0 : MEM_COMMIT);
      /* Always allocate R/W so that ReadProcessMemory doesn't fail
	 due to a non-writable target address.  The protection is
	 set to the correct one anyway in the fixup loop. */
      base = VirtualAlloc (address, size, alloc_type, PAGE_READWRITE);
    }
  else
    base = MapView (h, address, size, get_access (), prot, flags, offset);
  if (base != address)
    {
      MEMORY_BASIC_INFORMATION m;
      VirtualQuery (address, &m, sizeof (m));
      system_printf ("requested %p != %p mem alloc base %p, state %p, "
		     "size %d, %E", address, base, m.AllocationBase, m.State,
		     m.RegionSize);
    }
  return base == address;
}

/* Implementation for disk files and anonymous mappings. */
HANDLE
fhandler_disk_file::mmap (caddr_t *addr, size_t len, int prot,
			  int flags, _off64_t off)
{
  HANDLE h = CreateMapping (get_handle (), len, off, get_access (),
			    prot, flags);
  if (!h)
    {
      __seterrno ();
      debug_printf ("CreateMapping failed with %E");
      return INVALID_HANDLE_VALUE;
    }

  void *base = MapView (h, *addr, len, get_access (), prot, flags, off);
  if (!base || (fixed (flags) && base != *addr))
    {
      if (!base)
	__seterrno ();
      else
	{
	  UnmapViewOfFile (base);
	  set_errno (EINVAL);
	  debug_printf ("MapView: address shift with MAP_FIXED given");
	}
      NtClose (h);
      return INVALID_HANDLE_VALUE;
    }

  *addr = (caddr_t) base;
  return h;
}

int
fhandler_disk_file::munmap (HANDLE h, caddr_t addr, size_t len)
{
  UnmapViewOfFile (addr);
  NtClose (h);
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

bool
fhandler_disk_file::fixup_mmap_after_fork (HANDLE h, int prot, int flags,
					   _off64_t offset, DWORD size,
					   void *address)
{
  /* Re-create the map */
  void *base = MapView (h, address, size, get_access (), prot, flags, offset);
  if (base != address)
    {
      MEMORY_BASIC_INFORMATION m;
      VirtualQuery (address, &m, sizeof (m));
      system_printf ("requested %p != %p mem alloc base %p, state %p, "
		     "size %d, %E", address, base, m.AllocationBase, m.State,
		     m.RegionSize);
    }
  return base == address;
}

HANDLE
fhandler_dev_mem::mmap (caddr_t *addr, size_t len, int prot,
			int flags, _off64_t off)
{
  if (off >= mem_size
      || (DWORD) len >= mem_size
      || off + len >= mem_size)
    {
      set_errno (EINVAL);
      debug_printf ("-1 = mmap(): illegal parameter, set EINVAL");
      return INVALID_HANDLE_VALUE;
    }

  UNICODE_STRING memstr;
  RtlInitUnicodeString (&memstr, L"\\device\\physicalmemory");

  OBJECT_ATTRIBUTES attr;
  InitializeObjectAttributes (&attr, &memstr,
			      OBJ_CASE_INSENSITIVE | OBJ_INHERIT,
			      NULL, NULL);

  /* Section access is bit-wise ored, while on the Win32 level access
     is only one of the values.  It's not quite clear if the section
     access has to be defined this way, or if SECTION_ALL_ACCESS would
     be sufficient but this worked fine so far, so why change? */
  ACCESS_MASK section_access;
  if (prot & PROT_WRITE)
    section_access = SECTION_MAP_READ | SECTION_MAP_WRITE;
  else
    section_access = SECTION_MAP_READ;

  HANDLE h;
  NTSTATUS ret = NtOpenSection (&h, section_access, &attr);
  if (!NT_SUCCESS (ret))
    {
      __seterrno_from_nt_status (ret);
      debug_printf ("-1 = mmap(): NtOpenSection failed with %E");
      return INVALID_HANDLE_VALUE;
    }

  void *base = MapView (h, *addr, len, get_access (), prot,
			flags | MAP_ANONYMOUS, off);
  if (!base || (fixed (flags) && base != *addr))
    {
      if (!base)
	__seterrno ();
      else
	{
	  NtUnmapViewOfSection (GetCurrentProcess (), base);
	  set_errno (EINVAL);
	  debug_printf ("MapView: address shift with MAP_FIXED given");
	}
      NtClose (h);
      return INVALID_HANDLE_VALUE;
    }

  *addr = (caddr_t) base;
  return h;
}

int
fhandler_dev_mem::munmap (HANDLE h, caddr_t addr, size_t len)
{
  NTSTATUS ret;
  if (!NT_SUCCESS (ret = NtUnmapViewOfSection (INVALID_HANDLE_VALUE, addr)))
    {
      __seterrno_from_nt_status (ret);
      return -1;
    }
  NtClose (h);
  return 0;
}

int
fhandler_dev_mem::msync (HANDLE h, caddr_t addr, size_t len, int flags)
{
  return 0;
}

bool
fhandler_dev_mem::fixup_mmap_after_fork (HANDLE h, int prot, int flags,
					 _off64_t offset, DWORD size,
					 void *address)
{
  void *base = MapView (h, address, size, get_access (), prot,
			flags | MAP_ANONYMOUS, offset);
  if (base != address)
    {
      MEMORY_BASIC_INFORMATION m;
      VirtualQuery (address, &m, sizeof (m));
      system_printf ("requested %p != %p mem alloc base %p, state %p, "
		     "size %d, %E", address, base, m.AllocationBase, m.State,
		     m.RegionSize);
    }
  return base == address;
}

/* Call to re-create all the file mappings in a forked child. Called from
   the child in initialization. At this point we are passed a valid
   mmapped_areas map, and all the HANDLE's are valid for the child, but
   none of the mapped areas are in our address space. We need to iterate
   through the map, doing the MapViewOfFile calls.  */

int __stdcall
fixup_mmaps_after_fork (HANDLE parent)
{
  /* Iterate through the map */
  mmap_list *map_list;
  LIST_FOREACH (map_list, &mmapped_areas.lists, ml_next)
    {
      mmap_record *rec;
      LIST_FOREACH (rec, &map_list->recs, mr_next)
	{
	  debug_printf ("fd %d, h 0x%x, address %p, len 0x%x, prot: 0x%x, "
			"flags: 0x%x, offset %X",
			rec->get_fd (), rec->get_handle (), rec->get_address (),
			rec->get_len (), rec->get_prot (), rec->get_flags (),
			rec->get_offset ());

	  fhandler_base *fh = rec->alloc_fh ();
	  bool ret = fh->fixup_mmap_after_fork (rec->get_handle (),
						rec->get_prot (),
						rec->get_flags () | MAP_FIXED,
						rec->get_offset (),
						rec->get_len (),
						rec->get_address ());
	  rec->free_fh (fh);

	  if (!ret)
	    {
	      if (rec->attached ())
		{
		  system_printf ("Warning: Fixup mapping beyond EOF failed");
		  continue;
		}
	      return -1;
	    }

	  MEMORY_BASIC_INFORMATION mbi;
	  DWORD old_prot;

	  for (char *address = rec->get_address ();
	       address < rec->get_address () + rec->get_len ();
	       address += mbi.RegionSize)
	    {
	      if (!VirtualQueryEx (parent, address, &mbi, sizeof mbi))
		{
		  system_printf ("VirtualQueryEx failed for MAP_PRIVATE "
				 "address %p, %E", address);
		  return -1;
		}
	      /* Just skip reserved pages. */
	      if (mbi.State == MEM_RESERVE)
		continue;
	      /* Copy-on-write pages must be copied to the child to circumvent
		 a strange notion how copy-on-write is supposed to work. */
	      if (rec->priv ())
		{
		  if (rec->noreserve ()
		      && !VirtualAlloc (address, mbi.RegionSize,
					MEM_COMMIT, PAGE_READWRITE))
		    {
		      system_printf ("VirtualAlloc failed for MAP_PRIVATE "
				     "address %p, %E", address);
		      return -1;
		    }
		  if (mbi.Protect == PAGE_NOACCESS
		      && !VirtualProtectEx (parent, address, mbi.RegionSize,
					    PAGE_READONLY, &old_prot))
		    {
		      system_printf ("VirtualProtectEx failed for MAP_PRIVATE "
				     "address %p, %E", address);
		      return -1;
		    }
		  else if ((mbi.AllocationProtect == PAGE_WRITECOPY
			    || mbi.AllocationProtect == PAGE_EXECUTE_WRITECOPY)
			   && (mbi.Protect == PAGE_READWRITE
			       || mbi.Protect == PAGE_EXECUTE_READWRITE))
		    /* A WRITECOPY page which has been written to is set to
		       READWRITE, but that's an incompatible protection to
		       set the page to.  Convert the protection to WRITECOPY
		       so that the below VirtualProtect doesn't fail. */
		    mbi.Protect <<= 1;

		  if (!ReadProcessMemory (parent, address, address,
					  mbi.RegionSize, NULL))
		    {
		      system_printf ("ReadProcessMemory failed for MAP_PRIVATE "
				     "address %p, %E", address);
		      return -1;
		    }
		  if (mbi.Protect == PAGE_NOACCESS
		      && !VirtualProtectEx (parent, address, mbi.RegionSize,
					    PAGE_NOACCESS, &old_prot))
		    {
		      system_printf ("WARNING: VirtualProtectEx to return to "
				     "PAGE_NOACCESS state in parent failed for "
				     "MAP_PRIVATE address %p, %E", address);
		      return -1;
		    }
		}
	      /* Set child page protection to parent protection */
	      if (!VirtualProtect (address, mbi.RegionSize,
				   mbi.Protect, &old_prot))
		{
		  MEMORY_BASIC_INFORMATION m;
		  VirtualQuery (address, &m, sizeof m);
		  system_printf ("VirtualProtect failed for "
				 "address %p, "
				 "parentstate: 0x%x, "
				 "state: 0x%x, "
				 "parentprot: 0x%x, "
				 "prot: 0x%x, %E",
				 address, mbi.State, m.State,
				 mbi.Protect, m.Protect);
		  return -1;
		}
	    }
	}
    }

  debug_printf ("succeeded");
  return 0;
}
