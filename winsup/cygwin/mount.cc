/* mount.cc: mount handling.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <mntent.h>
#include <ctype.h>
#include <winioctl.h>
#include <cygwin/version.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "shared_info.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include <ntdll.h>
#include <wchar.h>
#include <stdio.h>
#include <assert.h>

/* Determine if path prefix matches current cygdrive */
#define iscygdrive(path) \
  (path_prefix_p (mount_table->cygdrive, (path), mount_table->cygdrive_len, false))

#define iscygdrive_device(path) \
  (isalpha (path[mount_table->cygdrive_len]) && \
   (path[mount_table->cygdrive_len + 1] == '/' || \
    !path[mount_table->cygdrive_len + 1]))

#define isproc(path) \
  (path_prefix_p (proc, (path), proc_len, false))

bool NO_COPY mount_info::got_usr_bin;
bool NO_COPY mount_info::got_usr_lib;
int NO_COPY mount_info::root_idx = -1;

/* is_unc_share: Return non-zero if PATH begins with //server/share
		 or with one of the native prefixes //./ or //?/
   This function is only used to test for valid input strings.
   The later normalization drops the native prefixes. */

static inline bool __stdcall
is_native_path (const char *path)
{
  return isdirsep (path[0])
	 && (isdirsep (path[1]) || path[1] == '?')
	 && (path[2] == '?' || path[2] == '.')
	 && isdirsep (path[3])
	 && isalpha (path[4]);
}

static inline bool __stdcall
is_unc_share (const char *path)
{
  const char *p;
  return (isdirsep (path[0])
	 && isdirsep (path[1])
	 && isalnum (path[2])
	 && ((p = strpbrk (path + 3, "\\/")) != NULL)
	 && isalnum (p[1]));
}

/* Return true if src_path is a valid, internally supported device name.
   In that case, win32_path gets the corresponding NT device name and
   dev is appropriately filled with device information. */

static bool
win32_device_name (const char *src_path, char *win32_path, device& dev)
{
  dev.parse (src_path);
  if (dev == FH_FS || dev == FH_DEV)
    return false;
  strcpy (win32_path, dev.native ());
  return true;
}

/* Beginning with Samba 3.0.28a, Samba allows to get version information using
   the ExtendedInfo member returned by a FileFsObjectIdInformation request.
   We just store the samba_version information for now.  Older versions than
   3.2 are still guessed at by testing the file system flags. */
#define SAMBA_EXTENDED_INFO_MAGIC 0x536d4261 /* "SmBa" */
#define SAMBA_EXTENDED_INFO_VERSION_STRING_LENGTH 28
#pragma pack(push,4)
struct smb_extended_info {
  DWORD		samba_magic;		/* Always SAMBA_EXTENDED_INFO_MAGIC */
  DWORD		samba_version;		/* Major/Minor/Release/Revision */
  DWORD		samba_subversion;	/* Prerelease/RC/Vendor patch */
  LARGE_INTEGER samba_gitcommitdate;
  char		samba_version_string[SAMBA_EXTENDED_INFO_VERSION_STRING_LENGTH];
};
#pragma pack(pop)

#define MAX_FS_INFO_CNT 32
class fs_info_cache
{
  static muto fsi_lock;
  uint32_t count;
  struct {
    fs_info fsi;
    uint32_t hash;
  } entry[MAX_FS_INFO_CNT];

  uint32_t genhash (PFILE_FS_VOLUME_INFORMATION);

public:
  fs_info_cache () : count (0) { fsi_lock.init ("fsi_lock"); }
  fs_info *search (PFILE_FS_VOLUME_INFORMATION, uint32_t &);
  void add (uint32_t, fs_info *);
};

static fs_info_cache fsi_cache;
muto NO_COPY fs_info_cache::fsi_lock;

uint32_t
fs_info_cache::genhash (PFILE_FS_VOLUME_INFORMATION pffvi)
{
  uint32_t hash = 0;
  const uint16_t *p = (const uint16_t *) pffvi;
  const uint16_t *end = (const uint16_t *)
			((const uint8_t *) p + sizeof *pffvi
			 + pffvi->VolumeLabelLength  - sizeof (WCHAR));
  pffvi->__dummy = 0;	/* This member can have random values! */
  while (p < end)
    hash = *p++ + (hash << 6) + (hash << 16) - hash;
  return hash;
}

fs_info *
fs_info_cache::search (PFILE_FS_VOLUME_INFORMATION pffvi, uint32_t &hash)
{
  hash = genhash (pffvi);
  for (uint32_t i = 0; i < count; ++i)
    if (entry[i].hash == hash)
      return &entry[i].fsi;
  return NULL;
}

void
fs_info_cache::add (uint32_t hashval, fs_info *new_fsi)
{
  fsi_lock.acquire ();
  if (count < MAX_FS_INFO_CNT)
    {
      entry[count].fsi = *new_fsi;
      entry[count].hash = hashval;
      ++count;
    }
  fsi_lock.release ();
}

bool
fs_info::update (PUNICODE_STRING upath, HANDLE in_vol)
{
  NTSTATUS status = STATUS_OBJECT_NAME_NOT_FOUND;
  HANDLE vol;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  bool no_media = false;
  FILE_FS_DEVICE_INFORMATION ffdi;
  FILE_FS_OBJECTID_INFORMATION ffoi;
  struct {
    FILE_FS_ATTRIBUTE_INFORMATION ffai;
    WCHAR buf[NAME_MAX + 1];
  } ffai_buf;
  struct {
    FILE_FS_VOLUME_INFORMATION ffvi;
    WCHAR buf[NAME_MAX + 1];
  } ffvi_buf;
  UNICODE_STRING dir;
  UNICODE_STRING fsname;

  clear ();
  /* Always caseinsensitive.  We really just need access to the drive. */
  InitializeObjectAttributes (&attr, upath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  if (in_vol)
    vol = in_vol;
  else
    {
      ULONG access = READ_CONTROL;
      /* Note: Don't use the FILE_OPEN_REPARSE_POINT flag here.  The reason
	 is that symlink_info::check relies on being able to open a handle
	 to the target of a volume mount point. */
      status = NtOpenFile (&vol, access, &attr, &io, FILE_SHARE_VALID_FLAGS,
			   FILE_OPEN_FOR_BACKUP_INTENT);
      /* At least one filesystem (HGFS, VMware shared folders) doesn't like
	 to be opened with access set to just READ_CONTROL. */
      if (status == STATUS_INVALID_PARAMETER)
	{
	  access |= FILE_READ_DATA;
	  status = NtOpenFile (&vol, access, &attr, &io, FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_FOR_BACKUP_INTENT);
	}
      while (!NT_SUCCESS (status)
	     && (attr.ObjectName->Length > 7 * sizeof (WCHAR)
		 || status == STATUS_NO_MEDIA_IN_DEVICE))
	{
	  RtlSplitUnicodePath (attr.ObjectName, &dir, NULL);
	  attr.ObjectName = &dir;
	  if (status == STATUS_NO_MEDIA_IN_DEVICE)
	    {
	      no_media = true;
	      dir.Length = 6 * sizeof (WCHAR);
	    }
	  else if (dir.Length > 7 * sizeof (WCHAR))
	    dir.Length -= sizeof (WCHAR);
	  status = NtOpenFile (&vol, access, &attr, &io, FILE_SHARE_VALID_FLAGS,
			       FILE_OPEN_FOR_BACKUP_INTENT);
	}
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("Cannot access path %S, status %y",
			attr.ObjectName, status);
	  return false;
	}
    }
  sernum = 0;
  status = NtQueryVolumeInformationFile (vol, &io, &ffvi_buf.ffvi,
					 sizeof ffvi_buf,
					 FileFsVolumeInformation);
  uint32_t hash = 0;
  if (NT_SUCCESS (status))
    {
      /* If the FS doesn't return a valid serial number (PrlSF is a candidate),
	 create reproducible serial number from path.  We need this to create
	 a unique per-drive/share hash. */
      if (ffvi_buf.ffvi.VolumeSerialNumber == 0)
	{
	  UNICODE_STRING path_prefix;
	  WCHAR *p;

	  if (upath->Buffer[5] == L':' && upath->Buffer[6] == L'\\')
	    p = upath->Buffer + 6;
	  else
	    {
	      /* We're expecting an UNC path.  Move p to the backslash after
	         "\??\UNC\server\share" or the trailing NUL. */
	      p = upath->Buffer + 7;  /* Skip "\??\UNC" */
	      int bs_cnt = 0;

	      while (*++p)
		if (*p == L'\\')
		    if (++bs_cnt > 1)
		      break;
	    }
	  RtlInitCountedUnicodeString (&path_prefix, upath->Buffer,
				       (p - upath->Buffer) * sizeof (WCHAR));
	  ffvi_buf.ffvi.VolumeSerialNumber = hash_path_name ((ino_t) 0,
							     &path_prefix);
	}
      fs_info *fsi = fsi_cache.search (&ffvi_buf.ffvi, hash);
      if (fsi)
	{
	  *this = *fsi;
	  if (!in_vol)
	    NtClose (vol);
	  return true;
	}
      sernum = ffvi_buf.ffvi.VolumeSerialNumber;
    }
  status = NtQueryVolumeInformationFile (vol, &io, &ffdi, sizeof ffdi,
					 FileFsDeviceInformation);
  if (!NT_SUCCESS (status))
    ffdi.DeviceType = ffdi.Characteristics = 0;

  if ((ffdi.Characteristics & FILE_REMOTE_DEVICE)
      || (!ffdi.DeviceType
	  && RtlEqualUnicodePathPrefix (attr.ObjectName, &ro_u_uncp, TRUE)))
    is_remote_drive (true);

  if (!no_media)
    status = NtQueryVolumeInformationFile (vol, &io, &ffai_buf.ffai,
					   sizeof ffai_buf,
					   FileFsAttributeInformation);
  if (no_media || !NT_SUCCESS (status))
    {
      debug_printf ("Cannot get volume attributes (%S), %y",
		    attr.ObjectName, status);
      if (!in_vol)
	NtClose (vol);
      return false;
    }
  flags (ffai_buf.ffai.FileSystemAttributes);
  name_len (ffai_buf.ffai.MaximumComponentNameLength);
  RtlInitCountedUnicodeString (&fsname, ffai_buf.ffai.FileSystemName,
			       ffai_buf.ffai.FileSystemNameLength);
  if (is_remote_drive ())
    {
/* Should be reevaluated for each new OS.  Right now this mask is valid up
   to Windows 8.  The important point here is to test only flags indicating
   capabilities and to ignore flags indicating a specific state of this
   volume.  At present these flags to ignore are FILE_VOLUME_IS_COMPRESSED,
   FILE_READ_ONLY_VOLUME, and FILE_SEQUENTIAL_WRITE_ONCE.  The additional
   filesystem flags supported since Windows 7 are also ignored for now.
   They add information, but only on W7 and later, and only for filesystems
   also supporting these flags, right now only NTFS. */
#define GETVOLINFO_VALID_MASK (0x002701ffUL)
#define TEST_GVI(f,m) (((f) & GETVOLINFO_VALID_MASK) == (m))

/* FIXME: This flag twist is getting awkward.  There should really be some
   other method.  Maybe we need mount flags to allow the user to fix file
   system problems without having to wait for a Cygwin fix. */

/* Volume quotas are potentially supported since Samba 3.0, object ids and
   the unicode on disk flag since Samba 3.2. */
#define SAMBA_IGNORE (FILE_VOLUME_QUOTAS \
		      | FILE_SUPPORTS_OBJECT_IDS \
		      | FILE_UNICODE_ON_DISK)
#define FS_IS_SAMBA TEST_GVI(flags () & ~SAMBA_IGNORE, \
			     FILE_CASE_SENSITIVE_SEARCH \
			     | FILE_CASE_PRESERVED_NAMES \
			     | FILE_PERSISTENT_ACLS)
