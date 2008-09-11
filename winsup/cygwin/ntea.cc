/* ntea.cc: code for manipulating Extended Attributes

   Copyright 1997, 1998, 2000, 2001, 2006, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygtls.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include <stdlib.h>
#include <attr/xattr.h>

#define MAX_EA_NAME_LEN    256
#define MAX_EA_VALUE_LEN 65536

/* At least one maximum sized entry fits. */
#define EA_BUFSIZ (sizeof (FILE_FULL_EA_INFORMATION) + MAX_EA_NAME_LEN \
		   + MAX_EA_VALUE_LEN)

#define NEXT_FEA(p) ((PFILE_FULL_EA_INFORMATION) (p->NextEntryOffset \
		     ? (char *) p + p->NextEntryOffset : NULL))

ssize_t __stdcall
read_ea (HANDLE hdl, path_conv &pc, const char *name, char *value, size_t size)
{
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  ssize_t ret = -1;
  HANDLE h = hdl;
  ULONG glen = 0;
  PFILE_GET_EA_INFORMATION gea = NULL;
  PFILE_FULL_EA_INFORMATION fea;
  /* We have to store the latest EaName to compare with the next one, since
     ZwQueryEaFile has a bug when accessing files on a remote share.  It
     returns the last EA entry of the file infinitely.  Even utilizing the
     optional EaIndex only helps marginally.  If you use that, the last
     EA in the file is returned twice. */
  char lastname[MAX_EA_NAME_LEN];

  myfault efault;
  if (efault.faulted (EFAULT))
    goto out;

  pc.get_object_attr (attr, sec_none_nih);

  debug_printf ("read_ea (%S, %s, %p, %lu)",
		attr.ObjectName, name, value, size);

  fea = (PFILE_FULL_EA_INFORMATION) alloca (EA_BUFSIZ);

  if (name)
    {
      size_t nlen;

      /* Samba hides the user namespace from Windows clients.  If we try to
	 retrieve a user namespace item, we remove the leading namespace from
	 the name, otherwise the search fails. */
      if (!pc.fs_is_samba ())
	/* nothing to do */;
      else if (ascii_strncasematch (name, "user.", 5))
	name += 5;
      else
	{
	  set_errno (ENOATTR);
	  goto out;
	}

      if ((nlen = strlen (name)) >= MAX_EA_NAME_LEN)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      glen = sizeof (FILE_GET_EA_INFORMATION) + nlen;
      gea = (PFILE_GET_EA_INFORMATION) alloca (glen);

      gea->NextEntryOffset = 0;
      gea->EaNameLength = nlen;
      strcpy (gea->EaName, name);
    }

  while (true)
    {
      if (h)
	{
	  status = NtQueryEaFile (h, &io, fea, EA_BUFSIZ, TRUE, gea, glen,
				  NULL, TRUE);
	  if (status != STATUS_ACCESS_DENIED || !hdl)
	    break;
	}
      status = NtOpenFile (&h, READ_CONTROL | FILE_READ_EA, &attr, &io,
			   FILE_SHARE_VALID_FLAGS, FILE_OPEN_FOR_BACKUP_INTENT);
      if (!NT_SUCCESS (status))
	break;
      hdl = NULL;
    }
  if (!NT_SUCCESS (status))
    {
      if (status == STATUS_NO_EAS_ON_FILE)
	ret = 0;
      else if (status == STATUS_NONEXISTENT_EA_ENTRY)
	/* Actually this error code is either never generated, or it was only
	   generated in some old and long forgotton NT version.  See below. */
	set_errno (ENOATTR);
      else
	__seterrno_from_nt_status (status);
      goto out;
    }
  if (name)
    {
      if (size > 0)
	{
	  if (size < fea->EaValueLength)
	    {
	      set_errno (ERANGE);
	      goto out;
	    }
	  /* Another weird behaviour of ZwQueryEaFile.  If you ask for a
	     specific EA which is not present in the file's EA list, you don't
	     get a useful error code like STATUS_NONEXISTENT_EA_ENTRY.  Rather
	     ZwQueryEaFile returns success with the entry's EaValueLength
	     set to 0. */
	  if (!fea->EaValueLength)
	    {
	      set_errno (ENOATTR);
	      goto out;
	    }
	  else
	    memcpy (value, fea->EaName + fea->EaNameLength + 1,
		    fea->EaValueLength);
	}
      ret = fea->EaValueLength;
    }
  else
    {
      ret = 0;
      do
	{
	  if (pc.fs_is_samba ())	/* See below. */
	    fea->EaNameLength += 5;
	  if (size > 0)
	    {
	      if ((size_t) ret + fea->EaNameLength + 1 > size)
		{
		  set_errno (ERANGE);
		  goto out;
		}
	      /* Samba hides the user namespace from Windows clients.  We add
		 it in EA listings to keep tools like attr/getfattr/setfattr
		 happy. */
	      char tmpbuf[MAX_EA_NAME_LEN * 2], *tp = tmpbuf;
	      if (pc.fs_is_samba ())
		tp = stpcpy (tmpbuf, "user.");
	      stpcpy (tp, fea->EaName);
	      /* NTFS stores all EA names in uppercase unfortunately.  To keep
		 compatibility with ext/xfs EA namespaces and accompanying
		 tools, which expect the namespaces to be lower case, we return
		 EA names in lowercase if the file is on a native NTFS. */
	      if (pc.fs_is_ntfs ())
		strlwr (tp);
	      tp = stpcpy (value, tmpbuf) + 1;
	      ret += tp - value;
	      value = tp;
	    }
	  else
	    ret += fea->EaNameLength + 1;
	  strcpy (lastname, fea->EaName);
	  status = NtQueryEaFile (h, &io, fea, EA_BUFSIZ, TRUE, NULL, 0,
				  NULL, FALSE);
	}
      while (NT_SUCCESS (status) && strcmp (lastname, fea->EaName) != 0);
    }

out:
  if (!hdl)
    CloseHandle (h);
  debug_printf ("%d = read_ea (%S, %s, %p, %lu)",
		ret, attr.ObjectName, name, value, size);
  return ret;
}

