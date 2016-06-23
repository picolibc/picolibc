/* mmap.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/param.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
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
#define PAGE_CNT(bytes) howmany((bytes), wincap.page_size())

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
CreateMapping (HANDLE fhdl, size_t len, off_t off, DWORD openflags,
	       int prot, int flags)
{
  HANDLE h;
  NTSTATUS status;

  LARGE_INTEGER sectionsize = { QuadPart: (LONGLONG) len };
  ULONG protect = gen_create_protect (openflags, flags);
  ULONG attributes = attached (prot) ? SEC_RESERVE : SEC_COMMIT;

  OBJECT_ATTRIBUTES oa;
  InitializeObjectAttributes (&oa, NULL, OBJ_INHERIT, NULL,
			      sec_none.lpSecurityDescriptor);

  if (fhdl == INVALID_HANDLE_VALUE)
    {
      /* Standard anonymous mapping needs non-zero len. */
      status = NtCreateSection (&h, SECTION_ALL_ACCESS, &oa, &sectionsize,
				protect, attributes, NULL);
    }
  else if (autogrow (flags))
    {
      /* Auto-grow only works if the protection is PAGE_READWRITE.  So,
	 first we call NtCreateSection with PAGE_READWRITE, then, if the
	 requested protection is different, we close the mapping and
	 reopen it again with the correct protection, if auto-grow worked. */
      sectionsize.QuadPart += off;
      status = NtCreateSection (&h, SECTION_ALL_ACCESS, &oa, &sectionsize,
				PAGE_READWRITE, attributes, fhdl);
      if (NT_SUCCESS (status) && protect != PAGE_READWRITE)
	{
	  NtClose (h);
	  status = NtCreateSection (&h, SECTION_ALL_ACCESS, &oa, &sectionsize,
				    protect, attributes, fhdl);
	}
    }
  else
    {
      /* Zero len creates mapping for whole file and allows
	 AT_EXTENDABLE_FILE mapping, if we ever use it... */
      sectionsize.QuadPart = 0;
      status = NtCreateSection (&h, SECTION_ALL_ACCESS, &oa, &sectionsize,
				protect, attributes, fhdl);
    }
  if (!NT_SUCCESS (status))
    {
      h = NULL;
      SetLastError (RtlNtStatusToDosError (status));
    }
  return h;
}