/* Netapp DataOnTap. */
#define NETAPP_IGNORE (FILE_SUPPORTS_SPARSE_FILES \
		       | FILE_PERSISTENT_ACLS)
#define FS_IS_NETAPP_DATAONTAP TEST_GVI(flags () & ~NETAPP_IGNORE, \
			     FILE_CASE_SENSITIVE_SEARCH \
			     | FILE_CASE_PRESERVED_NAMES \
			     | FILE_UNICODE_ON_DISK \
			     | FILE_NAMED_STREAMS)
/* These are the minimal flags supported by NTFS since Windows 2000.  Every
   filesystem not supporting these flags is not a native NTFS.  We subsume
   them under the filesystem type "cifs". */
#define MINIMAL_WIN_NTFS_FLAGS (FILE_CASE_SENSITIVE_SEARCH \
				| FILE_CASE_PRESERVED_NAMES \
				| FILE_UNICODE_ON_DISK \
				| FILE_PERSISTENT_ACLS \
				| FILE_FILE_COMPRESSION \
				| FILE_VOLUME_QUOTAS \
				| FILE_SUPPORTS_SPARSE_FILES \
				| FILE_SUPPORTS_REPARSE_POINTS \
				| FILE_SUPPORTS_OBJECT_IDS \
				| FILE_SUPPORTS_ENCRYPTION \
				| FILE_NAMED_STREAMS)
#define FS_IS_WINDOWS_NTFS TEST_GVI(flags () & MINIMAL_WIN_NTFS_FLAGS, \
				    MINIMAL_WIN_NTFS_FLAGS)
/* These are the exact flags of a real Windows FAT/FAT32 filesystem.
   Anything else is a filesystem faking to be FAT. */
#define WIN_FAT_FLAGS (FILE_CASE_PRESERVED_NAMES | FILE_UNICODE_ON_DISK)
#define FS_IS_WINDOWS_FAT  TEST_GVI(flags (), WIN_FAT_FLAGS)

      if ((flags () & FILE_SUPPORTS_OBJECT_IDS)
	  && NT_SUCCESS (NtQueryVolumeInformationFile (vol, &io, &ffoi,
						   sizeof ffoi,
						   FileFsObjectIdInformation)))
	{
	  smb_extended_info *extended_info = (smb_extended_info *)
					     &ffoi.ExtendedInfo;
	  if (extended_info->samba_magic == SAMBA_EXTENDED_INFO_MAGIC)
	    {
	      is_samba (true);
	      samba_version (extended_info->samba_version);
	    }
	}
      /* First check the remote filesystems claiming to be NTFS. */
      if (!got_fs ()
	  && is_ntfs (RtlEqualUnicodeString (&fsname, &ro_u_ntfs, FALSE))
	  /* Test for older Samba releases not supporting extended info. */
	  && !is_samba (FS_IS_SAMBA)
	  /* Netapp inode info is unusable, can't handle trailing dots and
	     spaces, has a bug in "move and delete" semantics. */
	  && !is_netapp (FS_IS_NETAPP_DATAONTAP))
	/* Any other remote FS faking to be NTFS. */
	is_cifs (!FS_IS_WINDOWS_NTFS);
      /* Then check remote filesystems claiming to be FAT.  Except for real
	 FAT and Netapp, all of them are subsumed under the "CIFS" filesystem
	 type for now. */
      if (!got_fs ()
	  && is_fat (RtlEqualUnicodePathPrefix (&fsname, &ro_u_fat, TRUE))
	  && !is_netapp (FS_IS_NETAPP_DATAONTAP))
	is_cifs (!FS_IS_WINDOWS_FAT);
      /* Then check remote filesystems honest about their name. */
      if (!got_fs ()
	  /* Microsoft NFS needs distinct access methods for metadata. */
	  && !is_nfs (RtlEqualUnicodeString (&fsname, &ro_u_nfs, FALSE))
	  /* MVFS == Rational ClearCase remote filesystem.  Has a couple of
	     drawbacks, like not supporting DOS attributes other than R/O
	     and stuff like that. */
	  && !is_mvfs (RtlEqualUnicodePathPrefix (&fsname, &ro_u_mvfs, FALSE))
	  /* NWFS == Novell Netware FS.  Broken info class, see below. */
	  /* NcFsd == Novell Netware FS via own driver. */
	  && !is_nwfs (RtlEqualUnicodeString (&fsname, &ro_u_nwfs, FALSE))
	  && !is_ncfsd (RtlEqualUnicodeString (&fsname, &ro_u_ncfsd, FALSE))
	  /* UNIXFS == TotalNet Advanced Server (TAS).  Doesn't support
	     FileIdBothDirectoryInformation.  See below. */
	  && !is_unixfs (RtlEqualUnicodeString (&fsname, &ro_u_unixfs, FALSE))
	  /* AFSRDRFsd == Andrew File System.  Doesn't support DOS attributes.
	     Only native symlinks are supported. */
	  && !is_afs (RtlEqualUnicodeString (&fsname, &ro_u_afs, FALSE)))
	{
	  /* PrlSF == Parallels Desktop File System.  Has a bug in
	     FileNetworkOpenInformation, see below. */
	  is_prlfs (RtlEqualUnicodeString (&fsname, &ro_u_prlfs, FALSE));
	}
      if (got_fs ())
	{
	  /* UNIXFS is known to choke on FileIdBothDirectoryInformation.
	     Some other CIFS servers have problems with this call as well.
	     Know example: EMC NS-702.  We just don't use that info class on
	     any remote CIFS.  */
	  has_buggy_fileid_dirinfo (is_cifs () || is_unixfs ());
	  /* NWFS is known to have a broken FileBasicInformation info
	     class.  It can't be used to fetch information, only to set
	     information.  Therefore, for NWFS we have to fallback to the
	     FileNetworkOpenInformation info class.  Unfortunately we can't
	     use FileNetworkOpenInformation all the time since that fails on
	     other filesystems like NFS.
	     UNUSED, but keep in for information purposes. */
	  has_buggy_basic_info (is_nwfs ());
	  /* Netapp and NWFS/NcFsd are too dumb to allow non-DOS filenames
	     containing trailing dots and spaces when accessed from Windows
	     clients.  We subsume CIFS into this class of filesystems right
	     away since at least some of them are not capable either. */
	  has_dos_filenames_only (is_netapp () || is_nwfs ()
				  || is_ncfsd () || is_cifs ());
	  /* Netapp and NWFS don't grok re-opening a file by handle.  They
	     only support this if the filename is non-null and the handle is
	     the handle to a directory. NcFsd IR10 is supposed to be ok. */
	  has_buggy_reopen (is_netapp () || is_nwfs ());
	}
    }
  if (!got_fs ()
      && !is_ntfs (RtlEqualUnicodeString (&fsname, &ro_u_ntfs, FALSE))
      && !is_fat (RtlEqualUnicodePathPrefix (&fsname, &ro_u_fat, TRUE))
      && !is_refs (RtlEqualUnicodeString (&fsname, &ro_u_refs, FALSE))
      && !is_csc_cache (RtlEqualUnicodeString (&fsname, &ro_u_csc, FALSE))
      && is_cdrom (ffdi.DeviceType == FILE_DEVICE_CD_ROM))
    is_udf (RtlEqualUnicodeString (&fsname, &ro_u_udf, FALSE));
  if (!got_fs ())
    {
      /* The filesystem name is only used in fillout_mntent and only if
	 the filesystem isn't one of the well-known filesystems anyway. */
      sys_wcstombs (fsn, sizeof fsn, ffai_buf.ffai.FileSystemName,
		    ffai_buf.ffai.FileSystemNameLength / sizeof (WCHAR));
      strlwr (fsn);
    }
  has_acls (flags () & FS_PERSISTENT_ACLS);
  /* Netapp inode numbers are fly-by-night. */
  hasgood_inode ((has_acls () && !is_netapp ()) || is_nfs ());
  /* Case sensitivity is supported if FILE_CASE_SENSITIVE_SEARCH is set,
     except on Samba which handles Windows clients case insensitive.

     NFS doesn't set the FILE_CASE_SENSITIVE_SEARCH flag but is case
     sensitive. */
  caseinsensitive ((!(flags () & FILE_CASE_SENSITIVE_SEARCH) || is_samba ())
		   && !is_nfs ());

  if (!in_vol)
    NtClose (vol);
  fsi_cache.add (hash, this);
  return true;
}