int __stdcall
write_ea (HANDLE hdl, path_conv &pc, const char *name, const char *value,
	  size_t size, int flags)
{
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  int ret = -1;
  HANDLE h = hdl;
  PFILE_FULL_EA_INFORMATION fea;
  ULONG flen;
  size_t nlen;

  myfault efault;
  if (efault.faulted (EFAULT))
    goto out;

  pc.get_object_attr (attr, sec_none_nih);

  debug_printf ("write_ea (%S, %s, %p, %lu, %d)",
		attr.ObjectName, name, value, size, flags);

  /* Samba hides the user namespace from Windows clients.  If we get a
     user namespace item, we remove the leading namespace from the name.
     This keeps tools like attr/getfattr/setfattr happy.  Otherwise
     setting the EA fails as if we don't have the permissions. */
  if (pc.fs_is_samba () && ascii_strncasematch (name, "user.", 5))
    name += 5;
  else
    {
      set_errno (EOPNOTSUPP);
      goto out;
    }

  /* removexattr is supposed to fail with ENOATTR if the requested EA is not
     available.  This is equivalent to the XATTR_REPLACE flag for setxattr. */
  if (!value)
    flags = XATTR_REPLACE;

  if (flags)
    {
      if (flags != XATTR_CREATE && flags != XATTR_REPLACE)
	{
	  set_errno (EINVAL);
	  goto out;
	}
      ssize_t rret = read_ea (hdl, pc, name, NULL, 0);
      if (flags == XATTR_CREATE && rret > 0)
	{
	  set_errno (EEXIST);
	  goto out;
	}
      if (flags == XATTR_REPLACE && rret < 0)
	goto out;
    }

  if ((nlen = strlen (name)) >= MAX_EA_NAME_LEN)
    {
      set_errno (EINVAL);
      goto out;
    }
  flen = sizeof (FILE_FULL_EA_INFORMATION) + nlen + 1 + size;
  fea = (PFILE_FULL_EA_INFORMATION) alloca (flen);
  fea->NextEntryOffset = 0;
  fea->Flags = 0;
  fea->EaNameLength = nlen;
  fea->EaValueLength = size;
  strcpy (fea->EaName, name);
  if (value)
    memcpy (fea->EaName + fea->EaNameLength + 1, value, size);

  while (true)
    {
      if (h)
	{
	  status = NtSetEaFile (h, &io, fea, flen);
	  if (status != STATUS_ACCESS_DENIED || !hdl)
	    break;
	}
      status = NtOpenFile (&h, READ_CONTROL | FILE_WRITE_EA, &attr, &io,
			   FILE_SHARE_VALID_FLAGS, FILE_OPEN_FOR_BACKUP_INTENT);
      if (!NT_SUCCESS (status))
	break;
      hdl = NULL;
    }
  if (!NT_SUCCESS (status))
    {
      /* STATUS_EA_TOO_LARGE has a matching Win32 error ERROR_EA_TABLE_FULL.
	 Too bad RtlNtStatusToDosError does not translate STATUS_EA_TOO_LARGE
	 to ERROR_EA_TABLE_FULL, but to ERROR_EA_LIST_INCONSISTENT.  This
	 error code is also returned for STATUS_EA_LIST_INCONSISTENT, which
	 means the incoming EA list is... inconsistent.  For obvious reasons
	 we translate ERROR_EA_LIST_INCONSISTENT to EINVAL, so we have to
	 handle STATUS_EA_TOO_LARGE explicitely here, to get the correct
	 mapping to ENOSPC. */
      if (status == STATUS_EA_TOO_LARGE)
	set_errno (ENOSPC);
      else
	__seterrno_from_nt_status (status);
    }
  else
    ret = 0;

out:
  if (!hdl)
    CloseHandle (h);
  debug_printf ("%d = write_ea (%S, %s, %p, %lu, %d)",
		ret, attr.ObjectName, name, value, size, flags);
  return ret;
}

