/* ntea.cc: code for manipulating NTEA information

   Copyright 1997, 1998, 2000, 2001, 2006 Red Hat, Inc.

   Written by Sergey S. Okhapkin (sos@prospect.com.ru)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <ntdef.h>
#include "security.h"
#include "ntdll.h"

/* Default to not using NTEA information */
bool allow_ntea;

/*
 * read_ea - read file's Extended Attribute.
 *
 * Parameters:
 *	file	- pointer to filename
 *	attrname- pointer to EA name (case insensitiv)
 *	attrbuf - pointer to buffer to store EA's value.
 *	len	- length of attrbuf.
 * Return value:
 *	0	- if file or attribute "attrname" not found.
 *	N	- number of bytes stored in attrbuf if success.
 *	-1	- attrbuf too small for EA value.
 */

int __stdcall
read_ea (HANDLE hdl, const char *file, const char *attrname, char *attrbuf,
	 int len)
{
  IO_STATUS_BLOCK io;

  /* Prepare buffer which receives the result. */
  ULONG flen = sizeof (FILE_FULL_EA_INFORMATION) + strlen (attrname)
	       + len + 1;
  PFILE_FULL_EA_INFORMATION fea = (PFILE_FULL_EA_INFORMATION) alloca (flen);
  /* Prepare buffer specifying the EA to search for. */
  ULONG glen = sizeof (FILE_GET_EA_INFORMATION) + strlen (attrname);
  PFILE_GET_EA_INFORMATION gea = (PFILE_GET_EA_INFORMATION) alloca (glen);
  gea->NextEntryOffset = 0;
  gea->EaNameLength = strlen (attrname);
  strcpy (gea->EaName, attrname);

  /* If no incoming hdl is given, the loop only runs once, trying to
     open the file and to query the EA.  If an incoming hdl is given,
     the loop runs twice, first trying to query with the given hdl.
     If this fails it tries to open the file and to query with that
     handle again. */
  HANDLE h = hdl;
  NTSTATUS status = STATUS_SUCCESS;
  int ret = 0;
  while (true)
    {
      if (!hdl && (h = CreateFile (file, FILE_READ_EA,
				   FILE_SHARE_READ | FILE_SHARE_WRITE,
				   &sec_none_nih, OPEN_EXISTING,
				   FILE_FLAG_BACKUP_SEMANTICS, NULL))
		  == INVALID_HANDLE_VALUE)
	{
	  debug_printf ("Opening %s for querying EA %s failed, %E",
			file, attrname);
	  goto out;
	}
      status = NtQueryEaFile (h, &io, fea, flen, FALSE, gea, glen, NULL, TRUE);
      if (NT_SUCCESS (status) || !hdl)
	break;
      debug_printf ("1. chance, %x = NtQueryEaFile (%s, %s), Win32 error %d",
		    status, file, attrname, RtlNtStatusToDosError (status));
      hdl = NULL;
    }
  if (!hdl)
    CloseHandle (h);
  if (!NT_SUCCESS (status))
    {
      ret = -1;
      debug_printf ("%x = NtQueryEaFile (%s, %s), Win32 error %d",
		    status, file, attrname, RtlNtStatusToDosError (status));
    }
  if (!fea->EaValueLength)
    ret = 0;
  else
    {
      memcpy (attrbuf, fea->EaName + fea->EaNameLength + 1,
	      fea->EaValueLength);
      ret = fea->EaValueLength;
    }

out:
  debug_printf ("%d = read_ea (%x, %s, %s, %x, %d)", ret, hdl, file, attrname,
		attrbuf, len);
  return ret;
}

/*
 * write_ea - write file's Extended Attribute.
 *
 * Parameters:
 *	file	- pointer to filename
 *	attrname- pointer to EA name (case insensitiv)
 *	attrbuf	- pointer to buffer with EA value.
 *	len	- length of attrbuf.
 * Return value:
 *	true if success, false otherwice.
 * Note: if len=0 given EA will be deleted.
 */

BOOL __stdcall
write_ea (HANDLE hdl, const char *file, const char *attrname,
	  const char *attrbuf, int len)
{
  IO_STATUS_BLOCK io;

  /* Prepare buffer specifying the EA to write back. */
  ULONG flen = sizeof (FILE_FULL_EA_INFORMATION) + strlen (attrname)
	       + len + 1;
  PFILE_FULL_EA_INFORMATION fea = (PFILE_FULL_EA_INFORMATION) alloca (flen);
  fea->NextEntryOffset = 0;
  fea->Flags = 0;
  fea->EaNameLength = strlen (attrname);
  fea->EaValueLength = len;
  strcpy (fea->EaName, attrname);
  memcpy (fea->EaName + fea->EaNameLength + 1, attrbuf, len);

  /* If no incoming hdl is given, the loop only runs once, trying to
     open the file and to set the EA.  If an incoming hdl is given,
     the loop runs twice, first trying to set the EA with the given hdl.
     If this fails it tries to open the file and to set the EA with that
     handle again. */
  HANDLE h = hdl;
  NTSTATUS status = STATUS_SUCCESS;
  bool ret = false;
  while (true)
    {
      if (!hdl && (h = CreateFile (file, FILE_READ_EA,
				   FILE_SHARE_READ | FILE_SHARE_WRITE,
				   &sec_none_nih, OPEN_EXISTING,
				   FILE_FLAG_BACKUP_SEMANTICS, NULL))
		  == INVALID_HANDLE_VALUE)
	{
	  debug_printf ("Opening %s for setting EA %s failed, %E",
			file, attrname);
	  goto out;
	}
      status = NtSetEaFile (h, &io, fea, flen);
      if (NT_SUCCESS (status) || !hdl)
	break;
      debug_printf ("1. chance, %x = NtQueryEaFile (%s, %s), Win32 error %d",
		    status, file, attrname, RtlNtStatusToDosError (status));
      hdl = NULL;
    }
  if (!hdl)
    CloseHandle (h);
  if (!NT_SUCCESS (status))
    debug_printf ("%x = NtQueryEaFile (%s, %s), Win32 error %d",
		  status, file, attrname, RtlNtStatusToDosError (status));
  else
    ret = true;

out:
  debug_printf ("%d = write_ea (%x, %s, %s, %x, %d)", ret, hdl, file, attrname,
		attrbuf, len);
  return ret;
}