inline void
mount_info::create_root_entry (const PWCHAR root)
{
 /* Create a default root dir derived from the location of the Cygwin DLL.
    The entry is immutable, unless the "override" option is given in /etc/fstab. */
  char native_root[PATH_MAX];
  sys_wcstombs (native_root, PATH_MAX, root);
  assert (*native_root != '\0');
  if (add_item (native_root, "/",
		MOUNT_SYSTEM | MOUNT_BINARY | MOUNT_IMMUTABLE | MOUNT_AUTOMATIC)
      < 0)
    api_fatal ("add_item (\"%s\", \"/\", ...) failed, errno %d", native_root, errno);
  /* Create a default cygdrive entry.  Note that this is a user entry.
     This allows to override it with mount, unless the sysadmin created
     a cygdrive entry in /etc/fstab. */
  cygdrive_flags = MOUNT_BINARY | MOUNT_NOPOSIX | MOUNT_CYGDRIVE;
  strcpy (cygdrive, CYGWIN_INFO_CYGDRIVE_DEFAULT_PREFIX "/");
  cygdrive_len = strlen (cygdrive);
}

/* init: Initialize the mount table.  */

void
mount_info::init (bool user_init)
{
  PWCHAR pathend;
  WCHAR path[PATH_MAX];

  pathend = wcpcpy (path, cygheap->installation_root);
  if (!user_init)
    create_root_entry (path);

  pathend = wcpcpy (pathend, L"\\etc\\fstab");
  from_fstab (user_init, path, pathend);

  if (!user_init && (!got_usr_bin || !got_usr_lib))
    {
      char native[PATH_MAX];
      if (root_idx < 0)
	api_fatal ("root_idx %d, user_shared magic %y, nmounts %d", root_idx, user_shared->version, nmounts);
      char *p = stpcpy (native, mount[root_idx].native_path);
      if (!got_usr_bin)
      {
	stpcpy (p, "\\bin");
	add_item (native, "/usr/bin",
		  MOUNT_SYSTEM | MOUNT_BINARY | MOUNT_AUTOMATIC);
      }
      if (!got_usr_lib)
      {
	stpcpy (p, "\\lib");
	add_item (native, "/usr/lib",
		  MOUNT_SYSTEM | MOUNT_BINARY | MOUNT_AUTOMATIC);
      }
    }
}

static void
set_flags (unsigned *flags, unsigned val)
{
  *flags = val;
  if (!(*flags & PATH_BINARY))
    {
      *flags |= PATH_TEXT;
      debug_printf ("flags: text (%y)", *flags & (PATH_TEXT | PATH_BINARY));
    }
  else
    {
      *flags |= PATH_BINARY;
      debug_printf ("flags: binary (%y)", *flags & (PATH_TEXT | PATH_BINARY));
    }
}

int
mount_item::build_win32 (char *dst, const char *src, unsigned *outflags, unsigned chroot_pathlen)
{
  int n, err = 0;
  const char *real_native_path;
  int real_posix_pathlen;
  set_flags (outflags, (unsigned) flags);
  if (!cygheap->root.exists () || posix_pathlen != 1 || posix_path[0] != '/')
    {
      n = native_pathlen;
      real_native_path = native_path;
      real_posix_pathlen = chroot_pathlen ?: posix_pathlen;
    }
  else
    {
      n = cygheap->root.native_length ();
      real_native_path = cygheap->root.native_path ();
      real_posix_pathlen = posix_pathlen;
    }
  memcpy (dst, real_native_path, n + 1);
  const char *p = src + real_posix_pathlen;
  if (*p == '/')
    /* nothing */;
  else if ((isdrive (dst) && !dst[2]) || *p)
    dst[n++] = '\\';
  if ((n + strlen (p)) >= NT_MAX_PATH)
    err = ENAMETOOLONG;
  else
    backslashify (p, dst + n, 0);
  return err;
}

/* conv_to_win32_path: Ensure src_path is a pure Win32 path and store
   the result in win32_path.

   If win32_path != NULL, the relative path, if possible to keep, is
   stored in win32_path.  If the relative path isn't possible to keep,
   the full path is stored.

   If full_win32_path != NULL, the full path is stored there.

   The result is zero for success, or an errno value.

   {,full_}win32_path must have sufficient space (i.e. NT_MAX_PATH bytes).  */

