/* fhandler_mem.cc.  See fhandler.h for a description of the fhandler classes.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <unistd.h>

#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "ntdll.h"

/**********************************************************************/
/* fhandler_dev_mem */

fhandler_dev_mem::fhandler_dev_mem ()
  : fhandler_base ()
{
}

fhandler_dev_mem::~fhandler_dev_mem ()
{
}

int
fhandler_dev_mem::open (int flags, mode_t)
{
  if (!wincap.has_physical_mem_access ())
    {
      set_errno (ENOENT);
      debug_printf ("%s is accessible under NT4/W2K/XP only", dev ().name);
      return 0;
    }

  if (dev () == FH_MEM) /* /dev/mem */
    {
      NTSTATUS ret;
      SYSTEM_BASIC_INFORMATION sbi;
      if ((ret = NtQuerySystemInformation (SystemBasicInformation, (PVOID) &sbi,
					   sizeof sbi, NULL)) != STATUS_SUCCESS)
	{
	  __seterrno_from_nt_status (ret);
	  debug_printf("NtQuerySystemInformation: ret %d, Dos(ret) %E", ret);
	  mem_size = 0;
	}
      else
	mem_size = sbi.PhysicalPageSize * sbi.NumberOfPhysicalPages;
      debug_printf ("MemSize: %d MB", mem_size >> 20);
    }
  else if (dev () == FH_KMEM) /* /dev/kmem - Not yet supported */
    {
      mem_size = 0;
      debug_printf ("KMemSize: %d MB", mem_size >> 20);
    }
  else if (dev () == FH_PORT) /* /dev/port == First 64K of /dev/mem */
    {
      mem_size = 65536;
      debug_printf ("PortSize: 64 KB");
    }
  else
    {
      mem_size = 0;
      debug_printf ("Illegal minor number!!!");
    }

  /* Check for illegal flags. */
  if (flags & (O_APPEND | O_TRUNC | O_EXCL))
    {
      set_errno (EINVAL);
      return 0;
    }

  UNICODE_STRING memstr;
  RtlInitUnicodeString (&memstr, L"\\device\\physicalmemory");

  OBJECT_ATTRIBUTES attr;
  InitializeObjectAttributes (&attr, &memstr,
			      OBJ_CASE_INSENSITIVE | OBJ_INHERIT,
			      NULL, NULL);

  ACCESS_MASK section_access;
  if ((flags & O_ACCMODE) == O_RDONLY)
    {
      set_access (GENERIC_READ);
      section_access = SECTION_MAP_READ;
    }
  else if ((flags & O_ACCMODE) == O_WRONLY)
    {
      set_access (GENERIC_WRITE);
      section_access = SECTION_MAP_READ | SECTION_MAP_WRITE;
    }
  else
    {
      set_access (GENERIC_READ | GENERIC_WRITE);
      section_access = SECTION_MAP_READ | SECTION_MAP_WRITE;
    }

  HANDLE mem;
  NTSTATUS ret = NtOpenSection (&mem, section_access, &attr);
  if (!NT_SUCCESS (ret))
    {
      __seterrno_from_nt_status (ret);
      set_io_handle (NULL);
      return 0;
    }

  set_io_handle (mem);
  set_open_status ();
  return 1;
}

int
fhandler_dev_mem::write (const void *ptr, size_t ulen)
{
  if (!ulen || pos >= mem_size)
    return 0;

  if (!(get_access () & GENERIC_WRITE))
    {
      set_errno (EINVAL);
      return -1;
    }

  if (pos + ulen > mem_size)
    ulen = mem_size - pos;

  PHYSICAL_ADDRESS phys;
  NTSTATUS ret;
  void *viewmem = NULL;
  DWORD len = ulen + getsystempagesize () - 1;

  phys.QuadPart = (ULONGLONG) pos;
  if ((ret = NtMapViewOfSection (get_handle (),
				 INVALID_HANDLE_VALUE,
				 &viewmem,
				 0L,
				 len,
				 &phys,
				 &len,
				 ViewShare,
				 0,
				 PAGE_READONLY)) != STATUS_SUCCESS)
    {
      __seterrno_from_nt_status (ret);
      return -1;
    }

  memcpy ((char *) viewmem + (pos - phys.QuadPart), ptr, ulen);

  if (!NT_SUCCESS (ret = NtUnmapViewOfSection (INVALID_HANDLE_VALUE, viewmem)))
    {
      __seterrno_from_nt_status (ret);
      return -1;
    }

  pos += ulen;
  return ulen;
}

void __stdcall
fhandler_dev_mem::read (void *ptr, size_t& ulen)
{
  if (!ulen || pos >= mem_size)
    {
      ulen = 0;
      return;
    }

  if (!(get_access () & GENERIC_READ))
    {
      set_errno (EINVAL);
      ulen = (size_t) -1;
      return;
    }

  if (pos + ulen > mem_size)
    ulen = mem_size - pos;

  PHYSICAL_ADDRESS phys;
  NTSTATUS ret;
  void *viewmem = NULL;
  DWORD len = ulen + getsystempagesize () - 1;

  phys.QuadPart = (ULONGLONG) pos;
  if ((ret = NtMapViewOfSection (get_handle (),
				 INVALID_HANDLE_VALUE,
				 &viewmem,
				 0L,
				 len,
				 &phys,
				 &len,
				 ViewShare,
				 0,
				 PAGE_READONLY)) != STATUS_SUCCESS)
    {
      __seterrno_from_nt_status (ret);
      ulen = (size_t) -1;
      return;
    }

  memcpy (ptr, (char *) viewmem + (pos - phys.QuadPart), ulen);

  if (!NT_SUCCESS (ret = NtUnmapViewOfSection (INVALID_HANDLE_VALUE, viewmem)))
    {
      __seterrno_from_nt_status (ret);
      ulen = (size_t) -1;
      return;
    }

  pos += ulen;
}

_off64_t
fhandler_dev_mem::lseek (_off64_t offset, int whence)
{
  switch (whence)
    {
    case SEEK_SET:
      pos = offset;
      break;

    case SEEK_CUR:
      pos += offset;
      break;

    case SEEK_END:
      pos = mem_size;
      pos += offset;
      break;

    default:
      set_errno (EINVAL);
      return ILLEGAL_SEEK;
    }

  if (pos > mem_size)
    {
      set_errno (EINVAL);
      return ILLEGAL_SEEK;
    }

  return pos;
}

int
fhandler_dev_mem::fstat (struct __stat64 *buf)
{
  fhandler_base::fstat (buf);
  buf->st_blksize = getsystempagesize ();
  if (is_auto_device ())
    {
      buf->st_mode = S_IFCHR;
      if (wincap.has_physical_mem_access ())
	buf->st_mode |= S_IRUSR | S_IWUSR |
			S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH;
    }

  return 0;
}

int
fhandler_dev_mem::dup (fhandler_base *child)
{
  int ret = fhandler_base::dup (child);

  if (! ret)
    {
      fhandler_dev_mem *fhc = (fhandler_dev_mem *) child;

      fhc->mem_size = mem_size;
      fhc->pos = pos;
    }
  return ret;
}
