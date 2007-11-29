/* mmap.cc

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
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

  /* Ignore EXECUTE permission on 9x. */
  if ((openflags & GENERIC_EXECUTE)
      && wincap.virtual_protect_works_on_shared_pages ())
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

  /* Ignore EXECUTE permission on 9x. */
  if ((prot & PROT_EXEC)
      && wincap.virtual_protect_works_on_shared_pages ())
    ret <<= 4;

  return ret;
}

/* Generate Windows access flags from mmap prot and flag values.
   Only used on 9x.  PROT_EXEC not supported here since it's not
   necessary. */
static inline DWORD
gen_access (DWORD openflags, int flags)
{
  DWORD ret = FILE_MAP_READ;
  if (priv (flags))
    ret = FILE_MAP_COPY;
  else if (openflags & GENERIC_WRITE)
    ret = priv (flags) ? FILE_MAP_COPY : FILE_MAP_WRITE;
  return ret;
}

/* OS specific wrapper functions for map/section functions. */
static BOOL
VirtualProt9x (PVOID addr, SIZE_T len, DWORD prot, PDWORD oldprot)
{
  if (addr >= (caddr_t)0x80000000 && addr <= (caddr_t)0xBFFFFFFF)
    return TRUE; /* FAKEALARM! */
  return VirtualProtect (addr, len, prot, oldprot);
}

static BOOL
VirtualProtNT (PVOID addr, SIZE_T len, DWORD prot, PDWORD oldprot)
{
  return VirtualProtect (addr, len, prot, oldprot);
}

static BOOL
VirtualProtEx9x (HANDLE parent, PVOID addr, SIZE_T len, DWORD prot,
		 PDWORD oldprot)
{
  if (addr >= (caddr_t)0x80000000 && addr <= (caddr_t)0xBFFFFFFF)
    return TRUE; /* FAKEALARM! */
  return VirtualProtectEx (parent, addr, len, prot, oldprot);
}
static BOOL
VirtualProtExNT (HANDLE parent, PVOID addr, SIZE_T len, DWORD prot,
		 PDWORD oldprot)
{
  return VirtualProtectEx (parent, addr, len, prot, oldprot);
}

/* This allows to stay lazy about VirtualProtect usage in subsequent code. */
#define VirtualProtect(a,l,p,o) (mmap_func->VirtualProt((a),(l),(p),(o)))
#define VirtualProtectEx(h,a,l,p,o) (mmap_func->VirtualProtEx((h),(a),(l),(p),(o)))

static HANDLE
CreateMapping9x (HANDLE fhdl, size_t len, _off64_t off, DWORD openflags,
		 int prot, int flags, const char *name)
{
  HANDLE h;
  DWORD high, low;

  DWORD protect = gen_create_protect (openflags, flags);

  /* copy-on-write doesn't work properly on 9x with real files.  While the
     changes are not propagated to the file, they are visible to other
     processes sharing the same file mapping object.  Workaround: Don't
     use named file mapping.  That should work since sharing file
     mappings only works reliable using named file mapping on 9x.

     On 9x/ME try first to open the mapping by name when opening a
     shared file object. This is needed since 9x/ME only shares objects
     between processes by name. What a mess... */

  if (fhdl != INVALID_HANDLE_VALUE && !priv (flags))
    {
      /* Grrr, the whole stuff is just needed to try to get a reliable
	 mapping of the same file. Even that uprising isn't bullet
	 proof but it does it's best... */
      char namebuf[CYG_MAX_PATH];
      cygwin_conv_to_full_posix_path (name, namebuf);
      for (int i = strlen (namebuf) - 1; i >= 0; --i)
	namebuf[i] = cyg_tolower (namebuf [i]);

      debug_printf ("named sharing");
      DWORD access = gen_access (openflags, flags);
      /* Different access modes result in incompatible mappings.  So we
	 create different maps per access mode by using different names. */
      switch (access)
	{
	  case FILE_MAP_READ:
	    namebuf[0] = 'R';
	    break;
	  case FILE_MAP_WRITE:
	    namebuf[0] = 'W';
	    break;
	  case FILE_MAP_COPY:
	    namebuf[0] = 'C';
	    break;
	}
      if (!(h = OpenFileMapping (access, TRUE, namebuf)))
	h = CreateFileMapping (fhdl, &sec_none, protect, 0, 0, namebuf);
    }
  else if (fhdl == INVALID_HANDLE_VALUE)
    {
      /* Standard anonymous mapping needs non-zero len. */
      h = CreateFileMapping (fhdl, &sec_none, protect, 0, len, NULL);
    }
  else if (autogrow (flags))
    {
      high = (off + len) >> 32;
      low = (off + len) & UINT32_MAX;
      /* Auto-grow only works if the protection is PAGE_READWRITE.  So,
	 first we call CreateFileMapping with PAGE_READWRITE, then, if the
	 requested protection is different, we close the mapping and
	 reopen it again with the correct protection, if auto-grow worked. */
      h = CreateFileMapping (fhdl, &sec_none, PAGE_READWRITE,
			     high, low, NULL);
      if (h && protect != PAGE_READWRITE)
	{
	  CloseHandle (h);
	  h = CreateFileMapping (fhdl, &sec_none, protect,
				 high, low, NULL);
	}
    }
  else
    {
      /* Zero len creates mapping for whole file. */
      h = CreateFileMapping (fhdl, &sec_none, protect, 0, 0, NULL);
    }
  return h;
}