int
mount_info::conv_to_win32_path (const char *src_path, char *dst, device& dev,
				unsigned *flags)
{
  bool chroot_ok = !cygheap->root.exists ();

  dev = FH_FS;

  *flags = 0;
  debug_printf ("conv_to_win32_path (%s)", src_path);

  int i, rc;
  mount_item *mi = NULL;	/* initialized to avoid compiler warning */

  /* The path is already normalized, without ../../ stuff, we need to have this
     so that we can move from one mounted directory to another with relative
     stuff.

     eg mounting c:/foo /foo
     d:/bar /bar

     cd /bar
     ls ../foo

     should look in c:/foo, not d:/foo.

     converting normalizex UNIX path to a DOS-style path, looking up the
     appropriate drive in the mount table.  */

  /* See if this is a cygwin "device" */
  if (win32_device_name (src_path, dst, dev))
    {
      *flags = MOUNT_BINARY;	/* FIXME: Is this a sensible default for devices? */
      rc = 0;
      goto out_no_chroot_check;
    }

  /* If the path is on a network drive or a //./ resp. //?/ path prefix,
     bypass the mount table.  If it's // or //MACHINE, use the netdrive
     device. */
  if (src_path[1] == '/')
    {
      if (!strchr (src_path + 2, '/'))
	{
	  dev = *netdrive_dev;
	  set_flags (flags, PATH_BINARY);
	}
      else
	{
	  /* For UNC paths, use the cygdrive prefix flags as default setting.
	     This is more natural since UNC paths, just like cygdrive paths,
	     are rather (warning, poetic description ahead) windows into the
	     native Win32 world.  This also gives the user an elegant way to
	     change the settings for those paths in a central place. */
	  set_flags (flags, (unsigned) cygdrive_flags);
	}
      backslashify (src_path, dst, 0);
      /* Go through chroot check */
      goto out;
    }
  if (isproc (src_path))
    {
      dev = *proc_dev;
      dev = fhandler_proc::get_proc_fhandler (src_path);
      if (dev == FH_NADA)
	return ENOENT;
      set_flags (flags, PATH_BINARY);
      if (isprocsys_dev (dev))
	{
	  if (src_path[procsys_len])
	    backslashify (src_path + procsys_len, dst, 0);
	  else	/* Avoid empty NT path. */
	    stpcpy (dst, "\\");
	  set_flags (flags, (unsigned) cygdrive_flags);
	}
      else
	strcpy (dst, src_path);
      goto out;
    }
  /* Check if the cygdrive prefix was specified.  If so, just strip
     off the prefix and transform it into an MS-DOS path. */
  else if (iscygdrive (src_path))
    {
      int n = mount_table->cygdrive_len - 1;
      int unit;

      if (!src_path[n])
	{
	  dst[0] = '\0';
	  if (mount_table->cygdrive_len > 1)
	    dev = *cygdrive_dev;
	}
      else if (cygdrive_win32_path (src_path, dst, unit))
	{
	  set_flags (flags, (unsigned) cygdrive_flags);
	  goto out;
	}
      else if (mount_table->cygdrive_len > 1)
	return ENOENT;
    }

  int chroot_pathlen;
  chroot_pathlen = 0;
  /* Check the mount table for prefix matches. */
  for (i = 0; i < nmounts; i++)
    {
      const char *path;
      int len;

      mi = mount + posix_sorted[i];
      if (!cygheap->root.exists ()
	  || (mi->posix_pathlen == 1 && mi->posix_path[0] == '/'))
	{
	  path = mi->posix_path;
	  len = mi->posix_pathlen;
	}
      else if (cygheap->root.posix_ok (mi->posix_path))
	{
	  path = cygheap->root.unchroot (mi->posix_path);
	  chroot_pathlen = len = strlen (path);
	}
      else
	{
	  chroot_pathlen = 0;
	  continue;
	}

      if (path_prefix_p (path, src_path, len, mi->flags & MOUNT_NOPOSIX))
	break;
    }

  if (i < nmounts)
    {
      int err = mi->build_win32 (dst, src_path, flags, chroot_pathlen);
      if (err)
	return err;
      chroot_ok = true;
    }
  else
    {
      int offset = 0;
      if (src_path[1] != '/' && src_path[1] != ':')
	offset = cygheap->cwd.get_drive (dst);
      backslashify (src_path, dst + offset, 0);
    }
 out:
  if (chroot_ok || cygheap->root.ischroot_native (dst))
    rc = 0;
  else
    {
      debug_printf ("attempt to access outside of chroot '%s - %s'",
		    cygheap->root.posix_path (), cygheap->root.native_path ());
      rc = ENOENT;
    }

 out_no_chroot_check:
  debug_printf ("src_path %s, dst %s, flags %y, rc %d", src_path, dst, *flags, rc);
  return rc;
}

int
mount_info::get_mounts_here (const char *parent_dir, int parent_dir_len,
			     PUNICODE_STRING mount_points,
			     PUNICODE_STRING cygd)
{
  int n_mounts = 0;

  for (int i = 0; i < nmounts; i++)
    {
      mount_item *mi = mount + posix_sorted[i];
      char *last_slash = strrchr (mi->posix_path, '/');
      if (!last_slash)
	continue;
      if (last_slash == mi->posix_path)
	{
	  if (parent_dir_len == 1 && mi->posix_pathlen > 1)
	    RtlCreateUnicodeStringFromAsciiz (&mount_points[n_mounts++],
					      last_slash + 1);
	}
      else if (parent_dir_len == last_slash - mi->posix_path
	       && strncasematch (parent_dir, mi->posix_path, parent_dir_len))
	RtlCreateUnicodeStringFromAsciiz (&mount_points[n_mounts++],
					  last_slash + 1);
    }
  RtlCreateUnicodeStringFromAsciiz (cygd, cygdrive + 1);
  if (cygd->Length)
    cygd->Length -= 2;	// Strip trailing slash
  return n_mounts;
}

/* cygdrive_posix_path: Build POSIX path used as the
   mount point for cygdrives created when there is no other way to
   obtain a POSIX path from a Win32 one.

   Recognized flag values:
   - 0x001:                      Add trailing slash.
   - 0x200 == CCP_PROC_CYGDRIVE: Return /proc/cygdrive rather than actual
                                 cygdrive prefix. */

void
mount_info::cygdrive_posix_path (const char *src, char *dst, int flags)
{
  int len;

  if (flags & CCP_PROC_CYGDRIVE)
    {
      len = sizeof ("/proc/cygdrive/") - 1;
      memcpy (dst, "/proc/cygdrive/", len + 1);
    }
  else
    {
      len = cygdrive_len;
      memcpy (dst, cygdrive, len + 1);
    }

  /* Now finish the path off with the drive letter to be used.
     The cygdrive prefix always ends with a trailing slash so
     the drive letter is added after the path. */
  dst[len++] = cyg_tolower (src[0]);
  if (!src[2] || (isdirsep (src[2]) && !src[3]))
    dst[len++] = '\000';
  else
    {
      int n;
      dst[len++] = '/';
      if (isdirsep (src[2]))
	n = 3;
      else
	n = 2;
      strcpy (dst + len, src + n);
    }
  slashify (dst, dst, !!(flags & 0x1));
}

int
mount_info::cygdrive_win32_path (const char *src, char *dst, int& unit)
{
  int res;
  const char *p = src + cygdrive_len;
  if (!isalpha (*p) || (!isdirsep (p[1]) && p[1]))
    {
      unit = -1; /* FIXME: should be zero, maybe? */
      dst[0] = '\0';
      res = 0;
    }
  else
    {
      /* drive letter must always be uppercase for casesensitive native NT. */
      dst[0] = cyg_toupper (*p);
      dst[1] = ':';
      strcpy (dst + 2, p + 1);
      backslashify (dst, dst, !dst[2]);
      unit = dst[0];
      res = 1;
    }
  debug_printf ("src '%s', dst '%s'", src, dst);
  return res;
}

/* conv_to_posix_path: Ensure src_path is a POSIX path.

   The result is zero for success, or an errno value.
   posix_path must have sufficient space (i.e. NT_MAX_PATH bytes).
   If keep_rel_p is non-zero, relative paths stay that way.  */

/* TODO: Change conv_to_posix_path to work with native paths. */

/* src_path is a wide Win32 path. */
int
mount_info::conv_to_posix_path (PWCHAR src_path, char *posix_path,
				int ccp_flags)
{
  bool changed = false;
  if (!wcsncmp (src_path, L"\\\\?\\", 4))
    {
      src_path += 4;
      if (src_path[1] != L':') /* native UNC path */
	{
	  *(src_path += 2) = L'\\';
	  changed = true;
	}
    }
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  sys_wcstombs (buf, NT_MAX_PATH, src_path);
  int ret = conv_to_posix_path (buf, posix_path, ccp_flags);
  if (changed)
    src_path[0] = L'C';
  return ret;
}

int
mount_info::conv_to_posix_path (const char *src_path, char *posix_path,
				int ccp_flags)
{
  int src_path_len = strlen (src_path);
  int relative = !isabspath (src_path);
  int append_slash;

  if (src_path_len <= 1)
    append_slash = 0;
  else
    {
      const char *lastchar = src_path + src_path_len - 1;
      append_slash = isdirsep (*lastchar)
		     && ((ccp_flags & __CCP_APP_SLASH) || lastchar[-1] != ':');
    }

  debug_printf ("conv_to_posix_path (%s, 0x%x, %s)", src_path, ccp_flags,
		append_slash ? "add-slash" : "no-add-slash");

  if (src_path_len >= NT_MAX_PATH)
    {
      debug_printf ("ENAMETOOLONG");
      return ENAMETOOLONG;
    }

  /* FIXME: For now, if the path is relative and it's supposed to stay
     that way, skip mount table processing. */

  if ((ccp_flags & CCP_RELATIVE) && relative)
    {
      slashify (src_path, posix_path, 0);
      debug_printf ("%s = conv_to_posix_path (%s)", posix_path, src_path);
      return 0;
    }

  tmp_pathbuf tp;
  char *pathbuf = tp.c_get ();
  char *tail;
  int rc = normalize_win32_path (src_path, pathbuf, tail);
  if (rc != 0)
    {
      debug_printf ("%d = conv_to_posix_path(%s)", rc, src_path);
      return rc;
    }

  int pathbuflen = tail - pathbuf;
  for (int i = 0; i < nmounts; ++i)
    {
      mount_item &mi = mount[native_sorted[i]];
      if (!path_prefix_p (mi.native_path, pathbuf, mi.native_pathlen,
			  mi.flags & MOUNT_NOPOSIX))
	continue;

      if (cygheap->root.exists () && !cygheap->root.posix_ok (mi.posix_path))
	continue;

      /* SRC_PATH is in the mount table. */
      int nextchar;
      const char *p = pathbuf + mi.native_pathlen;

      if (!*p || !p[1])
	nextchar = 0;
      else if (isdirsep (*p))
	nextchar = -1;
      else
	nextchar = 1;

      int addslash = nextchar > 0 ? 1 : 0;
      if ((mi.posix_pathlen + (pathbuflen - mi.native_pathlen) + addslash) >= NT_MAX_PATH)
	return ENAMETOOLONG;
      strcpy (posix_path, mi.posix_path);
      if (addslash || (!nextchar && append_slash))
	strcat (posix_path, "/");
      if (nextchar)
	slashify (p,
		  posix_path + addslash + (mi.posix_pathlen == 1
		  ? 0 : mi.posix_pathlen),
		  append_slash);

      if (cygheap->root.exists ())
	{
	  const char *p = cygheap->root.unchroot (posix_path);
	  memmove (posix_path, p, strlen (p) + 1);
	}
      goto out;
    }

  if (!cygheap->root.exists ())
    /* nothing */;
  else if (!cygheap->root.ischroot_native (pathbuf))
    return ENOENT;
  else
    {
      const char *p = pathbuf + cygheap->root.native_length ();
      if (*p)
	slashify (p, posix_path, append_slash);
      else
	{
	  posix_path[0] = '/';
	  posix_path[1] = '\0';
	}
      goto out;
    }

  /* Not in the database.  This should [theoretically] only happen if either
     the path begins with //, or / isn't mounted, or the path has a drive
     letter not covered by the mount table.  If it's a relative path then the
     caller must want an absolute path (otherwise we would have returned
     above).  So we always return an absolute path at this point. */
  if (isdrive (pathbuf))
    cygdrive_posix_path (pathbuf, posix_path, append_slash | ccp_flags);
  else
    {
      /* The use of src_path and not pathbuf here is intentional.
	 We couldn't translate the path, so just ensure no \'s are present. */
      slashify (src_path, posix_path, append_slash);
    }

out:
  debug_printf ("%s = conv_to_posix_path (%s)", posix_path, src_path);
  return 0;
}

