/* fhandler_procsys.cc: fhandler for native NT namespace.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include <winioctl.h>
#include "ntdll.h"
#include "tls_pbuf.h"

#include <dirent.h>

/* Path of the /proc/sys filesystem */
const char procsys[] = "/proc/sys";
const size_t procsys_len = sizeof (procsys) - 1;

#define mk_unicode_path(p) \
	WCHAR namebuf[strlen (get_name ()) + 1]; \
	{ \
	  const char *from; \
	  PWCHAR to; \
	  for (to = namebuf, from = get_name () + procsys_len; *from; \
	       to++, from++) \
	    /* The NT device namespace is ASCII only. */ \
	    *to = (*from == '/') ? L'\\' : (WCHAR) *from; \
	  if (to == namebuf) \
	    *to++ = L'\\'; \
	  *to = L'\0'; \
	  RtlInitUnicodeString ((p), namebuf); \
	}

virtual_ftype_t
fhandler_procsys::exists (struct stat *buf)
{
  UNICODE_STRING path;
  UNICODE_STRING dir;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  NTSTATUS status;
  HANDLE h;
  FILE_BASIC_INFORMATION fbi;
  bool internal = false;
  bool desperate_parent_check = false;
  /* Default device type is character device. */
  virtual_ftype_t file_type = virt_chr;

  if (strlen (get_name ()) == procsys_len)
    return virt_rootdir;
  mk_unicode_path (&path);

  /* Try to open parent dir.  If it works, the object is definitely
     an object within the internal namespace.  We don't need to test
     it for being a file or dir on the filesystem anymore.  If the
     error is STATUS_OBJECT_TYPE_MISMATCH, we know that the file
     itself is external.  Otherwise we don't know. */
  RtlSplitUnicodePath (&path, &dir, NULL);
  /* RtlSplitUnicodePath preserves the trailing backslash in dir.  Don't
     preserve it to open the dir, unless it's the object root. */
  if (dir.Length > sizeof (WCHAR))
    dir.Length -= sizeof (WCHAR);
  InitializeObjectAttributes (&attr, &dir, OBJ_CASE_INSENSITIVE, NULL, NULL);
  status = NtOpenDirectoryObject (&h, DIRECTORY_QUERY, &attr);
  debug_printf ("NtOpenDirectoryObject: %y", status);
  if (NT_SUCCESS (status))
    {
      internal = true;
      NtClose (h);
    }

  /* First check if the object is a symlink. */
  InitializeObjectAttributes (&attr, &path, OBJ_CASE_INSENSITIVE, NULL, NULL);
  status = NtOpenSymbolicLinkObject (&h, READ_CONTROL | SYMBOLIC_LINK_QUERY,
				     &attr);
  debug_printf ("NtOpenSymbolicLinkObject: %y", status);
  if (NT_SUCCESS (status))
    {
      /* If requested, check permissions. */
      if (buf)
	get_object_attribute (h, &buf->st_uid, &buf->st_gid, &buf->st_mode);
      NtClose (h);
      return virt_symlink;
    }
  else if (status == STATUS_ACCESS_DENIED)
    return virt_symlink;
  /* Then check if it's an object directory. */
  status = NtOpenDirectoryObject (&h, READ_CONTROL | DIRECTORY_QUERY, &attr);
  debug_printf ("NtOpenDirectoryObject: %y", status);
  if (NT_SUCCESS (status))
    {
      /* If requested, check permissions. */
      if (buf)
	get_object_attribute (h, &buf->st_uid, &buf->st_gid, &buf->st_mode);
      NtClose (h);
      return virt_directory;
    }
  else if (status == STATUS_ACCESS_DENIED)
    return virt_directory;
  /* Next try to open as file/device. */
  status = NtOpenFile (&h, READ_CONTROL | FILE_READ_ATTRIBUTES, &attr, &io,
		       FILE_SHARE_VALID_FLAGS, FILE_OPEN_FOR_BACKUP_INTENT);
  debug_printf ("NtOpenFile: %y", status);
  /* Name is invalid, that's nothing. */
  if (status == STATUS_OBJECT_NAME_INVALID)
    return virt_none;
  /* If no media is found, or we get this dreaded sharing violation, let
     the caller immediately try again as normal file. */
  if (status == STATUS_NO_MEDIA_IN_DEVICE
      || status == STATUS_SHARING_VIOLATION)
    return virt_fsfile;	/* Just try again as normal file. */
  /* If file or path can't be found, let caller try again as normal file. */
  if (status == STATUS_OBJECT_PATH_NOT_FOUND
      || status == STATUS_OBJECT_NAME_NOT_FOUND)
    return virt_fsfile;
  /* Check for pipe errors, which make a good hint... */
  if (status >= STATUS_PIPE_NOT_AVAILABLE && status <= STATUS_PIPE_BUSY)
    return virt_pipe;
  if (status == STATUS_ACCESS_DENIED && !internal)
    {
      /* Check if this is just some file or dir on a real FS to circumvent
	 most permission problems.  Don't try that on internal objects,
	 since NtQueryAttributesFile might crash the machine if the underlying
	 driver is badly written. */
      status = NtQueryAttributesFile (&attr, &fbi);
      debug_printf ("NtQueryAttributesFile: %y", status);
      if (NT_SUCCESS (status))
	return (fbi.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	       ? virt_fsdir : virt_fsfile;
      /* Ok, so we're desperate and the file still maybe on some filesystem.
	 To check this, we now split the path until we can finally access any
	 of the parent's.  Then we fall through to check the parent type.  In
	 contrast to the first parent check, we now check explicitely with
	 trailing backslash.  This will fail for directories in the internal
	 namespace, so we won't accidentally test those. */
      dir = path;
      InitializeObjectAttributes (&attr, &dir, OBJ_CASE_INSENSITIVE,
				  NULL, NULL);
      do
	{
	  RtlSplitUnicodePath (&dir, &dir, NULL);
	  status = NtOpenFile (&h, READ_CONTROL | FILE_READ_ATTRIBUTES,
			       &attr, &io, FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_FOR_BACKUP_INTENT);
	  debug_printf ("NtOpenDirectoryObject: %y", status);
	  if (dir.Length > sizeof (WCHAR))
	    dir.Length -= sizeof (WCHAR);
	}
      while (dir.Length > sizeof (WCHAR) && !NT_SUCCESS (status));
      desperate_parent_check = true;
    }
  if (NT_SUCCESS (status))
    {
      FILE_FS_DEVICE_INFORMATION ffdi;

      /* If requested, check permissions.  If this is a parent handle from
	 the above desperate parent check, skip. */
      if (buf && !desperate_parent_check)
	get_object_attribute (h, &buf->st_uid, &buf->st_gid, &buf->st_mode);

      /* Check for the device type. */
      status = NtQueryVolumeInformationFile (h, &io, &ffdi, sizeof ffdi,
					     FileFsDeviceInformation);
      debug_printf ("NtQueryVolumeInformationFile: %y", status);
      /* Don't call NtQueryInformationFile unless we know it's a safe type.
	 The call is known to crash machines, if the underlying driver is
	 badly written. */
      if (NT_SUCCESS (status))
	{
	  if (ffdi.DeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM)
	    file_type = virt_blk;
	  else if (ffdi.DeviceType == FILE_DEVICE_NAMED_PIPE)
	    file_type = internal ? virt_blk : virt_pipe;
	  else if (ffdi.DeviceType == FILE_DEVICE_DISK
		   || ffdi.DeviceType == FILE_DEVICE_CD_ROM
		   || ffdi.DeviceType == FILE_DEVICE_DFS
		   || ffdi.DeviceType == FILE_DEVICE_VIRTUAL_DISK)
	    {
	      /* Check for file attributes.  If we get them, we peeked
		 into a real FS through /proc/sys. */
	      status = NtQueryInformationFile (h, &io, &fbi, sizeof fbi,
					       FileBasicInformation);
	      debug_printf ("NtQueryInformationFile: %y", status);
	      if (!NT_SUCCESS (status))
		file_type = virt_blk;
	      else
		file_type = (fbi.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			    ? virt_fsdir : virt_fsfile;
	    }
	}
      NtClose (h);
    }
  /* That's it.  Return type we found above. */
  return file_type;
}

virtual_ftype_t
fhandler_procsys::exists ()
{
  return exists (NULL);
}

fhandler_procsys::fhandler_procsys ():
  fhandler_virtual ()
{
}

#define UNREADABLE_SYMLINK_CONTENT "<access denied>"

bool
fhandler_procsys::fill_filebuf ()
{
  char *fnamep;
  UNICODE_STRING path, target;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  HANDLE h;
  tmp_pathbuf tp;
  size_t len;

  mk_unicode_path (&path);
  if (path.Buffer[path.Length / sizeof (WCHAR) - 1] == L'\\')
    path.Length -= sizeof (WCHAR);
  InitializeObjectAttributes (&attr, &path, OBJ_CASE_INSENSITIVE, NULL, NULL);
  status = NtOpenSymbolicLinkObject (&h, SYMBOLIC_LINK_QUERY, &attr);
  if (!NT_SUCCESS (status))
    goto unreadable;
  RtlInitEmptyUnicodeString (&target, tp.w_get (),
			     (NT_MAX_PATH - 1) * sizeof (WCHAR));
  status = NtQuerySymbolicLinkObject (h, &target, NULL);
  NtClose (h);
  if (!NT_SUCCESS (status))
    goto unreadable;
  len = sys_wcstombs (NULL, 0, target.Buffer, target.Length / sizeof (WCHAR));
  filebuf = (char *) crealloc_abort (filebuf, procsys_len + len + 1);
  sys_wcstombs (fnamep = stpcpy (filebuf, procsys), len + 1, target.Buffer,
		target.Length / sizeof (WCHAR));
  while ((fnamep = strchr (fnamep, '\\')))
    *fnamep = '/';
  return true;

unreadable:
  filebuf = (char *) crealloc_abort (filebuf,
				     sizeof (UNREADABLE_SYMLINK_CONTENT));
  strcpy (filebuf, UNREADABLE_SYMLINK_CONTENT);
  return false;
}

int __reg2
fhandler_procsys::fstat (struct stat *buf)
{
  const char *path = get_name ();
  debug_printf ("fstat (%s)", path);

  fhandler_base::fstat (buf);
  /* Best bet. */
  buf->st_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  buf->st_uid = 544;
  buf->st_gid = 18;
  buf->st_dev = buf->st_rdev = dev ();
  buf->st_ino = get_ino ();
  switch (exists (buf))
    {
    case virt_directory:
    case virt_rootdir:
    case virt_fsdir:
      buf->st_mode |= S_IFDIR;
      if (buf->st_mode & S_IRUSR)
	buf->st_mode |= S_IXUSR;
      if (buf->st_mode & S_IRGRP)
	buf->st_mode |= S_IXGRP;
      if (buf->st_mode & S_IROTH)
	buf->st_mode |= S_IXOTH;
      break;
    case virt_file:
    case virt_fsfile:
      buf->st_mode |= S_IFREG;
      break;
    case virt_symlink:
      buf->st_mode |= S_IFLNK;
      break;
    case virt_pipe:
      buf->st_mode |= S_IFIFO;
      break;
    case virt_socket:
      buf->st_mode |= S_IFSOCK;
      break;
    case virt_chr:
      buf->st_mode |= S_IFCHR;
      break;
    case virt_blk:
      buf->st_mode |= S_IFBLK;
      break;
    default:
      set_errno (ENOENT);
      return -1;
    }
  return 0;
}

DIR *
fhandler_procsys::opendir (int fd)
{
  UNICODE_STRING path;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  HANDLE h;
  DIR *dir;

  mk_unicode_path (&path);
  InitializeObjectAttributes (&attr, &path, OBJ_CASE_INSENSITIVE, NULL, NULL);
  status = NtOpenDirectoryObject (&h, DIRECTORY_QUERY, &attr);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  if (!(dir = fhandler_virtual::opendir (fd)))
    NtClose (h);
  else
    dir->__handle = h;
  return dir;
}

int
fhandler_procsys::readdir (DIR *dir, dirent *de)
{
  NTSTATUS status;
  struct fdbi
  {
    DIRECTORY_BASIC_INFORMATION dbi;
    WCHAR buf[2][NAME_MAX + 1];
  } f;
  int res = EBADF;

  if (dir->__handle != INVALID_HANDLE_VALUE)
    {
      BOOLEAN restart = dir->__d_position ? FALSE : TRUE;
      status = NtQueryDirectoryObject (dir->__handle, &f, sizeof f, TRUE,
				       restart, (PULONG) &dir->__d_position,
				       NULL);
      if (!NT_SUCCESS (status))
	res = ENMFILE;
      else
	{
	  sys_wcstombs (de->d_name, NAME_MAX + 1, f.dbi.ObjectName.Buffer,
			f.dbi.ObjectName.Length / sizeof (WCHAR));
	  de->d_ino = hash_path_name (get_ino (), de->d_name);
	  if (RtlEqualUnicodeString (&f.dbi.ObjectTypeName, &ro_u_natdir,
				     FALSE))
	    de->d_type = DT_DIR;
	  else if (RtlEqualUnicodeString (&f.dbi.ObjectTypeName, &ro_u_natsyml,
					  FALSE))
	    de->d_type = DT_LNK;
	  else if (!RtlEqualUnicodeString (&f.dbi.ObjectTypeName, &ro_u_natdev,
					   FALSE))
	    de->d_type = DT_CHR;
	  else /* Can't nail down "Device" objects without further testing. */
	    de->d_type = DT_UNKNOWN;
	  res = 0;
	}
    }
  syscall_printf ("%d = readdir(%p, %p)", res, dir, de);
  return res;
}

long
fhandler_procsys::telldir (DIR *dir)
{
  return dir->__d_position;
}

void
fhandler_procsys::seekdir (DIR *dir, long pos)
{
  dir->__d_position = pos;
}

int
fhandler_procsys::closedir (DIR *dir)
{
  if (dir->__handle != INVALID_HANDLE_VALUE)
    {
      NtClose (dir->__handle);
      dir->__handle = INVALID_HANDLE_VALUE;
    }
  return fhandler_virtual::closedir (dir);
}

void __reg3
fhandler_procsys::read (void *ptr, size_t& len)
{
  fhandler_base::raw_read (ptr, len);
}

ssize_t __stdcall
fhandler_procsys::write (const void *ptr, size_t len)
{
  return fhandler_base::raw_write (ptr, len);
}

int
fhandler_procsys::open (int flags, mode_t mode)
{
  int res = 0;

  if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
    set_errno (EINVAL);
  else
    {
      switch (exists (NULL))
	{
	case virt_directory:
	case virt_rootdir:
	  if ((flags & O_ACCMODE) != O_RDONLY)
	    set_errno (EISDIR);
	  else
	    {
	      nohandle (true);
	      res = 1;
	    }
	  break;
	case virt_none:
	  set_errno (ENOENT);
	  break;
	default:
	  res = fhandler_base::open (flags, mode);
	  break;
	}
    }
  syscall_printf ("%d = fhandler_procsys::open(%p, 0%o)", res, flags, mode);
  return res;
}

int
fhandler_procsys::close ()
{
  if (!nohandle ())
    NtClose (get_handle ());
  return fhandler_virtual::close ();
}
#if 0
int
fhandler_procsys::ioctl (unsigned int cmd, void *)
{
}
#endif
