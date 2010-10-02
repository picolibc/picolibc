/* nfs.cc

   Copyright 2008, 2010 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "sys/fcntl.h"
#include "nfs.h"
#include "ntdll.h"

struct nfs_aol_ffei_t nfs_aol_ffei = { 0, 0, sizeof (NFS_ACT_ON_LINK) - 1, 0,
				       NFS_ACT_ON_LINK };

uint32_t nfs_type_mapping[] = { 0, S_IFREG, S_IFDIR, S_IFBLK,
				S_IFCHR, S_IFLNK, S_IFSOCK, S_IFIFO };

NTSTATUS
nfs_fetch_fattr3 (HANDLE h, fattr3 *fattr_buf)
{
  struct {
    FILE_FULL_EA_INFORMATION ffei;
    char buf[sizeof (NFS_V3_ATTR) + sizeof (fattr3)];
  } ffei_buf;
  struct {
     FILE_GET_EA_INFORMATION fgei;
     char buf[sizeof (NFS_V3_ATTR)];
  } fgei_buf;
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  fgei_buf.fgei.NextEntryOffset = 0;
  fgei_buf.fgei.EaNameLength = sizeof (NFS_V3_ATTR) - 1;
  stpcpy (fgei_buf.fgei.EaName, NFS_V3_ATTR);
  status = NtQueryEaFile (h, &io, &ffei_buf.ffei, sizeof ffei_buf, TRUE,
			  &fgei_buf.fgei, sizeof fgei_buf, NULL, TRUE);
  if (NT_SUCCESS (status))
    {
      fattr3 *nfs_attr = (fattr3 *) (ffei_buf.ffei.EaName
				     + ffei_buf.ffei.EaNameLength + 1);
      if (fattr_buf)
	memcpy (fattr_buf, nfs_attr, sizeof (fattr3));
    }
  return status;
}