/* Return flags associated with a mount point given the win32 path. */

unsigned
mount_info::set_flags_from_win32_path (const char *p)
{
  for (int i = 0; i < nmounts; i++)
    {
      mount_item &mi = mount[native_sorted[i]];
      if (path_prefix_p (mi.native_path, p, mi.native_pathlen,
			 mi.flags & MOUNT_NOPOSIX))
	return mi.flags;
    }
  return PATH_BINARY;
}

inline char *
skip_ws (char *in)
{
  while (*in == ' ' || *in == '\t')
    ++in;
  return in;
}

inline char *
find_ws (char *in)
{
  while (*in && *in != ' ' && *in != '\t')
    ++in;
  return in;
}

inline char *
conv_fstab_spaces (char *field)
{
  register char *sp = field;
  while ((sp = strstr (sp, "\\040")) != NULL)
    {
      *sp++ = ' ';
      memmove (sp, sp + 3, strlen (sp + 3) + 1);
    }
  return field;
}

struct opt
{
  const char *name;
  unsigned val;
  bool clear;
} oopts[] =
{
  {"acl", MOUNT_NOACL, 1},
  {"auto", 0, 0},
  {"binary", MOUNT_BINARY, 0},
  {"bind", MOUNT_BIND, 0},
  {"cygexec", MOUNT_CYGWIN_EXEC, 0},
  {"dos", MOUNT_DOS, 0},
  {"exec", MOUNT_EXEC, 0},
  {"ihash", MOUNT_IHASH, 0},
  {"noacl", MOUNT_NOACL, 0},
  {"nosuid", 0, 0},
  {"notexec", MOUNT_NOTEXEC, 0},
  {"nouser", MOUNT_SYSTEM, 0},
  {"override", MOUNT_OVERRIDE, 0},
  {"posix=0", MOUNT_NOPOSIX, 0},
  {"posix=1", MOUNT_NOPOSIX, 1},
  {"sparse", MOUNT_SPARSE, 0},
  {"text", MOUNT_BINARY, 1},
  {"user", MOUNT_SYSTEM, 1}
};

static int
compare_flags (const void *a, const void *b)
{
  const opt *oa = (const opt *) a;
  const opt *ob = (const opt *) b;

  return strcmp (oa->name, ob->name);
}

extern "C" bool
fstab_read_flags (char **options, unsigned &flags, bool external)
{
  opt key;

  while (**options)
    {
      char *p = strchr (*options, ',');
      if (p)
	*p++ = '\0';
      else
	p = strchr (*options, '\0');

      key.name = *options;
      opt *o = (opt *) bsearch (&key, oopts,
				sizeof oopts / sizeof (opt),
				sizeof (opt), compare_flags);
      if (!o)
	{
	  if (!external)
	    system_printf ("invalid fstab option - '%s'", *options);
	  return false;
	}
      if (o->clear)
	flags &= ~o->val;
      else
	flags |= o->val;
      *options = p;
    }
  return true;
}

extern "C" char *
fstab_list_flags ()
{
  size_t len = 0;
  opt *o;

  for (o = oopts; o < (oopts + (sizeof (oopts) / sizeof (oopts[0]))); o++)
    len += strlen (o->name) + 1;
  char *buf = (char *) malloc (len);
  if (buf)
    {
      char *bp = buf;
      for (o = oopts; o < (oopts + (sizeof (oopts) / sizeof (oopts[0]))); o++)
	{
	  bp = stpcpy (bp, o->name);
	  *bp++ = ',';
	}
      *--bp = '\0';
    }
  return buf;
}

bool
mount_info::from_fstab_line (char *line, bool user)
{
  char *native_path, *posix_path, *fs_type;

  /* First field: Native path. */
  char *c = skip_ws (line);
  if (!*c || *c == '#')
    return true;
  char *cend = find_ws (c);
  *cend = '\0';
  native_path = conv_fstab_spaces (c);
  /* Always convert drive letter to uppercase for case sensitivity. */
  if (isdrive (native_path))
    native_path[0] = cyg_toupper (native_path[0]);
  /* Second field: POSIX path. */
  c = skip_ws (cend + 1);
  if (!*c)
    return true;
  cend = find_ws (c);
  *cend = '\0';
  posix_path = conv_fstab_spaces (c);
  /* Third field: FS type. */
  c = skip_ws (cend + 1);
  if (!*c)
    return true;
  cend = find_ws (c);
  *cend = '\0';
  fs_type = c;
  /* Forth field: Flags. */
  c = skip_ws (cend + 1);
  if (!*c)
    return true;
  cend = find_ws (c);
  *cend = '\0';
  unsigned mount_flags = MOUNT_SYSTEM | MOUNT_BINARY;
  if (!strcmp (fs_type, "cygdrive"))
    mount_flags |= MOUNT_NOPOSIX;
  if (!strcmp (fs_type, "usertemp"))
    mount_flags |= MOUNT_IMMUTABLE;
  if (!fstab_read_flags (&c, mount_flags, false))
    return true;
  if (mount_flags & MOUNT_BIND)
    {
      /* Prepend root path to bound path. */
      char *bound_path = native_path;
      device dev;
      unsigned flags = 0;
      native_path = (char *) alloca (PATH_MAX);
      int error = conv_to_win32_path (bound_path, native_path, dev, &flags);
      if (error || strlen (native_path) >= MAX_PATH)
	return true;
      if ((mount_flags & ~MOUNT_SYSTEM) == (MOUNT_BIND | MOUNT_BINARY))
	mount_flags = (MOUNT_BIND | flags)
		      & ~(MOUNT_IMMUTABLE | MOUNT_AUTOMATIC);
    }
  if (user)
    mount_flags &= ~MOUNT_SYSTEM;
  if (!strcmp (fs_type, "cygdrive"))
    {
      cygdrive_flags = mount_flags | MOUNT_CYGDRIVE;
      slashify (posix_path, cygdrive, 1);
      cygdrive_len = strlen (cygdrive);
    }
  else if (!strcmp (fs_type, "usertemp"))
    {
      WCHAR tmp[PATH_MAX + 1];

      if (GetTempPathW (PATH_MAX, tmp))
	{
	  tmp_pathbuf tp;
	  char *mb_tmp = tp.c_get ();
	  sys_wcstombs (mb_tmp, PATH_MAX, tmp);

	  mount_flags |= MOUNT_USER_TEMP;
	  int res = mount_table->add_item (mb_tmp, posix_path, mount_flags);
	  if (res && get_errno () == EMFILE)
	    return false;
	}
    }
  else
    {
      int res = mount_table->add_item (native_path, posix_path, mount_flags);
      if (res && get_errno () == EMFILE)
	return false;
    }
  return true;
}