static ssize_t __stdcall
getxattr_worker (path_conv &pc, const char *name, void *value, size_t size)
{
  int res = -1;

  if (pc.error)
    {
      debug_printf ("got %d error from build_fh_name", pc.error);
      set_errno (pc.error);
    }
  else if (pc.exists ())
    {
      fhandler_base *fh;

      if (!(fh = build_fh_pc (pc)))
	return -1;

      res = fh->fgetxattr (name, value, size);
      delete fh;
    }
  else
    set_errno (ENOENT);
  return res;
}

extern "C" ssize_t
getxattr (const char *path, const char *name, void *value, size_t size)
{
  if (!name)
    {
      set_errno (EINVAL);
      return -1;
    }
  path_conv pc (path, PC_SYM_FOLLOW | PC_POSIX, stat_suffixes);
  return getxattr_worker (pc, name, value, size);
}

extern "C" ssize_t
lgetxattr (const char *path, const char *name, void *value, size_t size)
{
  if (!name)
    {
      set_errno (EINVAL);
      return -1;
    }
  path_conv pc (path, PC_SYM_NOFOLLOW | PC_POSIX, stat_suffixes);
  return getxattr_worker (pc, name, value, size);
}

extern "C" ssize_t
fgetxattr (int fd, const char *name, void *value, size_t size)
{
  int res;

  if (!name)
    {
      set_errno (EINVAL);
      return -1;
    }
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else
    res = cfd->fgetxattr (name, value, size);
  return res;
}

extern "C" ssize_t
listxattr (const char *path, char *list, size_t size)
{
  path_conv pc (path, PC_SYM_FOLLOW | PC_POSIX, stat_suffixes);
  return getxattr_worker (pc, NULL, list, size);
}

extern "C" ssize_t
llistxattr (const char *path, char *list, size_t size)
{
  path_conv pc (path, PC_SYM_NOFOLLOW | PC_POSIX, stat_suffixes);
  return getxattr_worker (pc, NULL, list, size);
}

extern "C" ssize_t
flistxattr (int fd, char *list, size_t size)
{
  int res;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else
    res = cfd->fgetxattr (NULL, list, size);
  return res;
}

static int __stdcall
setxattr_worker (path_conv &pc, const char *name, const void *value,
		 size_t size, int flags)
{
  int res = -1;

  if (pc.error)
    {
      debug_printf ("got %d error from build_fh_name", pc.error);
      set_errno (pc.error);
    }
  else if (pc.exists ())
    {
      fhandler_base *fh;

      if (!(fh = build_fh_pc (pc)))
	return -1;

      res = fh->fsetxattr (name, value, size, flags);
      delete fh;
    }
  else
    set_errno (ENOENT);
  return res;
}

extern "C" int
setxattr (const char *path, const char *name, const void *value, size_t size,
	  int flags)
{
  if (!size)
    {
      set_errno (EINVAL);
      return -1;
    }
  path_conv pc (path, PC_SYM_NOFOLLOW | PC_POSIX, stat_suffixes);
  return setxattr_worker (pc, name, value, size, flags);
}

extern "C" int
lsetxattr (const char *path, const char *name, const void *value, size_t size,
	   int flags)
{
  if (!size)
    {
      set_errno (EINVAL);
      return -1;
    }
  path_conv pc (path, PC_SYM_NOFOLLOW | PC_POSIX, stat_suffixes);
  return setxattr_worker (pc, name, value, size, flags);
}

extern "C" int
fsetxattr (int fd, const char *name, const void *value, size_t size, int flags)
{
  int res;

  if (!size)
    {
      set_errno (EINVAL);
      return -1;
    }
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else
    res = cfd->fsetxattr (name, value, size, flags);
  return res;
}

extern "C" int
removexattr (const char *path, const char *name)
{
  path_conv pc (path, PC_SYM_FOLLOW | PC_POSIX, stat_suffixes);
  return setxattr_worker (pc, name, NULL, 0, 0);
}

extern "C" int
lremovexattr (const char *path, const char *name)
{
  path_conv pc (path, PC_SYM_NOFOLLOW | PC_POSIX, stat_suffixes);
  return setxattr_worker (pc, name, NULL, 0, 0);
}

extern "C" int
fremovexattr (int fd, const char *name)
{
  int res;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else
    res = cfd->fsetxattr (name, NULL, 0, 0);
  return res;
}