static HANDLE
CreateMappingNT (HANDLE fhdl, size_t len, _off64_t off, DWORD openflags,
		 int prot, int flags, const char *)
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
	  CloseHandle (h);
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
MapView9x (HANDLE h, void *addr, size_t len, DWORD openflags,
	   int prot, int flags, _off64_t off)
{
  DWORD high = off >> 32;
  DWORD low = off & UINT32_MAX;
  DWORD access = gen_access (openflags, flags);
  void *base;

  /* Try mapping using the given address first, even if it's NULL.
     If it failed, and addr was not NULL and flags is not MAP_FIXED,
     try again with NULL address.
     
     Note: Retrying the mapping might be unnecessary, now that mmap64 checks
	   for a valid memory area first. */
  if (!addr)
    base = MapViewOfFile (h, access, high, low, len);
  else
    {
      base = MapViewOfFileEx (h, access, high, low, len, addr);
      if (!base && !fixed (flags))
	base = MapViewOfFile (h, access, high, low, len);
    }
  debug_printf ("%x = MapViewOfFileEx (h:%x, access:%x, 0, off:%D, "
		"len:%u, addr:%x)", base, h, access, off, len, addr);
  return base;
}

void *
MapViewNT (HANDLE h, void *addr, size_t len, DWORD openflags,
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

struct mmap_func_t
{
  HANDLE (*CreateMapping)(HANDLE, size_t, _off64_t, DWORD, int, int,
			  const char *);
  void * (*MapView)(HANDLE, void *, size_t, DWORD, int, int, _off64_t);
  BOOL	 (*VirtualProt)(PVOID, SIZE_T, DWORD, PDWORD);
  BOOL	 (*VirtualProtEx)(HANDLE, PVOID, SIZE_T, DWORD, PDWORD);
};

mmap_func_t mmap_funcs_9x =
{
  CreateMapping9x,
  MapView9x,
  VirtualProt9x,
  VirtualProtEx9x
};

mmap_func_t mmap_funcs_nt =
{
  CreateMappingNT,
  MapViewNT,
  VirtualProtNT,
  VirtualProtExNT
};

mmap_func_t *mmap_func;

void
mmap_init ()
{
  mmap_func = wincap.is_winnt () ? &mmap_funcs_nt : &mmap_funcs_9x;
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
    DWORD gen_access () const
      { return ::gen_access (get_openflags (), get_flags ()); }
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
    void set (int nfd);
    mmap_record *add_record (mmap_record r);
    bool del_record (mmap_record *rec);
    caddr_t try_map (void *addr, size_t len, int flags, _off64_t off);
};

class mmap_areas
{
  public:
    LIST_HEAD (, mmap_list) lists;

    mmap_list *get_list_by_fd (int fd);
    mmap_list *add_list (int fd);
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
mmap_list::set (int nfd)
{
  fd = nfd;
  if (!anonymous ())
    {
      /* The fd isn't sufficient since it could already be the fd of another
	 file.  So we use the inode number as evaluated by fstat to identify
	 the file. */
      struct stat st;
      fstat (nfd, &st);
      hash = st.st_ino;
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
mmap_areas::get_list_by_fd (int fd)
{
  mmap_list *ml;
  LIST_FOREACH (ml, &lists, ml_next)
    {
      if (fd == -1 && ml->anonymous ())
	return ml;
      /* The fd isn't sufficient since it could already be the fd of another
	 file.  So we use the inode number as evaluated by fstat to identify
	 the file. */
      struct stat st;
      if (fd != -1 && !fstat (fd, &st) && ml->get_hash () == st.st_ino)
	return ml;
    }
  return 0;
}

mmap_list *
mmap_areas::add_list (int fd)
{
  mmap_list *ml = (mmap_list *) cmalloc (HEAP_MMAP, sizeof (mmap_list));
  if (!ml)
    return NULL;
  ml->set (fd);
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
   violation has happened.  We have two cases to check here.
   
   First, is it an address within "attached" mmap pages (indicated by
   the __PROT_ATTACH protection, see there)?  In this case the function
   returns 1 and the exception_handler raises SIGBUS, as demanded by the
   memory protection extension described in SUSv3 (see the mmap man
   page).
   
   Second, check if the address is within "noreserve" mmap pages
   (indicated by MAP_NORESERVE flag).  If so, the function calls
   VirtualAlloc to commit the page and returns 2.  The exception handler
   then just returns with 0 and the affected application retries the
   failing memory access.  If VirtualAlloc fails, the function returns
   1, so that the exception handler raises a SIGBUS, as described in the
   MAP_NORESERVE man pages for Linux and Solaris.
   
   In any other case 0 is returned and a normal SIGSEGV is raised. */
int
mmap_is_attached_or_noreserve_page (ULONG_PTR addr)
{
  mmap_list *map_list;
  mmap_record *rec;
  caddr_t u_addr;
  DWORD u_len;
  DWORD pagesize = getsystempagesize ();
  int ret;

  addr = rounddown (addr, pagesize);
  LIST_LOCK ();
  if (!(map_list = mmapped_areas.get_list_by_fd (-1)))
    ret = 0;
  else
    {
      LIST_FOREACH (rec, &map_list->recs, mr_next)
	if (rec->match ((caddr_t) addr, pagesize, u_addr, u_len))
	  break;
      if (!rec)
	ret = 0;
      else if (rec->attached ())
	ret = 1;
      else if (!rec->noreserve ())
	ret = 0;
      else
	ret = VirtualAlloc ((void *)addr, pagesize, MEM_COMMIT,
			    rec->gen_protect ())
	      ? 2 : 1;
    }
  LIST_UNLOCK ();
  return ret;
}

static caddr_t
mmap_worker (fhandler_base *fh, caddr_t base, size_t len, int prot, int flags,
	     int fd, _off64_t off)
{
  mmap_list *map_list;
  HANDLE h = fh->mmap (&base, len, prot, flags, off);
  if (h == INVALID_HANDLE_VALUE)
    return NULL;
  if (!(map_list = mmapped_areas.get_list_by_fd (fd))
      && !(map_list = mmapped_areas.add_list (fd)))
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

  DWORD pagesize = getpagesize ();
  DWORD checkpagesize;

  fh_anonymous.set_io_handle (INVALID_HANDLE_VALUE);
  fh_anonymous.set_access (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);

  /* EINVAL error conditions.  Note that the addr%pagesize test is deferred
     to workaround a serious alignment problem in Windows 98.  */
  if (off % pagesize
      || ((prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC)))
      || ((flags & MAP_TYPE) != MAP_SHARED
	  && (flags & MAP_TYPE) != MAP_PRIVATE)
#if 0
      || (fixed (flags) && ((uintptr_t) addr % pagesize))
#endif
      || !len)
    {
      set_errno (EINVAL);
      goto out;
    }

  /* There's a serious alignment problem in Windows 98.  MapViewOfFile
     sometimes returns addresses which are page aligned instead of
     granularity aligned.  OTOH, it's not possible to force such an
     address using MapViewOfFileEx.  So what we do here to let it work
     at least most of the time is, allow 4K aligned addresses in 98,
     to enable remapping of formerly mapped pages.  If no matching
     free pages exist, check addr again, this time for the real alignment. */
  checkpagesize = wincap.has_mmap_alignment_bug () ?
    		  getsystempagesize () : pagesize;
  if (fixed (flags) && ((uintptr_t) addr % checkpagesize))
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

      /* On 9x you can't create mappings with PAGE_WRITECOPY protection if
	 the file isn't explicitely opened with WRITE access. */
      if (!wincap.is_winnt () && priv (flags)
	  && !(fh->get_access () & GENERIC_WRITE))
	{
	  HANDLE h = CreateFile (fh->get_win32_name (),
				 fh->get_access () | GENERIC_WRITE,
				 wincap.shared (), &sec_none_nih,
				 OPEN_EXISTING, 0, NULL);
	  if (h == INVALID_HANDLE_VALUE)
	    {
	      set_errno (EACCES);
	      goto out;
	    }
	  fh_disk_file = new (alloca (sizeof *fh_disk_file)) fhandler_disk_file;
	  path_conv pc;
	  pc.set_name (fh->get_win32_name (), "");
	  fh_disk_file->set_name (pc);
	  fh_disk_file->set_io_handle (h);
	  fh_disk_file->set_access (fh->get_access () | GENERIC_WRITE);
	  fh = fh_disk_file;
	}

      /* On NT you can't create mappings with PAGE_EXECUTE protection if
	 the file isn't explicitely opened with EXECUTE access. */
      if (wincap.is_winnt ())
	{
	  HANDLE h = CreateFile (fh->get_win32_name (),
				 fh->get_access () | GENERIC_EXECUTE,
				 wincap.shared (), &sec_none_nih,
				 OPEN_EXISTING, 0, NULL);
	  if (h != INVALID_HANDLE_VALUE)
	    {
	      fh_disk_file = new (alloca (sizeof *fh_disk_file)) fhandler_disk_file;
	      path_conv pc;
	      pc.set_name (fh->get_win32_name (), "");
	      fh_disk_file->set_name (pc);
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
	}

      DWORD high;
      DWORD low = GetFileSize (fh->get_handle (), &high);
      _off64_t fsiz = ((_off64_t)high << 32) + low;

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
      /* On NT systems we're creating the pages beyond EOF as reserved,
	 anonymous pages.  That's not possible on 9x for two reasons.
	 It neither allows to create reserved pages in the shared memory
	 area, nor does it allow to create page aligend mappings (in
	 contrast to granularity aligned mappings).

	 Note that this isn't done in WOW64 environments since apparently
	 WOW64 does not support the AT_ROUND_TO_PAGE flag which is required
	 to get this right.  Too bad. */
      if (wincap.virtual_protect_works_on_shared_pages ()
	  && !wincap.is_wow64 ()
	  && ((len > fsiz && !autogrow (flags))
	      || len < pagesize))
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
  map_list = mmapped_areas.get_list_by_fd (fd);

  /* Test if an existing anonymous mapping can be recycled. */
  if (map_list && anonymous (flags))
    {
      caddr_t tried = map_list->try_map (addr, len, flags, off);
      /* try_map returns NULL if no map matched, otherwise it returns
	 a valid address, of MAP_FAILED in case of a fatal error. */
      if (tried)
	{
	  ret = tried;
	  goto out_with_unlock;
	}
    }

  /* Deferred alignment test, see above. */
  if (wincap.has_mmap_alignment_bug ()
      && fixed (flags) && ((uintptr_t) addr % pagesize))
    {
      set_errno (EINVAL);
      goto out_with_unlock;
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

  base = mmap_worker (fh, (caddr_t) addr, len, prot, flags, fd, off);
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
	 so don't even try on 9x and in WOW64.  This is accomplished by not
	 setting orig_len on 9x and in WOW64 above. */
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
	      at_base = mmap_worker (&fh_anonymous, at_base, valid_page_len,
				     prot, flags, -1, 0);
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
	      at_base = mmap_worker (&fh_anonymous, at_base, sigbus_page_len,
				     prot, flags, -1, 0);
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
    CloseHandle (fh_disk_file->get_handle ());

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
  /* See comment in mmap64 for a description. */
  size_t pagesize = wincap.has_mmap_alignment_bug () ?
		    getsystempagesize () : getpagesize ();
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

  /* See comment in mmap64 for a description. */
  size_t pagesize = wincap.has_mmap_alignment_bug () ?
		    getsystempagesize () : getpagesize ();
  if (((uintptr_t) addr % pagesize)
      || (flags & ~(MS_ASYNC | MS_SYNC | MS_INVALIDATE))
      || (flags & (MS_ASYNC | MS_SYNC) == (MS_ASYNC | MS_SYNC)))
    {
      set_errno (EINVAL);
      goto out;
    }
#if 0 /* If I only knew why I did that... */
  len = roundup2 (len, pagesize);
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
  syscall_printf ("%d = msync()", ret);
  LIST_UNLOCK ();
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
  size_t pagesize = wincap.has_mmap_alignment_bug () ?
		    getsystempagesize () : getpagesize ();
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
  if (!wincap.has_working_virtual_lock ())
    return 0;

  int ret = -1;

  /* Instead of using VirtualLock, which does not guarantee that the pages
     aren't swapped out when the process is inactive, we're using
     ZwLockVirtualMemory with the LOCK_VM_IN_RAM flag to do what mlock on
     POSIX systems does.  On NT, this requires SeLockMemoryPrivilege,
     which is given only to SYSTEM by default. */

  push_thread_privilege (SE_LOCK_MEMORY_PRIV, true);

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
  if (!wincap.has_working_virtual_lock ())
    return 0;

  int ret = -1;

  push_thread_privilege (SE_LOCK_MEMORY_PRIV, true);

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
      h = mmap_func->CreateMapping (get_handle (), len, off, get_access (),
				    prot, flags, get_win32_name ());
      if (!h)
	{
	  __seterrno ();
	  debug_printf ("CreateMapping failed with %E");
	  return INVALID_HANDLE_VALUE;
	}

      base = mmap_func->MapView (h, *addr, len, get_access(), prot, flags, off);
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
	  CloseHandle (h);
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
      CloseHandle (h);
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
    base = mmap_func->MapView (h, address, size, get_access (),
			       prot, flags, offset);
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
  HANDLE h = mmap_func->CreateMapping (get_handle (), len, off, get_access (),
				       prot, flags, get_win32_name ());
  if (!h)
    {
      __seterrno ();
      debug_printf ("CreateMapping failed with %E");
      return INVALID_HANDLE_VALUE;
    }

  void *base = mmap_func->MapView (h, *addr, len, get_access (),
				   prot, flags, off);
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

bool
fhandler_disk_file::fixup_mmap_after_fork (HANDLE h, int prot, int flags,
					   _off64_t offset, DWORD size,
					   void *address)
{
  /* Re-create the map */
  void *base = mmap_func->MapView (h, address, size, get_access (),
				   prot, flags, offset);
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

  void *base = MapViewNT (h, *addr, len, get_access (),
			  prot, flags | MAP_ANONYMOUS, off);
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
      CloseHandle (h);
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
  CloseHandle (h);
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
  void *base = MapViewNT (h, address, size, get_access (), prot,
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