bool
mount_info::from_fstab (bool user, WCHAR fstab[], PWCHAR fstab_end)
{
  UNICODE_STRING upath;
  OBJECT_ATTRIBUTES attr;
  NT_readline rl;
  tmp_pathbuf tp;
  char *buf = tp.c_get ();

  if (user)
    {
      PWCHAR username;
      sys_mbstowcs (username = wcpcpy (fstab_end, L".d\\"),
		    NT_MAX_PATH - (fstab_end - fstab),
		    cygheap->user.name ());
      /* Make sure special chars in the username are converted according to
	 the rules. */
      transform_chars (username, username + wcslen (username) - 1);
    }
  RtlInitUnicodeString (&upath, fstab);
  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  debug_printf ("Try to read mounts from %W", fstab);
  if (rl.init (&attr, buf, NT_MAX_PATH))
    while ((buf = rl.gets ()))
      if (!from_fstab_line (buf, user))
	break;
  return true;
}

/* write_cygdrive_info: Store default prefix and flags
   to use when creating cygdrives to the special user shared mem
   location used to store cygdrive information. */

int
mount_info::write_cygdrive_info (const char *cygdrive_prefix, unsigned flags)
{
  /* Verify cygdrive prefix starts with a forward slash and if there's
     another character, it's not a slash. */
  if ((cygdrive_prefix == NULL) || (*cygdrive_prefix == 0) ||
      (!isslash (cygdrive_prefix[0])) ||
      ((cygdrive_prefix[1] != '\0') && (isslash (cygdrive_prefix[1]))))
    {
      set_errno (EINVAL);
      return -1;
    }
  /* Don't allow overriding of a system cygdrive prefix. */
  if (cygdrive_flags & MOUNT_SYSTEM)
    {
      set_errno (EPERM);
      return -1;
    }

  slashify (cygdrive_prefix, cygdrive, 1);
  cygdrive_flags = flags & ~MOUNT_SYSTEM;
  cygdrive_len = strlen (cygdrive);

  return 0;
}

int
mount_info::get_cygdrive_info (char *user, char *system, char *user_flags,
			       char *system_flags)
{
  if (user)
    *user = '\0';
  if (system)
    *system = '\0';
  if (user_flags)
    *user_flags = '\0';
  if (system_flags)
    *system_flags = '\0';

  char *path = (cygdrive_flags & MOUNT_SYSTEM) ? system : user;
  char *flags = (cygdrive_flags & MOUNT_SYSTEM) ? system_flags : user_flags;

  if (path)
    {
      strcpy (path, cygdrive);
      /* Strip trailing slash for backward compatibility. */
      if (cygdrive_len > 2)
	path[cygdrive_len - 1] = '\0';
    }
  if (flags)
    strcpy (flags, (cygdrive_flags & MOUNT_BINARY) ? "binmode" : "textmode");
  return 0;
}

static mount_item *mounts_for_sort;

/* sort_by_posix_name: qsort callback to sort the mount entries.  Sort
   user mounts ahead of system mounts to the same POSIX path. */
/* FIXME: should the user should be able to choose whether to
   prefer user or system mounts??? */
static int
sort_by_posix_name (const void *a, const void *b)
{
  mount_item *ap = mounts_for_sort + (*((int*) a));
  mount_item *bp = mounts_for_sort + (*((int*) b));

  /* Base weighting on longest posix path first so that the most
     obvious path will be chosen. */
  size_t alen = strlen (ap->posix_path);
  size_t blen = strlen (bp->posix_path);

  int res = blen - alen;

  if (res)
    return res;		/* Path lengths differed */

  /* The two paths were the same length, so just determine normal
     lexical sorted order. */
  res = strcmp (ap->posix_path, bp->posix_path);

  if (res == 0)
   {
     /* need to select between user and system mount to same POSIX path */
     if (!(bp->flags & MOUNT_SYSTEM))	/* user mount */
      return 1;
     else
      return -1;
   }

  return res;
}

/* sort_by_native_name: qsort callback to sort the mount entries.  Sort
   user mounts ahead of system mounts to the same POSIX path. */
/* FIXME: should the user should be able to choose whether to
   prefer user or system mounts??? */
static int
sort_by_native_name (const void *a, const void *b)
{
  mount_item *ap = mounts_for_sort + (*((int*) a));
  mount_item *bp = mounts_for_sort + (*((int*) b));

  /* Base weighting on longest win32 path first so that the most
     obvious path will be chosen. */
  size_t alen = strlen (ap->native_path);
  size_t blen = strlen (bp->native_path);

  int res = blen - alen;

  if (res)
    return res;		/* Path lengths differed */

  /* The two paths were the same length, so just determine normal
     lexical sorted order. */
  res = strcmp (ap->native_path, bp->native_path);

  if (res == 0)
   {
     /* need to select between user and system mount to same POSIX path */
     if (!(bp->flags & MOUNT_SYSTEM))	/* user mount */
      return 1;
     else
      return -1;
   }

  return res;
}

void
mount_info::sort ()
{
  for (int i = 0; i < nmounts; i++)
    native_sorted[i] = posix_sorted[i] = i;
  /* Sort them into reverse length order, otherwise we won't
     be able to look for /foo in /.  */
  mounts_for_sort = mount;	/* ouch. */
  qsort (posix_sorted, nmounts, sizeof (posix_sorted[0]), sort_by_posix_name);
  qsort (native_sorted, nmounts, sizeof (native_sorted[0]), sort_by_native_name);
}

/* Add an entry to the mount table.
   Returns 0 on success, -1 on failure and errno is set.

   This is where all argument validation is done.  It may not make sense to
   do this when called internally, but it's cleaner to keep it all here.  */

int
mount_info::add_item (const char *native, const char *posix,
		      unsigned mountflags)
{
  tmp_pathbuf tp;
  char *nativetmp = tp.c_get ();
  /* FIXME: The POSIX path is stored as value name right now, which is
     restricted to 256 bytes. */
  char posixtmp[CYG_MAX_PATH];
  char *nativetail, *posixtail, error[] = "error";
  int nativeerr, posixerr;

  /* Something's wrong if either path is NULL or empty, or if it's
     not a UNC or absolute path. */

  if (native == NULL || !isabspath (native) ||
      !(is_native_path (native) || is_unc_share (native) || isdrive (native)))
    nativeerr = EINVAL;
  else
    nativeerr = normalize_win32_path (native, nativetmp, nativetail);

  if (posix == NULL || !isabspath (posix) ||
      is_unc_share (posix) || isdrive (posix))
    posixerr = EINVAL;
  else
    posixerr = normalize_posix_path (posix, posixtmp, posixtail);

  debug_printf ("%s[%s], %s[%s], %y",
		native, nativeerr ? error : nativetmp,
		posix, posixerr ? error : posixtmp, mountflags);

  if (nativeerr || posixerr)
    {
      set_errno (nativeerr ?: posixerr);
      return -1;
    }

  /* Make sure both paths do not end in /. */
  if (nativetail > nativetmp + 1 && nativetail[-1] == '\\')
    nativetail[-1] = '\0';
  if (posixtail > posixtmp + 1 && posixtail[-1] == '/')
    posixtail[-1] = '\0';

  /* Write over an existing mount item with the same POSIX path if
     it exists and is from the same registry area. */
  int i;
  for (i = 0; i < nmounts; i++)
    {
      if (!strcmp (mount[i].posix_path, posixtmp))
	{
	  /* Don't allow overriding of a system mount with a user mount. */
	  if ((mount[i].flags & MOUNT_SYSTEM) && !(mountflags & MOUNT_SYSTEM))
	    {
	      set_errno (EPERM);
	      return -1;
	    }
	  if ((mount[i].flags & MOUNT_SYSTEM) != (mountflags & MOUNT_SYSTEM))
	    continue;
	  else if (!(mount[i].flags & MOUNT_IMMUTABLE))
	    break;
	  else if (mountflags & MOUNT_OVERRIDE)
	    {
	      mountflags |= MOUNT_IMMUTABLE;
	      break;
	    }
	  else
	    {
	      set_errno (EPERM);
	      return -1;
	    }
	}
    }

  if (i == nmounts && nmounts == MAX_MOUNTS)
    {
      set_errno (EMFILE);
      return -1;
    }

  if (i == nmounts)
    nmounts++;

  if (strcmp (posixtmp, "/usr/bin") == 0)
    got_usr_bin = true;

  if (strcmp (posixtmp, "/usr/lib") == 0)
    got_usr_lib = true;

  if (posixtmp[0] == '/' && posixtmp[1] == '\0' && !(mountflags & MOUNT_CYGDRIVE))
    root_idx = i;

  mount[i].init (nativetmp, posixtmp, mountflags);
  sort ();

  return 0;
}

/* Delete a mount table entry where path is either a Win32 or POSIX
   path. Since the mount table is really just a table of aliases,
   deleting / is ok (although running without a slash mount is
   strongly discouraged because some programs may run erratically
   without one).  If MOUNT_SYSTEM is set in flags, remove from system
   registry, otherwise remove the user registry mount.
*/

