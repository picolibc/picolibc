/* fhandler_mem.cc.  See fhandler.h for a description of the fhandler classes.

   Copyright 1999, 2000 Cygnus Solutions.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <sys/termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ntdef.h>

#include "cygheap.h"
#include "cygerrno.h"
#include "fhandler.h"
#include "path.h"

static int inited = FALSE;
NTSTATUS (__stdcall *NtMapViewOfSection)(HANDLE,HANDLE,PVOID*,ULONG,ULONG,
                                         PLARGE_INTEGER,PULONG,SECTION_INHERIT,
                                         ULONG,ULONG);
NTSTATUS (__stdcall *NtOpenSection)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES);
NTSTATUS (__stdcall *NtUnmapViewOfSection)(HANDLE,PVOID);
VOID (__stdcall *RtlInitUnicodeString)(PUNICODE_STRING,PCWSTR);
ULONG (__stdcall *RtlNtStatusToDosError) (NTSTATUS);

int
load_ntdll_funcs ()
{
  int ret = 0;

  if (os_being_run != winNT)
    {
      set_errno (ENOENT);
      debug_printf ("/dev/mem is accessible under NT/W2K only");
      return 0;
    }

  HMODULE ntdll = GetModuleHandle ("ntdll.dll");

  if (inited)
    {
      debug_printf ("function pointers already inited");
      return 1;
    }

  if (!ntdll)
    {
      __seterrno ();
      goto out;
    }

  if (!(NtMapViewOfSection = (NTSTATUS (__stdcall *)(HANDLE,HANDLE,PVOID*,ULONG,
                                                     ULONG,PLARGE_INTEGER,
                                                     PULONG,SECTION_INHERIT,
                                                     ULONG,ULONG))
                             GetProcAddress (ntdll, "NtMapViewOfSection")))
    {
      __seterrno ();
      goto out;
    }

  if (!(NtOpenSection = (NTSTATUS (__stdcall *)(PHANDLE,ACCESS_MASK,
                                                POBJECT_ATTRIBUTES))
                        GetProcAddress (ntdll, "NtOpenSection")))
    {
      __seterrno ();
      goto out;
    }

  if (!(NtUnmapViewOfSection = (NTSTATUS (__stdcall *)(HANDLE,PVOID))
                               GetProcAddress (ntdll, "NtUnmapViewOfSection")))
    {
      __seterrno ();
      goto out;
    }

  if (!(RtlInitUnicodeString = (VOID (__stdcall *)(PUNICODE_STRING,PCWSTR))
                               GetProcAddress (ntdll, "RtlInitUnicodeString")))
    {
      __seterrno ();
      goto out;
    }

  if (!(RtlNtStatusToDosError = (ULONG (__stdcall *)(NTSTATUS))
                               GetProcAddress (ntdll, "RtlNtStatusToDosError")))
    {
      __seterrno ();
      goto out;
    }

  inited = TRUE;
  ret = 1;

out:
  debug_printf ("%d = load_ntdll_funcs()", ret);
  return ret;
}

/**********************************************************************/
/* fhandler_dev_mem */

fhandler_dev_mem::fhandler_dev_mem (const char *name, int)
: fhandler_base (FH_MEM, name),
  pos(0UL)
{
}

fhandler_dev_mem::~fhandler_dev_mem (void)
{
}

int
fhandler_dev_mem::open (const char *, int flags, mode_t)
{
  if (!load_ntdll_funcs ())
    return 0;

  UNICODE_STRING memstr;
  RtlInitUnicodeString (&memstr, L"\\device\\physicalmemory");

  OBJECT_ATTRIBUTES attr;
  InitializeObjectAttributes(&attr, &memstr, OBJ_CASE_INSENSITIVE, NULL, NULL);

  if ((flags & (O_RDONLY | O_WRONLY | O_RDWR)) == O_RDONLY)
    set_access (SECTION_MAP_READ);
  else if ((flags & (O_RDONLY | O_WRONLY | O_RDWR)) == O_WRONLY)
    set_access (SECTION_MAP_WRITE);
  else
    set_access (SECTION_MAP_READ | SECTION_MAP_WRITE);

  HANDLE mem;
  NTSTATUS ret = NtOpenSection (&mem, get_access (), &attr);
  if (!NT_SUCCESS(ret))
    {
      __seterrno_from_win_error (RtlNtStatusToDosError (ret));
      set_io_handle (NULL);
      return 0;
    }

  set_io_handle (mem);
  return 1;
}

int
fhandler_dev_mem::write (const void *ptr, size_t len)
{
  set_errno (ENOSYS);
  return -1;
}

int
fhandler_dev_mem::read (void *ptr, size_t ulen)
{
  if (!ulen)
    return 0;

  PHYSICAL_ADDRESS phys;
  NTSTATUS ret;
  void *viewmem = NULL;
  DWORD len = ulen + 4095;

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
      __seterrno_from_win_error (RtlNtStatusToDosError (ret));
      return -1;
    }

  memcpy (ptr, (char *) viewmem + (pos - phys.QuadPart), ulen);

  if (!NT_SUCCESS(ret = NtUnmapViewOfSection (INVALID_HANDLE_VALUE, viewmem)))
    {
      __seterrno_from_win_error (RtlNtStatusToDosError (ret));
      return -1;
    }

  pos += ulen;

  return ulen;
}

int
fhandler_dev_mem::close (void)
{
  return fhandler_base::close ();
}

off_t
fhandler_dev_mem::lseek (off_t offset, int whence)
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
      pos = 0;
      pos -= offset;
      break;
    
    default:
      set_errno (EINVAL);
      return (off_t) -1;
    }

  return pos;
}

int
fhandler_dev_mem::fstat (struct stat *buf)
{
  if (!buf)
    {
      set_errno (EINVAL);
      return -1;
    }

  memset (buf, 0, sizeof *buf);
  buf->st_mode = S_IFCHR;
  if (os_being_run != winNT)
    buf->st_mode |= S_IRUSR | S_IWUSR |
		    S_IRGRP | S_IWGRP |
		    S_IROTH | S_IWOTH;
  buf->st_nlink = 1;
  buf->st_blksize = 4096;
  buf->st_dev = buf->st_rdev = get_device () << 8;

  return 0;
}

int
fhandler_dev_mem::dup (fhandler_base *child)
{
  set_errno (ENOSYS);
  return -1;
}

void
fhandler_dev_mem::dump ()
{
  paranoid_printf("here, fhandler_dev_mem");
}