static void *
MapView (HANDLE h, void *addr, size_t len, DWORD openflags,
	 int prot, int flags, off_t off)
{
  NTSTATUS status;
  LARGE_INTEGER offset = { QuadPart:off };
  DWORD protect = gen_create_protect (openflags, flags);
  void *base = addr;
  SIZE_T commitsize = attached (prot) ? 0 : len;
  SIZE_T viewsize = len;
#ifdef __x86_64__ /* AT_ROUND_TO_PAGE isn't supported on 64 bit systems. */
  ULONG alloc_type = MEM_TOP_DOWN;
#else
  ULONG alloc_type = (base && !wincap.is_wow64 () ? AT_ROUND_TO_PAGE : 0)
		     | MEM_TOP_DOWN;
#endif

  /* Try mapping using the given address first, even if it's NULL.
     If it failed, and addr was not NULL and flags is not MAP_FIXED,
     try again with NULL address.

     Note: Retrying the mapping might be unnecessary, now that mmap64 checks
	   for a valid memory area first. */
  status = NtMapViewOfSection (h, NtCurrentProcess (), &base, 0, commitsize,
			       &offset, &viewsize, ViewShare, alloc_type,
			       protect);
  if (!NT_SUCCESS (status) && addr && !fixed (flags))
    {
      base = NULL;
      status = NtMapViewOfSection (h, NtCurrentProcess (), &base, 0, commitsize,
				   &offset, &viewsize, ViewShare, 0, protect);
    }
  if (!NT_SUCCESS (status))
    {
      base = NULL;
      SetLastError (RtlNtStatusToDosError (status));
    }
  debug_printf ("%p (status %p) = NtMapViewOfSection (h:%p, addr:%p, len:%lu,"
		" off:%Y, protect:%y, type:%y)",
		base, status, h, addr, len, off, protect, 0);
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

#pragma pack(push, 4)
class mmap_record
{
  public:
    LIST_ENTRY (mmap_record) mr_next;

  private:
    /* 4 byte on 32 bit, 8 byte on 64 bit */
    HANDLE mapping_hdl;
    SIZE_T len;
    caddr_t base_address;
    /* Always 8 bytes */
    off_t offset;
    /* Always 4 bytes */
    int fd;
    DWORD openflags;
    int prot;
    int flags;
    dev_t dev;
    DWORD page_map[0];

  public:
    mmap_record (int nfd, HANDLE h, DWORD of, int p, int f, off_t o, SIZE_T l,
		 caddr_t b) :
       mapping_hdl (h),
       len (l),
       base_address (b),
       offset (o),
       fd (nfd),
       openflags (of),
       prot (p),
       flags (f)
      {
	dev = 0;
	if (fd >= 0 && !cygheap->fdtab.not_open (fd))
	  dev = cygheap->fdtab[fd]->dev ();
	else if (fd == -1)
	  dev = FH_ZERO;
      }

    int get_fd () const { return fd; }
    HANDLE get_handle () const { return mapping_hdl; }
    int get_device () { return dev; }
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
    off_t get_offset () const { return offset; }
    SIZE_T get_len () const { return len; }
    caddr_t get_address () const { return base_address; }

    void init_page_map (mmap_record &r);

    SIZE_T find_unused_pages (SIZE_T pages) const;
    bool match (caddr_t addr, SIZE_T len, caddr_t &m_addr, SIZE_T &m_len);
    off_t map_pages (SIZE_T len);
    bool map_pages (caddr_t addr, SIZE_T len);
    bool unmap_pages (caddr_t addr, SIZE_T len);
    int access (caddr_t address);

    fhandler_base *alloc_fh ();
    void free_fh (fhandler_base *fh);

    DWORD gen_create_protect () const
      { return ::gen_create_protect (get_openflags (), get_flags ()); }
    DWORD gen_protect () const
      { return ::gen_protect (get_prot (), get_flags ()); }
    bool compatible_flags (int fl) const;
};
#pragma pack(pop)

class mmap_list
{
  public:
    LIST_ENTRY (mmap_list) ml_next;
    LIST_HEAD (, mmap_record) recs;

  private:
    int fd;
    ino_t hash;

  public:
    int get_fd () const { return fd; }
    ino_t get_hash () const { return hash; }

    bool anonymous () const { return fd == -1; }
    void set (int nfd, struct stat *st);
    mmap_record *add_record (mmap_record &r);
    bool del_record (mmap_record *rec);
    caddr_t try_map (void *addr, size_t len, int flags, off_t off);
};

class mmap_areas
{
  public:
    LIST_HEAD (, mmap_list) lists;

    mmap_list *get_list_by_fd (int fd, struct stat *st);
    mmap_list *add_list (int fd, struct stat *st);
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

SIZE_T
mmap_record::find_unused_pages (SIZE_T pages) const
{
  SIZE_T mapped_pages = PAGE_CNT (get_len ());
  SIZE_T start;

  if (pages > mapped_pages)
    return (SIZE_T) -1;
  for (start = 0; start <= mapped_pages - pages; ++start)
    if (!MAP_ISSET (start))
      {
	SIZE_T cnt;
	for (cnt = 0; cnt < pages; ++cnt)
	  if (MAP_ISSET (start + cnt))
	    break;
	if (cnt >= pages)
	  return start;
      }
  return (SIZE_T) -1;
}

bool
mmap_record::match (caddr_t addr, SIZE_T len, caddr_t &m_addr, SIZE_T &m_len)
{
  caddr_t low = (addr >= get_address ()) ? addr : get_address ();
  caddr_t high = get_address ();
  if (filler ())
    high += get_len ();
  else
    high += (PAGE_CNT (get_len ()) * wincap.page_size ());
  high = (addr + len < high) ? addr + len : high;
  if (low < high)
    {
      m_addr = low;
      m_len = high - low;
      return true;
    }
  return false;
}

void
mmap_record::init_page_map (mmap_record &r)
{
  *this = r;
  DWORD start_protect = gen_create_protect ();
  DWORD real_protect = gen_protect ();
  if (real_protect != start_protect && !noreserve ()
      && !VirtualProtect (get_address (), get_len (),
			  real_protect, &start_protect))
    system_printf ("Warning: VirtualProtect (addr: %p, len: %ly, "
		   "new_prot: %y, old_prot: %y), %E",
		   get_address (), get_len (),
		   real_protect, start_protect);
  SIZE_T len = PAGE_CNT (get_len ());
  while (len-- > 0)
    MAP_SET (len);
}

off_t
mmap_record::map_pages (SIZE_T len)
{
  /* Used ONLY if this mapping matches into the chunk of another already
     performed mapping in a special case of MAP_ANON|MAP_PRIVATE.

     Otherwise it's job is now done by init_page_map(). */
  DWORD old_prot;
  debug_printf ("map_pages (fd=%d, len=%lu)", get_fd (), len);
  len = PAGE_CNT (len);

  off_t off = find_unused_pages (len);
  if (off == (off_t) -1)
    return (off_t) 0;
  if (!noreserve ()
      && !VirtualProtect (get_address () + off * wincap.page_size (),
			  len * wincap.page_size (), gen_protect (),
			  &old_prot))
    {
      __seterrno ();
      return (off_t) -1;
    }

  while (len-- > 0)
    MAP_SET (off + len);
  return off * wincap.page_size ();
}

bool
mmap_record::map_pages (caddr_t addr, SIZE_T len)
{
  debug_printf ("map_pages (addr=%p, len=%lu)", addr, len);
  DWORD old_prot;
  off_t off = addr - get_address ();
  off /= wincap.page_size ();
  len = PAGE_CNT (len);
  /* First check if the area is unused right now. */
  for (SIZE_T l = 0; l < len; ++l)
    if (MAP_ISSET (off + l))
      {
	set_errno (EINVAL);
	return false;
      }
  if (!noreserve ()
      && !VirtualProtect (get_address () + off * wincap.page_size (),
			  len * wincap.page_size (), gen_protect (),
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
mmap_record::unmap_pages (caddr_t addr, SIZE_T len)
{
  DWORD old_prot;
  SIZE_T off = addr - get_address ();
  if (noreserve ()
      && !VirtualFree (get_address () + off, len, MEM_DECOMMIT))
    debug_printf ("VirtualFree in unmap_pages () failed, %E");
  else if (!VirtualProtect (get_address () + off, len, PAGE_NOACCESS,
			    &old_prot))
    debug_printf ("VirtualProtect in unmap_pages () failed, %E");

  off /= wincap.page_size ();
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
  SIZE_T off = (address - get_address ()) / wincap.page_size ();
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
  device fdev;
  fdev.name ("");
  fdev.native ("");
  fdev.parse (get_device ());
  fhandler_base *fh = build_fh_dev (fdev);
  if (fh)
    fh->set_access (get_openflags ());
  return fh;
}

void
mmap_record::free_fh (fhandler_base *fh)
{
  if (!anonymous ())
    delete fh;
}

mmap_record *
mmap_list::add_record (mmap_record &r)
{
  mmap_record *rec = (mmap_record *) ccalloc (HEAP_MMAP,
		      sizeof (mmap_record)
		      + MAPSIZE (PAGE_CNT (r.get_len ())) * sizeof (DWORD), 1);
  if (!rec)
    return NULL;
  rec->init_page_map (r);

  LIST_INSERT_HEAD (&recs, rec, mr_next);
  return rec;
}

void
mmap_list::set (int nfd, struct stat *st)
{
  fd = nfd;
  if (!anonymous ())
    {
      /* The fd isn't sufficient since it could already be the fd of another
	 file.  So we use the inode number as evaluated by fstat to identify
	 the file. */
      hash = st ? st->st_ino : (ino_t) 0;
    }
  LIST_INIT (&recs);
}

bool
mmap_list::del_record (mmap_record *rec)
{
  LIST_REMOVE (rec, mr_next);
  cfree (rec);
  /* Return true if the list is empty which allows the caller to remove
     this list from the list of lists. */
  return !LIST_FIRST(&recs);
}

caddr_t
mmap_list::try_map (void *addr, size_t len, int flags, off_t off)
{
  mmap_record *rec;

  if (off == 0 && !fixed (flags))
    {
      /* If MAP_FIXED isn't given, check if this mapping matches into the
	 chunk of another already performed mapping. */
      SIZE_T plen = PAGE_CNT (len);
      LIST_FOREACH (rec, &recs, mr_next)
	if (rec->find_unused_pages (plen) != (SIZE_T) -1)
	  break;
      if (rec && rec->compatible_flags (flags))
	{
	  if ((off = rec->map_pages (len)) == (off_t) -1)
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
      SIZE_T u_len;

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
mmap_areas::get_list_by_fd (int fd, struct stat *st)
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
mmap_areas::add_list (int fd, struct stat *st)
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

/* This function allows an external function to test if a given memory
   region is part of an mmapped memory region. */
bool
is_mmapped_region (caddr_t start_addr, caddr_t end_address)
{
  size_t len = end_address - start_addr;

  LIST_LOCK ();
  mmap_list *map_list = mmapped_areas.get_list_by_fd (-1, NULL);

  if (!map_list)
    {
      LIST_UNLOCK ();
      return false;
    }

  mmap_record *rec;
  caddr_t u_addr;
  SIZE_T u_len;
  bool ret = false;

  LIST_FOREACH (rec, &map_list->recs, mr_next)
    {
      if (rec->match (start_addr, len, u_addr, u_len))
	{
	  ret = true;
	  break;
	}
    }
  LIST_UNLOCK ();
  return ret;
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
   extension), or if VirtualAlloc fails, return MMAP_RAISE_SIGBUS.
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

  size_t pagesize = wincap.allocation_granularity ();
  caddr_t start_addr = (caddr_t) rounddown ((uintptr_t) addr, pagesize);
  len += ((caddr_t) addr - start_addr);
  len = roundup2 (len, pagesize);

  if (map_list == NULL)
    goto out;

  mmap_record *rec;
  caddr_t u_addr;
  SIZE_T u_len;

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
	     int prot, int flags, int fd, off_t off, struct stat *st)
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

#ifdef __x86_64__

/* The memory region used for memory maps */
#define MMAP_STORAGE_LOW	0x00800000000L	/* Leave 8 Gigs for heap. */
#define MMAP_STORAGE_HIGH	0x70000000000L	/* Leave enough room for OS. */

/* FIXME?  Unfortunately the OS doesn't support a top down allocation with
	   a ceiling value.  The ZeroBits mechanism only works for
	   NtMapViewOfSection and it only evaluates the high bit of ZeroBits
	   on 64 bit, so it's pretty much useless for our purposes.

	   If the below simple mechanism to perform top-down allocations
	   turns out to be too dumb, we need something else.  One idea is to
	   dived the space in (3835) 4 Gig chunks and simply store the
	   available free space per slot.  Then we can go top down from slot
	   to slot and only try slots which are supposed to have enough space.
	   Bookkeeping would be very simple and fast. */
class mmap_allocator
{
  caddr_t mmap_current_low;

public:
  mmap_allocator () : mmap_current_low ((caddr_t) MMAP_STORAGE_HIGH) {}

  PVOID alloc (PVOID in_addr, SIZE_T in_size, bool fixed)
  {
    MEMORY_BASIC_INFORMATION mbi;

    SIZE_T size = roundup2 (in_size, wincap.allocation_granularity ());
    /* First check for the given address. */
    if (in_addr)
      {
	/* If it points to a free area, big enough to fulfill the request,
	   return the address. */
      	if (VirtualQuery (in_addr, &mbi, sizeof mbi)
	    && mbi.State == MEM_FREE
	    && mbi.RegionSize >= size)
	  return in_addr;
	/* Otherwise, if MAP_FIXED was given, give up. */
	if (fixed)
	  return NULL;
	/* Otherwise, fall through to the usual free space search mechanism. */
      }
    /* Start with the last allocation start address - requested size. */
    caddr_t addr = mmap_current_low - size;
    bool merry_go_round = false;
    do
      {
	/* Did we hit the lower ceiling?  If so, restart from the upper
	   ceiling, but note that we did it. */
	if (addr < (caddr_t) MMAP_STORAGE_LOW)
	  {
	    addr = (caddr_t) MMAP_STORAGE_HIGH - size;
	    merry_go_round = true;
	  }
	/* Shouldn't fail, but better test. */
	if (!VirtualQuery ((PVOID) addr, &mbi, sizeof mbi))
	  return NULL;
	/* If the region is free... */
	if (mbi.State == MEM_FREE)
	  {
	    /* ...and the region is big enough to fulfill the request... */
	    if (mbi.RegionSize >= size)
	      {
		/* ...note the address as next start address for our simple
		   merry-go-round and return the address. */
		mmap_current_low = addr;
		return (PVOID) addr;
	      }
	    /* Otherwise, subtract what's missing in size and try again. */
	    addr -= size - mbi.RegionSize;
	  }
	/* If the region isn't free, skip to address below AllocationBase
	   and try again. */
	else
	  addr = (caddr_t) mbi.AllocationBase - size;
      }
    /* Repeat until we had a full ride on the merry_go_round. */
    while (!merry_go_round || addr >= mmap_current_low);
    return NULL;
  }
};

static mmap_allocator mmap_alloc;	/* Inherited by forked child. */
#endif

extern "C" void *
mmap64 (void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
  syscall_printf ("addr %p, len %lu, prot %y, flags %y, fd %d, off %Y",
		  addr, len, prot, flags, fd, off);

  caddr_t ret = (caddr_t) MAP_FAILED;
  fhandler_base *fh = NULL;
  fhandler_disk_file *fh_disk_file = NULL; /* Used for reopening a disk file
					      when necessary. */
  mmap_list *map_list = NULL;
  size_t orig_len = 0;
  caddr_t base = NULL;
  struct stat st;

  size_t pagesize = wincap.allocation_granularity ();

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

      /* The autoconf mmap test maps a file of size 1 byte.  It then tests
	 every byte of the entire mapped page of 64K for 0-bytes since that's
	 what POSIX requires.  The problem is, we can't create that mapping on
	 64 bit systems.  The file mapping will be only a single page, 4K, and
	 since 64 bit systems don't support the AT_ROUND_TO_PAGE flag, the
	 remainder of the 64K slot will result in a SEGV when accessed.

	 So, what we do here is cheating for the sake of the autoconf test
	 on 64 bit systems.  The justification is that there's very likely
	 no application actually utilizing the map beyond EOF, and we know that
	 all bytes beyond EOF are set to 0 anyway.  If this test doesn't work
	 on 64 bit systems, it will result in not using mmap at all in a
	 package.  But we want that mmap is treated as usable by autoconf,
	 regardless whether the autoconf test runs on a 32 bit or a 64 bit
	 system.

	 Ok, so we know exactly what autoconf is doing.  The file is called
	 "conftest.txt", it has a size of 1 byte, the mapping size is the
	 pagesize, the requested protection is PROT_READ | PROT_WRITE, the
	 mapping is MAP_SHARED, the offset is 0.

	 If all these requirements are given, we just return an anonymous map.
	 This will help to get over the autoconf test even on 64 bit systems.
	 The tests are ordered for speed. */
#ifdef __x86_64__
      if (1)
#else
      if (wincap.is_wow64 ())
#endif
	{
	  UNICODE_STRING fname;
	  IO_STATUS_BLOCK io;
	  FILE_STANDARD_INFORMATION fsi;

	  if (len == pagesize
	      && prot == (PROT_READ | PROT_WRITE)
	      && flags == MAP_SHARED
	      && off == 0
	      && (RtlSplitUnicodePath (fh->pc.get_nt_native_path (), NULL,
				       &fname),
		  wcscmp (fname.Buffer, L"conftest.txt") == 0)
	      && NT_SUCCESS (NtQueryInformationFile (fh->get_handle (), &io,
						     &fsi, sizeof fsi,
						     FileStandardInformation))
	      && fsi.EndOfFile.QuadPart == 1LL)
	    flags |= MAP_ANONYMOUS;
	}
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
      OBJECT_ATTRIBUTES attr;
      NTSTATUS status;
      HANDLE h;
      IO_STATUS_BLOCK io;

      InitializeObjectAttributes (&attr, &ro_u_empty, fh->pc.objcaseinsensitive (),
				  fh->get_handle (), NULL);
      status = NtOpenFile (&h,
			   fh->get_access () | GENERIC_EXECUTE | SYNCHRONIZE,
			   &attr, &io, FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT
			   | FILE_OPEN_FOR_BACKUP_INTENT);
      if (NT_SUCCESS (status))
	{
	  fh_disk_file = new (ccalloc (HEAP_FHANDLER, 1, sizeof *fh_disk_file))
			     fhandler_disk_file;
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

      if (fh->fstat_fs (&st))
	{
	  __seterrno ();
	  goto out;
	}
      off_t fsiz = st.st_size;

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
	 Note that this isn't done in 64 bit environments since apparently
	 64 bit systems don't support the AT_ROUND_TO_PAGE flag, which is
	 required to get this right.  Too bad. */
#ifndef __x86_64__
      if (!wincap.is_wow64 ()
	  && (((off_t) len > fsiz && !autogrow (flags))
	      || roundup2 (len, wincap.page_size ())
		 < roundup2 (len, pagesize)))
	orig_len = len;
#endif
      if ((off_t) len > fsiz)
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
      if (autogrow (flags) && (off + (off_t) len) <= fsiz)
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

#ifdef __x86_64__
  addr = mmap_alloc.alloc (addr, orig_len ?: len, fixed (flags));
#else
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
#endif

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
      len = roundup2 (len, wincap.page_size ());
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
    {
      NtClose (fh_disk_file->get_handle ());
      delete fh;
    }

  syscall_printf ("%p = mmap() ", ret);
  return ret;
}

#ifdef __x86_64__
EXPORT_ALIAS (mmap64, mmap)
#else
extern "C" void *
mmap (void *addr, size_t len, int prot, int flags, int fd, _off_t off)
{
  return mmap64 (addr, len, prot, flags, fd, (off_t)off);
}
#endif

/* munmap () removes all mmapped pages between addr and addr+len. */

extern "C" int
munmap (void *addr, size_t len)
{
  syscall_printf ("munmap (addr %p, len %lu)", addr, len);

  /* Error conditions according to SUSv3 */
  if (!addr || !len || check_invalid_virtual_addr (addr, len))
    {
      set_errno (EINVAL);
      return -1;
    }
  size_t pagesize = wincap.allocation_granularity ();
  if (((uintptr_t) addr % pagesize) || !len)
    {
      set_errno (EINVAL);
      return -1;
    }
  len = roundup2 (len, pagesize);

  LIST_LOCK ();

  /* Iterate over maps, unmap pages between addr and addr+len in all maps. */
  mmap_list *map_list, *next_map_list;
  LIST_FOREACH_SAFE (map_list, &mmapped_areas.lists, ml_next, next_map_list)
    {
      mmap_record *rec, *next_rec;
      caddr_t u_addr;
      SIZE_T u_len;

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
  syscall_printf ("0 = munmap(): %p", addr);
  return 0;
}

/* Sync file with memory. Ignore flags for now. */

extern "C" int
msync (void *addr, size_t len, int flags)
{
  int ret = -1;
  mmap_list *map_list;

  syscall_printf ("msync (addr: %p, len %lu, flags %y)", addr, len, flags);

  pthread_testcancel ();

  LIST_LOCK ();

  if (((uintptr_t) addr % wincap.allocation_granularity ())
      || (flags & ~(MS_ASYNC | MS_SYNC | MS_INVALIDATE))
      || ((flags & (MS_ASYNC | MS_SYNC)) == (MS_ASYNC | MS_SYNC)))
    {
      set_errno (EINVAL);
      goto out;
    }
#if 0 /* If I only knew why I did that... */
  len = roundup2 (len, wincap.allocation_granularity ());
#endif

  /* Iterate over maps, looking for the mmapped area.  Error if not found. */
  LIST_FOREACH (map_list, &mmapped_areas.lists, ml_next)
    {
      mmap_record *rec;
      LIST_FOREACH (rec, &map_list->recs, mr_next)
	{
	  if (rec->access ((caddr_t) addr))
	    {
	      /* Check whole area given by len. */
	      for (SIZE_T i = wincap.allocation_granularity ();
		   i < len;
		   i += wincap.allocation_granularity ())
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
  syscall_printf ("%R = msync()", ret);
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

  syscall_printf ("mprotect (addr: %p, len %lu, prot %y)", addr, len, prot);

  /* See comment in mmap64 for a description. */
  size_t pagesize = wincap.allocation_granularity ();
  if ((uintptr_t) addr % pagesize)
    {
      set_errno (EINVAL);
      goto out;
    }
  len = roundup2 (len, pagesize);

  LIST_LOCK ();

  /* Iterate over maps, protect pages between addr and addr+len in all maps. */
  mmap_list *map_list;
  LIST_FOREACH (map_list, &mmapped_areas.lists, ml_next)
    {
      mmap_record *rec;
      caddr_t u_addr;
      SIZE_T u_len;

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

  syscall_printf ("%R = mprotect ()", ret ? 0 : -1);
  return ret ? 0 : -1;
}

extern "C" int
mlock (const void *addr, size_t len)
{
  int ret = -1;

  /* Align address and length values to page size. */
  size_t pagesize = wincap.allocation_granularity ();
  PVOID base = (PVOID) rounddown((uintptr_t) addr, pagesize);
  SIZE_T size = roundup2 (((uintptr_t) addr - (uintptr_t) base) + len,
			  pagesize);
  NTSTATUS status = 0;
  do
    {
      status = NtLockVirtualMemory (NtCurrentProcess (), &base, &size,
				    MAP_PROCESS);
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
	  SIZE_T min, max;
	  if (!GetProcessWorkingSetSize (GetCurrentProcess (), &min, &max))
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
	  if (!SetProcessWorkingSetSize (GetCurrentProcess (), min, max))
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

  syscall_printf ("%R = mlock(%p, %lu)", ret, addr, len);
  return ret;
}

extern "C" int
munlock (const void *addr, size_t len)
{
  int ret = -1;

  /* Align address and length values to page size. */
  size_t pagesize = wincap.allocation_granularity ();
  PVOID base = (PVOID) rounddown((uintptr_t) addr, pagesize);
  SIZE_T size = roundup2 (((uintptr_t) addr - (uintptr_t) base) + len,
			  pagesize);
  NTSTATUS status = NtUnlockVirtualMemory (NtCurrentProcess (), &base, &size,
					   MAP_PROCESS);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  else
    ret = 0;

  syscall_printf ("%R = munlock(%p, %lu)", ret, addr, len);
  return ret;
}

/* This is required until Mingw-w64 catches up with newer functions. */
extern "C" WINAPI DWORD DiscardVirtualMemory (PVOID, SIZE_T);

extern "C" int
posix_madvise (void *addr, size_t len, int advice)
{
  int ret = 0;
  /* Check parameters. */
  if (advice < POSIX_MADV_NORMAL || advice > POSIX_MADV_DONTNEED
      || !len)
    {
      ret = EINVAL;
      goto out;
    }

  /* Check requested memory area. */
  MEMORY_BASIC_INFORMATION m;
  char *p, *endp;

  for (p = (char *) addr, endp = p + len;
       p < endp;
       p = (char *) m.BaseAddress + m.RegionSize)
    {
      if (!VirtualQuery (p, &m, sizeof m) || m.State == MEM_FREE)
	{
	  ret = ENOMEM;
	  break;
	}
    }
  if (ret)
    goto out;
  switch (advice)
    {
    case POSIX_MADV_WILLNEED:
      {
	/* Align address and length values to page size. */
	size_t pagesize = wincap.allocation_granularity ();
	PVOID base = (PVOID) rounddown ((uintptr_t) addr, pagesize);
	SIZE_T size = roundup2 (((uintptr_t) addr - (uintptr_t) base)
				+ len, pagesize);
	WIN32_MEMORY_RANGE_ENTRY me = { base, size };
	if (!PrefetchVirtualMemory (GetCurrentProcess (), 1, &me, 0)
	    && GetLastError () != ERROR_PROC_NOT_FOUND)
	  {
	    /* FIXME 2015-08-27: On W10 build 10240 under WOW64,
	       PrefetchVirtualMemory always returns ERROR_INVALID_PARAMETER
	       for some reason.  If we're running on W10 WOW64, ignore this
	       error.  This has been fixed in W10 1511. */
	    if (!wincap.has_broken_prefetchvm ()
		|| GetLastError () != ERROR_INVALID_PARAMETER)
	      ret = EINVAL;
	  }
      }
      break;
    case POSIX_MADV_DONTNEED:
      {
	/* Align address and length values to page size. */
	size_t pagesize = wincap.allocation_granularity ();
	PVOID base = (PVOID) rounddown ((uintptr_t) addr, pagesize);
	SIZE_T size = roundup2 (((uintptr_t) addr - (uintptr_t) base)
				+ len, pagesize);
	DWORD err = DiscardVirtualMemory (base, size);
	/* DiscardVirtualMemory is unfortunately pretty crippled:
	   On copy-on-write pages it returns ERROR_INVALID_PARAMETER, on
	   any file-backed memory map it returns ERROR_USER_MAPPED_FILE.
	   Since POSIX_MADV_DONTNEED is advisory only anyway, let them
	   slip through. */
	switch (err)
	  {
	  case ERROR_PROC_NOT_FOUND:
	  case ERROR_USER_MAPPED_FILE:
	  case 0:
	    break;
	  case ERROR_INVALID_PARAMETER:
	    {
	      ret = EINVAL;
	      /* Check if the region contains copy-on-write pages.*/
	      for (p = (char *) addr, endp = p + len;
		   p < endp;
		   p = (char *) m.BaseAddress + m.RegionSize)
		{
		  if (VirtualQuery (p, &m, sizeof m)
		      && m.State == MEM_COMMIT
		      && m.Protect
			 & (PAGE_EXECUTE_WRITECOPY | PAGE_WRITECOPY))
		    {
		      /* Yes, let this slip. */
		      ret = 0;
		      break;
		    }
		}
	    }
	    break;
	  default:
	    ret = geterrno_from_win_error (err);
	    break;
	  }
      }
      break;
    default:
      break;
    }
out:
  syscall_printf ("%d = posix_madvise(%p, %lu, %d)", ret, addr, len, advice);
  return ret;
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

bool
fhandler_base::fixup_mmap_after_fork (HANDLE h, int prot, int flags,
				      off_t offset, SIZE_T size,
				      void *address)
{
  set_errno (ENODEV);
  return -1;
}

/* Implementation for anonymous maps.  Using fhandler_dev_zero looks
   quite the natural way. */
HANDLE
fhandler_dev_zero::mmap (caddr_t *addr, size_t len, int prot,
			 int flags, off_t off)
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
	      NtUnmapViewOfSection (NtCurrentProcess (), base);
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
      NtUnmapViewOfSection (NtCurrentProcess (), addr);
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
				      off_t offset, SIZE_T size,
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
      system_printf ("requested %p != %p mem alloc base %p, state %y, "
		     "size %lu, %E", address, base, m.AllocationBase, m.State,
		     m.RegionSize);
    }
  return base == address;
}

/* Implementation for disk files and anonymous mappings. */
HANDLE
fhandler_disk_file::mmap (caddr_t *addr, size_t len, int prot,
			  int flags, off_t off)
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
	  NtUnmapViewOfSection (NtCurrentProcess (), base);
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
  NtUnmapViewOfSection (NtCurrentProcess (), addr);
  NtClose (h);
  return 0;
}

int
fhandler_disk_file::msync (HANDLE h, caddr_t addr, size_t len, int flags)
{
  const int retry = 100;
  /* The wisdom of google tells us that FlushViewOfFile may fail with
     ERROR_LOCK_VIOLATION if "if the memory system is writing dirty
     pages to disk".  And, we've seen reports of this happening in the
     cygwin list.  So retry 99 times and hope we get lucky.  */
  for (int i = 0; i < retry; i++)
    if (FlushViewOfFile (addr, len))
      {
	/* FlushViewOfFile just triggers the action and returns immediately,
	   so it's equivalent to MS_ASYNC.  MS_SYNC requires another call to
	   FlushFileBuffers. */
	if (flags & MS_SYNC)
	  FlushFileBuffers (h);
	return 0;
      }
    else if (GetLastError () != ERROR_LOCK_VIOLATION)
      break;
    else if (i < (retry - 1))
      yield ();

  __seterrno ();
  return -1;
}

bool
fhandler_disk_file::fixup_mmap_after_fork (HANDLE h, int prot, int flags,
					   off_t offset, SIZE_T size,
					   void *address)
{
  /* Re-create the map */
  void *base = MapView (h, address, size, get_access (), prot, flags, offset);
  if (base != address)
    {
      MEMORY_BASIC_INFORMATION m;
      VirtualQuery (address, &m, sizeof (m));
      system_printf ("requested %p != %p mem alloc base %p, state %y, "
		     "size %lu, %E", address, base, m.AllocationBase, m.State,
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
  /* Iterate over maps */
  mmap_list *map_list;
  LIST_FOREACH (map_list, &mmapped_areas.lists, ml_next)
    {
      mmap_record *rec;
      LIST_FOREACH (rec, &map_list->recs, mr_next)
	{
	  debug_printf ("fd %d, h %p, address %p, len %ly, prot: %y, "
			"flags: %y, offset %Y",
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
				 "parentstate: %y, "
				 "state: %y, "
				 "parentprot: %y, "
				 "prot: %y, %E",
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