int
mount_info::del_item (const char *path, unsigned flags)
{
  tmp_pathbuf tp;
  char *pathtmp = tp.c_get ();
  int posix_path_p = false;

  /* Something's wrong if path is NULL or empty. */
  if (path == NULL || *path == 0 || !isabspath (path))
    {
      set_errno (EINVAL);
      return -1;
    }

  if (is_unc_share (path) || strpbrk (path, ":\\"))
    backslashify (path, pathtmp, 0);
  else
    {
      slashify (path, pathtmp, 0);
      posix_path_p = true;
    }
  nofinalslash (pathtmp, pathtmp);

  for (int i = 0; i < nmounts; i++)
    {
      int ent = native_sorted[i]; /* in the same order as getmntent() */
      if (((posix_path_p)
	   ? !strcmp (mount[ent].posix_path, pathtmp)
	   : strcasematch (mount[ent].native_path, pathtmp)))
	{
	  /* Don't allow removal of a system mount. */
	  if (mount[ent].flags & MOUNT_SYSTEM)
	    {
	      set_errno (EPERM);
	      return -1;
	    }
	  nmounts--; /* One less mount table entry */
	  /* Fill in the hole if not at the end of the table */
	  if (ent < nmounts)
	    memmove (mount + ent, mount + ent + 1,
		     sizeof (mount[ent]) * (nmounts - ent));
	  sort (); /* Resort the table */
	  return 0;
	}
    }
  set_errno (EINVAL);
  return -1;
}

/************************* mount_item class ****************************/

/* Don't add new fs types without adding them to fs_info_type in mount.h!
   Don't reorder without reordering fs_info_type in mount.h!*/
fs_names_t fs_names[] = {
    { "none", false },
    { "vfat", true },
    { "ntfs", true },
    { "refs", true },
    { "smbfs", false },
    { "nfs", false },
    { "netapp", false },
    { "iso9660", true },
    { "udf", true },
    { "csc-cache", false },
    { "unixfs", false },
    { "mvfs", false },
    { "cifs", false },
    { "nwfs", false },
    { "ncfsd", false },
    { "afs", false },
    { "prlfs", false },
    { NULL, false }
};

static mntent *
fillout_mntent (const char *native_path, const char *posix_path, unsigned flags)
{
  struct mntent& ret=_my_tls.locals.mntbuf;
  bool append_bs = false;

  /* Remove drivenum from list if we see a x: style path */
  if (strlen (native_path) == 2 && native_path[1] == ':')
    {
      int drivenum = cyg_tolower (native_path[0]) - 'a';
      if (drivenum >= 0 && drivenum <= 31)
	_my_tls.locals.available_drives &= ~(1 << drivenum);
      append_bs = true;
    }

  /* Pass back pointers to mount_table strings reserved for use by
     getmntent rather than pointers to strings in the internal mount
     table because the mount table might change, causing weird effects
     from the getmntent user's point of view. */

  ret.mnt_fsname = _my_tls.locals.mnt_fsname;
  strcpy (_my_tls.locals.mnt_dir, posix_path);
  ret.mnt_dir = _my_tls.locals.mnt_dir;

  /* Try to give a filesystem type that matches what a Linux application might
     expect. Naturally, this is a moving target, but we can make some
     reasonable guesses for popular types. */

  fs_info mntinfo;
  tmp_pathbuf tp;
  UNICODE_STRING unat;
  tp.u_get (&unat);
  get_nt_native_path (native_path, unat, false);
  if (append_bs)
    RtlAppendUnicodeToString (&unat, L"\\");
  mntinfo.update (&unat, NULL);

  if (mntinfo.what_fs () > none && mntinfo.what_fs () < max_fs_type)
    strcpy (_my_tls.locals.mnt_type, fs_names[mntinfo.what_fs ()].name);
  else
    strcpy (_my_tls.locals.mnt_type, mntinfo.fsname ());

  ret.mnt_type = _my_tls.locals.mnt_type;

  slashify (native_path, _my_tls.locals.mnt_fsname, false);

  /* mnt_opts is a string that details mount params such as
     binary or textmode, or exec.  We don't print
     `silent' here; it's a magic internal thing. */

  if (!(flags & MOUNT_BINARY))
    strcpy (_my_tls.locals.mnt_opts, (char *) "text");
  else
    strcpy (_my_tls.locals.mnt_opts, (char *) "binary");

  if (flags & MOUNT_CYGWIN_EXEC)
    strcat (_my_tls.locals.mnt_opts, (char *) ",cygexec");
  else if (flags & MOUNT_EXEC)
    strcat (_my_tls.locals.mnt_opts, (char *) ",exec");
  else if (flags & MOUNT_NOTEXEC)
    strcat (_my_tls.locals.mnt_opts, (char *) ",notexec");

  if (flags & MOUNT_NOACL)
    strcat (_my_tls.locals.mnt_opts, (char *) ",noacl");

  if (flags & MOUNT_DOS)
    strcat (_my_tls.locals.mnt_opts, (char *) ",dos");

  if (flags & MOUNT_IHASH)
    strcat (_my_tls.locals.mnt_opts, (char *) ",ihash");

  if (flags & MOUNT_NOPOSIX)
    strcat (_my_tls.locals.mnt_opts, (char *) ",posix=0");

  if (flags & MOUNT_SPARSE)
    strcat (_my_tls.locals.mnt_opts, (char *) ",sparse");

  if (!(flags & MOUNT_SYSTEM))		/* user mount */
    strcat (_my_tls.locals.mnt_opts, (char *) ",user");

  if (flags & MOUNT_CYGDRIVE)		/* cygdrive */
    strcat (_my_tls.locals.mnt_opts, (char *) ",noumount");

  if (flags & (MOUNT_AUTOMATIC | MOUNT_CYGDRIVE))
    strcat (_my_tls.locals.mnt_opts, (char *) ",auto");

  if (flags & (MOUNT_BIND))
    strcat (_my_tls.locals.mnt_opts, (char *) ",bind");

  if (flags & (MOUNT_USER_TEMP))
    strcat (_my_tls.locals.mnt_opts, (char *) ",usertemp");

  ret.mnt_opts = _my_tls.locals.mnt_opts;

  ret.mnt_freq = 1;
  ret.mnt_passno = 1;
  return &ret;
}

struct mntent *
mount_item::getmntent ()
{
  return fillout_mntent (native_path, posix_path, flags);
}

static struct mntent *
cygdrive_getmntent ()
{
  char native_path[4];
  char posix_path[CYG_MAX_PATH];
  DWORD mask = 1, drive = 'a';
  struct mntent *ret = NULL;

  while (_my_tls.locals.available_drives)
    {
      for (/* nothing */; drive <= 'z'; mask <<= 1, drive++)
	if (_my_tls.locals.available_drives & mask)
	  break;

      __small_sprintf (native_path, "%c:\\", cyg_toupper (drive));
      if (GetFileAttributes (native_path) == INVALID_FILE_ATTRIBUTES)
	{
	  _my_tls.locals.available_drives &= ~mask;
	  continue;
	}
      native_path[2] = '\0';
      __small_sprintf (posix_path, "%s%c", mount_table->cygdrive, drive);
      ret = fillout_mntent (native_path, posix_path, mount_table->cygdrive_flags);
      break;
    }

  return ret;
}

struct mntent *
mount_info::getmntent (int x)
{
  if (x < 0 || x >= nmounts)
    return cygdrive_getmntent ();

  return mount[native_sorted[x]].getmntent ();
}

/* Fill in the fields of a mount table entry.  */

void
mount_item::init (const char *native, const char *posix, unsigned mountflags)
{
  strcpy ((char *) native_path, native);
  strcpy ((char *) posix_path, posix);

  native_pathlen = strlen (native_path);
  posix_pathlen = strlen (posix_path);

  flags = mountflags;
}

/********************** Mount System Calls **************************/

/* Mount table system calls.
   Note that these are exported to the application.  */

/* mount: Add a mount to the mount table in memory and to the registry
   that will cause paths under win32_path to be translated to paths
   under posix_path. */

extern "C" int
mount (const char *win32_path, const char *posix_path, unsigned flags)
{
  /* FIXME: Should we disallow setting MOUNT_SYSTEM in flags since it
     isn't really supported except from fstab? */
  int res = -1;

  __try
    {
      if (!*posix_path)
	set_errno (EINVAL);
      else if (strpbrk (posix_path, "\\:"))
	set_errno (EINVAL);
      else if (flags & MOUNT_CYGDRIVE) /* normal mount */
	{
	  /* When flags include MOUNT_CYGDRIVE, take this to mean that
	    we actually want to change the cygdrive prefix and flags
	    without actually mounting anything. */
	  res = mount_table->write_cygdrive_info (posix_path, flags);
	  win32_path = NULL;
	}
      else if (!*win32_path)
	set_errno (EINVAL);
      else
	{
	  char *w32_path = (char *) win32_path;
	  if (flags & MOUNT_BIND)
	    {
	      /* Prepend root path to bound path. */
	      tmp_pathbuf tp;
	      device dev;

	      unsigned conv_flags = 0;
	      const char *bound_path = w32_path;

	      w32_path = tp.c_get ();
	      int error = mount_table->conv_to_win32_path (bound_path, w32_path,
							   dev, &conv_flags);
	      if (error || strlen (w32_path) >= MAX_PATH)
		return true;
	      if ((flags & ~MOUNT_SYSTEM) == (MOUNT_BIND | MOUNT_BINARY))
		flags = (MOUNT_BIND | conv_flags)
			& ~(MOUNT_IMMUTABLE | MOUNT_AUTOMATIC);
	    }
	  /* Make sure all mounts are user mounts, even those added via
	     mount -a. */
	  flags &= ~MOUNT_SYSTEM;
	  res = mount_table->add_item (w32_path, posix_path, flags);
	}

      syscall_printf ("%R = mount(%s, %s, %y)",
		      res, win32_path, posix_path, flags);
    }
  __except (EFAULT) {}
  __endtry
  return res;
}

/* umount: The standard umount call only has a path parameter.  Since
   it is not possible for this call to specify whether to remove the
   mount from the user or global mount registry table, assume the user
   table. */

extern "C" int
umount (const char *path)
{
  __try
    {
      if (!*path)
	{
	  set_errno (EINVAL);
	  __leave;
	}
      return cygwin_umount (path, 0);
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

/* cygwin_umount: This is like umount but takes an additional flags
   parameter that specifies whether to umount from the user or system-wide
   registry area. */

extern "C" int
cygwin_umount (const char *path, unsigned flags)
{
  int res = -1;

  if (!(flags & MOUNT_CYGDRIVE))
    res = mount_table->del_item (path, flags & ~MOUNT_SYSTEM);

  syscall_printf ("%R = cygwin_umount(%s, %d)", res,  path, flags);
  return res;
}

#define is_dev(d,s)	wcsncmp((d),(s),sizeof(s) - 1)

disk_type
get_disk_type (LPCWSTR dos)
{
  WCHAR dev[MAX_PATH], *d = dev;
  if (!QueryDosDeviceW (dos, dev, MAX_PATH))
    return DT_NODISK;
  if (is_dev (dev, L"\\Device\\"))
    {
      d += 8;
      switch (*d)
	{
	case L'C':
	  if (is_dev (d, L"CdRom"))
	    return DT_CDROM;
	  break;
	case L'F':
	  if (is_dev (d, L"Floppy"))
	    return DT_FLOPPY;
	  break;
	case L'H':
	  if (is_dev (d, L"Harddisk"))
	    return DT_HARDDISK;
	  break;
	case L'L':
	  if (is_dev (d, L"LanmanRedirector\\"))
	    return DT_SHARE_SMB;
	  break;
	case L'M':
	  if (is_dev (d, L"MRxNfs\\"))
	    return DT_SHARE_NFS;
	  break;
	}
    }
  return DT_NODISK;
}

extern "C" FILE *
setmntent (const char *filep, const char *)
{
  _my_tls.locals.iteration = 0;
  _my_tls.locals.available_drives = GetLogicalDrives ();
  /* Filter floppy drives on A: and B: */
  if ((_my_tls.locals.available_drives & 1)
      && get_disk_type (L"A:") == DT_FLOPPY)
    _my_tls.locals.available_drives &= ~1;
  if ((_my_tls.locals.available_drives & 2)
      && get_disk_type (L"B:") == DT_FLOPPY)
    _my_tls.locals.available_drives &= ~2;
  return (FILE *) filep;
}

extern "C" struct mntent *
getmntent (FILE *)
{
  return mount_table->getmntent (_my_tls.locals.iteration++);
}

extern "C" struct mntent *
getmntent_r (FILE *, struct mntent *mntbuf, char *buf, int buflen)
{
  struct mntent *mnt = mount_table->getmntent (_my_tls.locals.iteration++);
  int fsname_len, dir_len, type_len, tmplen = buflen;

  if (!mnt)
    return NULL;

  fsname_len = strlen (mnt->mnt_fsname) + 1;
  dir_len = strlen (mnt->mnt_dir) + 1;
  type_len = strlen (mnt->mnt_type) + 1;

  snprintf (buf, buflen, "%s%c%s%c%s%c%s", mnt->mnt_fsname, '\0',
	    mnt->mnt_dir, '\0', mnt->mnt_type, '\0', mnt->mnt_opts);

  mntbuf->mnt_fsname = buf;
  tmplen -= fsname_len;
  mntbuf->mnt_dir = tmplen > 0 ? buf + fsname_len : (char *)"";
  tmplen -= dir_len;
  mntbuf->mnt_type = tmplen > 0 ? buf + fsname_len + dir_len : (char *)"";
  tmplen -= type_len;
  mntbuf->mnt_opts = tmplen > 0 ? buf + fsname_len + dir_len + type_len : (char *)"";
  mntbuf->mnt_freq = mnt->mnt_freq;
  mntbuf->mnt_passno = mnt->mnt_passno;
  return mntbuf;
}

extern "C" int
endmntent (FILE *)
{
  return 1;
}

dos_drive_mappings::dos_drive_mappings ()
: mappings(0)
{
  tmp_pathbuf tp;
  wchar_t vol[64]; /* Long enough for Volume GUID string */
  wchar_t *devpath = tp.w_get ();
  wchar_t *mounts = tp.w_get ();

  /* Iterate over all volumes, fetch the first path from the list of
     DOS paths the volume is mounted to, or use the GUID volume path
     otherwise. */
  HANDLE sh = FindFirstVolumeW (vol, 64);
  if (sh == INVALID_HANDLE_VALUE)
    debug_printf ("FindFirstVolumeW, %E");
  else {
    do
      {
	/* Skip drives which are not mounted. */
	DWORD len;
	if (!GetVolumePathNamesForVolumeNameW (vol, mounts, NT_MAX_PATH, &len)
	    || mounts[0] == L'\0')
	  continue;
	*wcsrchr (vol, L'\\') = L'\0';
	if (QueryDosDeviceW (vol + 4, devpath, NT_MAX_PATH))
	  {
	    /* The DOS drive mapping can be another symbolic link.  If so,
	       the mapping won't work since the section name is the name
	       after resolving all symlinks.  Resolve symlinks here, too. */
	    for (int syml_cnt = 0; syml_cnt < SYMLOOP_MAX; ++syml_cnt)
	      {
		UNICODE_STRING upath;
		OBJECT_ATTRIBUTES attr;
		NTSTATUS status;
		HANDLE h;

		RtlInitUnicodeString (&upath, devpath);
		InitializeObjectAttributes (&attr, &upath,
					    OBJ_CASE_INSENSITIVE, NULL, NULL);
		status = NtOpenSymbolicLinkObject (&h, SYMBOLIC_LINK_QUERY,
						   &attr);
		if (!NT_SUCCESS (status))
		  break;
		RtlInitEmptyUnicodeString (&upath, devpath, (NT_MAX_PATH - 1)
							    * sizeof (WCHAR));
		status = NtQuerySymbolicLinkObject (h, &upath, NULL);
		NtClose (h);
		if (!NT_SUCCESS (status))
		  break;
		devpath[upath.Length / sizeof (WCHAR)] = L'\0';
	      }
	    mapping *m = new mapping ();
	    if (m)
	      {
		m->dospath = wcsdup (mounts);
		m->ntdevpath = wcsdup (devpath);
		if (!m->dospath || !m->ntdevpath)
		  {
		    free (m->dospath);
		    free (m->ntdevpath);
		    delete m;
		    continue;
		  }
		m->doslen = wcslen (m->dospath);
		m->dospath[--m->doslen] = L'\0'; /* Drop trailing backslash */
		m->ntlen = wcslen (m->ntdevpath);
		m->next = mappings;
		mappings = m;
	      }
	  }
	else
	  debug_printf ("Unable to determine the native mapping for %ls "
			"(error %u)", vol, GetLastError ());
      }
    while (FindNextVolumeW (sh, vol, 64));
    FindVolumeClose (sh);
  }
}

wchar_t *
dos_drive_mappings::fixup_if_match (wchar_t *path)
{
  /* Check for network drive first. */
  if (!wcsncmp (path, L"\\Device\\Mup\\", 12))
    {
      path += 10;
      path[0] = L'\\';
      return path;
    }
  /* Then test local drives. */
  for (mapping *m = mappings; m; m = m->next)
    if (!wcsncmp (m->ntdevpath, path, m->ntlen))
      {
	wchar_t *tmppath;

	if (m->ntlen > m->doslen)
	  wcsncpy (path += m->ntlen - m->doslen, m->dospath, m->doslen);
	else if ((tmppath = wcsdup (path + m->ntlen)) != NULL)
	  {
	    wcpcpy (wcpcpy (path, m->dospath), tmppath);
	    free (tmppath);
	  }
	break;
      }
  return path;
}

dos_drive_mappings::~dos_drive_mappings ()
{
  mapping *n = 0;
  for (mapping *m = mappings; m; m = n)
    {
      n = m->next;
      free (m->dospath);
      free (m->ntdevpath);
      delete m;
    }
}
