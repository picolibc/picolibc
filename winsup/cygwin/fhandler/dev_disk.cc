/* fhandler/dev_disk.cc: fhandler for the /dev/disk/by-id/... symlinks.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "path.h"
#include "fhandler.h"
#include "tls_pbuf.h"
#include <storduid.h>
#include <wctype.h>
#include <winioctl.h>

/* Replace spaces, non-printing and unexpected characters.  Remove
   leading and trailing spaces.  Return remaining string length. */
static int
sanitize_id_string (char *s)
{
  int first = 0;
  while (s[first] == ' ')
    first++;
  int last = -1, i;
  for (i = 0; s[first + i]; i++)
    {
      char c = s[first + i];
      if (c != ' ')
	last = -1;
      else if (last < 0)
	last = i;
      if (!(('0' <= c && c <= '9') || c == '.' || c == '-'
	  || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')))
	c = '_';
      else if (!first)
	continue;
      s[i] = c;
    }
  if (last < 0)
    last = i;
  s[last] = '\0';
  return last;
}

/* Fetch storage properties and create the ID string.
   returns: 1: success, 0: device ignored, -1: IoControl error. */
static int
storprop_to_id_name (HANDLE devhdl, const UNICODE_STRING *upath,
		     char *ioctl_buf, char (& name)[NAME_MAX + 1])
{
  DWORD bytes_read;
  STORAGE_PROPERTY_QUERY descquery =
    { StorageDeviceProperty, PropertyStandardQuery, { 0 } };
  if (!DeviceIoControl (devhdl, IOCTL_STORAGE_QUERY_PROPERTY,
			&descquery, sizeof (descquery),
			ioctl_buf, NT_MAX_PATH,
			&bytes_read, nullptr))
    {
      __seterrno_from_win_error (GetLastError ());
      debug_printf ("DeviceIoControl (%S, IOCTL_STORAGE_QUERY_PROPERTY,"
		    " {StorageDeviceProperty}): %E", upath);
      return -1;
    }

  const STORAGE_DEVICE_DESCRIPTOR *desc =
    reinterpret_cast<const STORAGE_DEVICE_DESCRIPTOR *>(ioctl_buf);
  int vendor_len = 0, product_len = 0, serial_len = 0;
  if (desc->VendorIdOffset)
    vendor_len = sanitize_id_string (ioctl_buf + desc->VendorIdOffset);
  if (desc->ProductIdOffset)
    product_len = sanitize_id_string (ioctl_buf + desc->ProductIdOffset);
  if (desc->SerialNumberOffset)
    serial_len = sanitize_id_string (ioctl_buf + desc->SerialNumberOffset);

  /* Ignore drive if information is empty or too long (should not happen). */
  if (!((vendor_len || product_len) && (20/*bustype*/ + vendor_len + 1 +
      product_len + 1 + serial_len + 1 + 34/*hash*/ + 1 + 10/*partN*/
      < (int) sizeof (name))))
    return 0;

  /* Translate bus types. */
  const char *bus;
  switch (desc->BusType)
    {
      case BusTypeAta:     bus = "ata-"; break;
      case BusTypeFibre:   bus = "fibre-"; break;
      case BusTypeNvme:    bus = "nvme-"; break;
      case BusTypeRAID:    bus = "raid-"; break;
      case BusTypeSas:     bus = "sas-"; break;
      case BusTypeSata:    bus = "sata-"; break;
      case BusTypeScsi:    bus = "scsi-"; break;
      case BusTypeUsb:     bus = "usb-"; break;
      case BusTypeVirtual: bus = "virtual-"; break;
      default:             bus = nullptr; break;
    }

  /* Create "BUSTYPE-[VENDOR_]PRODUCT[_SERIAL]" string. */
  char * cp = name;
  if (bus)
    cp = stpcpy (cp, bus);
  else
    cp += __small_sprintf (cp, "bustype%02_y-", desc->BusType);

  if (vendor_len)
    cp = stpcpy (cp, ioctl_buf + desc->VendorIdOffset);
  if (product_len)
    {
      if (vendor_len)
	cp = stpcpy (cp, "_");
      cp = stpcpy (cp, ioctl_buf + desc->ProductIdOffset);
    }
  if (serial_len)
    {
      cp = stpcpy (cp, "_");
      cp = stpcpy (cp, ioctl_buf + desc->SerialNumberOffset);
    }

  /* Add hash if information is too short (e.g. missing serial number). */
  bool add_hash = !(4 <= vendor_len + product_len && 4 <= serial_len);
  debug_printf ("%S: bustype: %02_y, add_hash: %d, id: '%s' '%s' '%s' ",
		upath, (unsigned) desc->BusType, (int) add_hash,
		(vendor_len ? ioctl_buf + desc->VendorIdOffset : ""),
		(product_len ? ioctl_buf + desc->ProductIdOffset : ""),
		(serial_len ? ioctl_buf + desc->SerialNumberOffset : ""));
  if (!add_hash)
    return 1;

  /* The call below also returns the STORAGE_DEVICE_DESCRIPTOR used above.
     MSDN documentation for STORAGE_DEVICE_UNIQUE_IDENTIFIER says:
     "The device descriptor contains IDs that are extracted from non-VPD
     inquiry data."  This may mean that the serial number (part of
     SCSI/SAS VPD data) may not always be provided.  Therefore a separate
     call to retrieve STORAGE_DEVICE_DESCRIPTOR only is done above. */
  STORAGE_PROPERTY_QUERY idquery =
    { StorageDeviceUniqueIdProperty, PropertyStandardQuery, { 0 } };
  if (!DeviceIoControl (devhdl, IOCTL_STORAGE_QUERY_PROPERTY,
			&idquery, sizeof (idquery),
			ioctl_buf, NT_MAX_PATH,
			&bytes_read, nullptr))
    {
      __seterrno_from_win_error (GetLastError ());
      debug_printf ("DeviceIoControl (%S, IOCTL_STORAGE_QUERY_PROPERTY,"
		    " {StorageDeviceUniqueIdProperty}): %E", upath);
      return -1;
    }

  /* Utilize the DUID as defined by MSDN to generate a hash. */
  const STORAGE_DEVICE_UNIQUE_IDENTIFIER *id =
    reinterpret_cast<const STORAGE_DEVICE_UNIQUE_IDENTIFIER *>(ioctl_buf);
  debug_printf ("STORAGE_DEVICE_UNIQUE_IDENTIFIER.Size: %u", id->Size);

   __int128 hash = 0;
   for (ULONG i = 0; i < id->Size; i++)
      hash = ioctl_buf[i] + (hash << 6) + (hash << 16) - hash;
  __small_sprintf (cp, "_%016_Y%016_X", (unsigned long long) (hash >> 64),
		   (unsigned long long) hash);
  return 1;
}

struct by_id_entry
{
  char name[NAME_MAX + 1];
  u_int8_t drive;
  u_int8_t part;
};

static int
by_id_compare_name (const void *a, const void *b)
{
  const by_id_entry *ap = reinterpret_cast<const by_id_entry *>(a);
  const by_id_entry *bp = reinterpret_cast<const by_id_entry *>(b);
  return strcmp (ap->name, bp->name);
}

static by_id_entry *
by_id_realloc (by_id_entry *p, size_t n)
{
  void *p2 = realloc (p, n * sizeof (*p));
  if (!p2)
    free (p);
  return reinterpret_cast<by_id_entry *>(p2);
}

static bool
format_partuuid (char *name, const PARTITION_INFORMATION_EX *pix)
{
  const GUID *guid;
  switch (pix->PartitionStyle)
    {
      case PARTITION_STYLE_MBR: guid = &pix->Mbr.PartitionId; break;
      case PARTITION_STYLE_GPT: guid = &pix->Gpt.PartitionId; break;
      default: return false;
    }

  if (pix->PartitionStyle == PARTITION_STYLE_MBR && !guid->Data2
      && !guid->Data3 && !guid->Data4[6] && !guid->Data4[7])
      /* MBR "GUID": SERIAL-0000-0000-00NN-NNNNNNN00000
	 Byte offset in LE order -----^^^^-^^^^^^^
	 Print as: SERIAL-OFFSET */
    __small_sprintf(name,
		    "%08_x-%02_x%02_x%02_x%02_x%02_x%02_x",
		    guid->Data1, guid->Data4[5], guid->Data4[4], guid->Data4[3],
		    guid->Data4[2], guid->Data4[1], guid->Data4[0]);
  else
    __small_sprintf(name,
		    "%08_x-%04_x-%04_x-%02_x%02_x-%02_x%02_x%02_x%02_x%02_x%02_x",
		    guid->Data1, guid->Data2, guid->Data3,
		    guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		    guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
   return true;
}

/* Create sorted name -> drive mapping table. Must be freed by caller. */
static int
get_by_id_table (by_id_entry * &table, fhandler_dev_disk::dev_disk_location loc)
{
  table = nullptr;

  /* Open \Device object directory. */
  wchar_t wpath[MAX_PATH] = L"\\Device";
  UNICODE_STRING upath = {14, 16, wpath};
  OBJECT_ATTRIBUTES attr;
  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
  HANDLE dirhdl;
  NTSTATUS status = NtOpenDirectoryObject (&dirhdl, DIRECTORY_QUERY, &attr);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtOpenDirectoryObject, status %y", status);
      __seterrno_from_nt_status (status);
      return -1;
    }

  /* Limit disk and partition numbers like fhandler_dev::readdir (). */
  const unsigned max_drive_num = 127, max_part_num = 63;

  unsigned alloc_size = 0, table_size = 0;
  tmp_pathbuf tp;
  char *ioctl_buf = tp.c_get ();
  DIRECTORY_BASIC_INFORMATION *dbi_buf =
    reinterpret_cast<DIRECTORY_BASIC_INFORMATION *>(tp.w_get ());

  /* Traverse \Device directory ... */
  bool errno_set = false;
  HANDLE devhdl = nullptr;
  BOOLEAN restart = TRUE;
  bool last_run = false;
  ULONG context = 0;
  while (!last_run)
    {
      if (devhdl)
	{
	  /* Close handle from previous run. */
	  NtClose(devhdl);
	  devhdl = nullptr;
	}

      status = NtQueryDirectoryObject (dirhdl, dbi_buf, 65536, FALSE, restart,
				       &context, nullptr);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  errno_set = true;
	  debug_printf ("NtQueryDirectoryObject, status %y", status);
	  break;
	}
      if (status != STATUS_MORE_ENTRIES)
	last_run = true;
      restart = FALSE;
      for (const DIRECTORY_BASIC_INFORMATION *dbi = dbi_buf;
	   dbi->ObjectName.Length > 0;
	   dbi++)
	{
	  /* ... and check for a "Harddisk[0-9]*" entry. */
	  if (dbi->ObjectName.Length < 9 * sizeof (WCHAR)
	      || wcsncasecmp (dbi->ObjectName.Buffer, L"Harddisk", 8) != 0
	      || !iswdigit (dbi->ObjectName.Buffer[8]))
	    continue;
	  /* Got it.  Now construct the path to the entire disk, which is
	     "\\Device\\HarddiskX\\Partition0", and open the disk with
	     minimum permissions. */
	  unsigned long drive_num = wcstoul (dbi->ObjectName.Buffer + 8,
					     nullptr, 10);
	  if (drive_num > max_drive_num)
	    continue;
	  wcscpy (wpath, dbi->ObjectName.Buffer);
	  PWCHAR wpart = wpath + dbi->ObjectName.Length / sizeof (WCHAR);
	  wcpcpy (wpart, L"\\Partition0");
	  upath.Length = dbi->ObjectName.Length + 22;
	  upath.MaximumLength = upath.Length + sizeof (WCHAR);
	  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE,
				      dirhdl, nullptr);
	  /* SYNCHRONIZE access is required for IOCTL_STORAGE_QUERY_PROPERTY
	     for drives behind some drivers (nvmestor.sys). */
	  IO_STATUS_BLOCK io;
	  status = NtOpenFile (&devhdl, READ_CONTROL | SYNCHRONIZE, &attr, &io,
			       FILE_SHARE_VALID_FLAGS, 0);
	  if (!NT_SUCCESS (status))
	    {
	      devhdl = nullptr;
	      __seterrno_from_nt_status (status);
	      errno_set = true;
	      debug_printf ("NtOpenFile(%S), status %y", &upath, status);
	      continue;
	    }

	  /* Add table space for drive, partitions and end marker. */
	  if (alloc_size <= table_size + max_part_num)
	    {
	      alloc_size = table_size + max_part_num + 8;
	      table = by_id_realloc (table, alloc_size);
	      if (!table)
		{
		  NtClose (devhdl);
		  NtClose (dirhdl);
		  return -1;
		}
	    }

	  const char *drive_name = "";
	  if (loc == fhandler_dev_disk::disk_by_id)
	    {
	      /* Fetch storage properties and create the ID string. */
	      int rc = storprop_to_id_name (devhdl, &upath, ioctl_buf,
					    table[table_size].name);
	      if (rc <= 0)
		{
		  if (rc < 0)
		    errno_set = true;
		  continue;
		}
	      drive_name = table[table_size].name;
	      table[table_size].drive = drive_num;
	      table[table_size].part = 0;
	      table_size++;
	    }

	  /* Fetch drive layout info to information of all partitions on disk. */
	  DWORD bytes_read;
	  if (!DeviceIoControl (devhdl, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, nullptr, 0,
			       ioctl_buf, NT_MAX_PATH, &bytes_read, nullptr))
	    {
	      __seterrno_from_win_error (GetLastError ());
	      errno_set = true;
	      debug_printf ("DeviceIoControl(%S, "
			    "IOCTL_DISK_GET_DRIVE_LAYOUT_EX): %E", &upath);
	      continue;
	    }

	  /* Loop over partitions. */
	  const DRIVE_LAYOUT_INFORMATION_EX *dlix =
	    reinterpret_cast<const DRIVE_LAYOUT_INFORMATION_EX *>(ioctl_buf);
	  for (DWORD i = 0; i < dlix->PartitionCount; i++)
	    {
	      const PARTITION_INFORMATION_EX *pix = &dlix->PartitionEntry[i];
	      DWORD part_num = pix->PartitionNumber;
	      /* A partition number of 0 denotes an extended partition or a
		 filler entry as described in
		 fhandler_dev_floppy::lock_partition.  Just skip. */
	      if (part_num == 0)
		continue;
	      if (part_num > max_part_num)
		break;

	      char *name = table[table_size].name;
	      switch (loc)
		{
		  case fhandler_dev_disk::disk_by_id:
		    __small_sprintf (name, "%s-part%u", drive_name, part_num);
		    break;

		  case fhandler_dev_disk::disk_by_partuuid:
		    if (!format_partuuid (name, pix))
		      continue;
		    break;

		  default: continue; /* Should not happen. */
		}
	      table[table_size].drive = drive_num;
	      table[table_size].part = part_num;
	      table_size++;
	    }
	}
    }
  if (devhdl)
    NtClose(devhdl);
  NtClose (dirhdl);

  if (!table_size && table)
    {
      free (table);
      table = nullptr;
    }
  if (!table)
    return (errno_set ? -1 : 0);

  /* Sort by name and remove duplicates. */
  qsort (table, table_size, sizeof (*table), by_id_compare_name);
  for (unsigned i = 0; i < table_size; i++)
    {
      unsigned j = i + 1;
      while (j < table_size && !strcmp (table[i].name, table[j].name))
	j++;
      if (j == i + 1)
	continue;
      /* Duplicate(s) found, remove all entries with this name. */
      debug_printf ("removing duplicates %d-%d: '%s'", i, j - 1, table[i].name);
      if (j < table_size)
	memmove (table + i, table + j, (table_size - j) * sizeof (*table));
      table_size -= j - i;
      i--;
    }

  debug_printf ("table_size: %d", table_size);
  return table_size;
}

const char dev_disk[] = "/dev/disk";
const size_t dev_disk_len = sizeof (dev_disk) - 1;
static const char by_id[] = "/by-id";
const size_t by_id_len = sizeof(by_id) - 1;
static const char by_partuuid[] = "/by-partuuid";
const size_t by_partuuid_len = sizeof(by_partuuid) - 1;

fhandler_dev_disk::fhandler_dev_disk ():
  fhandler_virtual (),
  loc (unknown_loc),
  loc_is_link (false),
  drive_from_id (-1),
  part_from_id (0)
{
}

void
fhandler_dev_disk::init_dev_disk ()
{
  if (loc != unknown_loc)
    return;

  /* Determine location. */
  const char *path = get_name ();
  size_t dirlen = 0;
  if (!path_prefix_p (dev_disk, path, dev_disk_len, false))
    loc = invalid_loc; // should not happen
  else if (!path[dev_disk_len])
    loc = disk_dir; // "/dev/disk"
  else if (path_prefix_p (by_id, path + dev_disk_len, by_id_len, false))
    {
      loc = disk_by_id; // "/dev/disk/by-id.."
      dirlen = dev_disk_len + by_id_len;
    }
  else if (path_prefix_p (by_partuuid, path + dev_disk_len, by_partuuid_len, false))
    {
      loc = disk_by_partuuid; // "/dev/disk/by-partuuid..."
      dirlen = dev_disk_len + by_partuuid_len;
    }
  else
      loc = invalid_loc; // "/dev/disk/invalid"

  loc_is_link = false;
  if (dirlen)
    {
      if (!path[dirlen])
	; // "/dev/disk/by-..."
      else if (!strchr (path + dirlen + 1, '/'))
	loc_is_link = true; // "/dev/disk/by-.../LINK"
      else
	loc = invalid_loc; // "/dev/disk/by-.../dir/invalid"
    }

  debug_printf ("%s: loc %d, loc_is_link %d", path, (int) loc, (int) loc_is_link);

  /* Done if directory or invalid. */
  if (!loc_is_link)
    return;

  /* Check whether "/dev/disk/by-.../LINK" exists. */
  by_id_entry *table;
  int table_size = get_by_id_table (table, loc);
  if (!table)
    {
      loc = invalid_loc;
      return;
    }

  by_id_entry key;
  strcpy (key.name, path + dirlen + 1);
  const void *found = bsearch (&key, table, table_size, sizeof (*table),
			       by_id_compare_name);
  if (found)
    {
      /* Preserve drive and partition numbers for fillbuf (). */
      const by_id_entry *e = reinterpret_cast<const by_id_entry *>(found);
      drive_from_id = e->drive;
      part_from_id = e->part;
    }
  else
    loc = invalid_loc;
  free (table);
}

virtual_ftype_t
fhandler_dev_disk::exists ()
{
  debug_printf ("exists (%s)", get_name ());
  ensure_inited ();
  if (loc == invalid_loc)
    return virt_none;
  else if (loc_is_link)
    return virt_symlink;
  else
    return virt_directory;
}

int
fhandler_dev_disk::fstat (struct stat *buf)
{
  debug_printf ("fstat (%s)", get_name ());
  ensure_inited ();
  if (loc == invalid_loc)
    {
      set_errno (ENOENT);
      return -1;
    }

  fhandler_base::fstat (buf);
  buf->st_mode = (loc_is_link ? S_IFLNK | S_IWUSR | S_IWGRP | S_IWOTH
		  : S_IFDIR) | STD_RBITS | STD_XBITS;
  buf->st_ino = get_ino ();
  return 0;
}

static inline by_id_entry **
dir_id_table (DIR *dir)
{
  return reinterpret_cast<by_id_entry **>(&dir->__d_internal);
}

DIR *
fhandler_dev_disk::opendir (int fd)
{
  ensure_inited ();
  if (loc_is_link)
    {
      set_errno (ENOTDIR);
      return nullptr;
    }

  by_id_entry *table = nullptr;
  if (loc != disk_dir)
    {
      int table_size = get_by_id_table (table, loc);
      if (table_size < 0)
	return nullptr; /* errno is set. */
      if (table)
	{
	  /* Shrink to required table_size. */
	  table = by_id_realloc (table, table_size + 1);
	  if (!table)
	    return nullptr; /* Should not happen. */
	  /* Mark end of table for readdir (). */
	  table[table_size].name[0] = '\0';
	}
    }

  DIR *dir = fhandler_virtual::opendir (fd);
  if (!dir)
    {
      free (table);
      return nullptr;
    }
  dir->__flags = dirent_saw_dot | dirent_saw_dot_dot;
  *dir_id_table (dir) = table;
  return dir;
}

int
fhandler_dev_disk::closedir (DIR *dir)
{
  free (*dir_id_table (dir));
  return fhandler_virtual::closedir (dir);
}

int
fhandler_dev_disk::readdir (DIR *dir, dirent *de)
{
  int res;
  if (dir->__d_position < 2)
    {
      de->d_name[0] = '.';
      de->d_name[1] = (dir->__d_position ? '.' : '\0');
      de->d_name[2] = '\0';
      de->d_type = DT_DIR;
      dir->__d_position++;
      res = 0;
    }
  else if (loc == disk_dir && dir->__d_position < 2 + 2)
    {
      static const char * const names[2] {by_id, by_partuuid};
      strcpy (de->d_name, names[dir->__d_position - 2] + 1);
      de->d_type = DT_DIR;
      dir->__d_position++;
      res = 0;
    }
  else if (*dir_id_table (dir))
    {
      const char *name = (*dir_id_table (dir))[dir->__d_position - 2].name;
      if (name[0])
	{
	  strcpy (de->d_name, name);
	  de->d_type = DT_LNK;
	  dir->__d_position++;
	  res = 0;
	}
      else
	res = ENMFILE;
    }
  else
    res = ENMFILE;

  syscall_printf ("%d = readdir(%p, %p) (%s)", res, dir, de,
		  (!res ? de->d_name : ""));
  return res;
}

int
fhandler_dev_disk::open (int flags, mode_t mode)
{
  ensure_inited ();
  int err = 0;
  if (!fhandler_virtual::open (flags, mode))
    err = -1;
  /* else if (loc_is_link) {} */ /* should not happen. */
  else if (loc != invalid_loc)
    {
      if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
	err = EEXIST;
      else if (flags & O_WRONLY)
	err = EISDIR;
      else
	diropen = true;
    }
  else
    {
      if (flags & O_CREAT)
	err = EROFS;
      else
	err = ENOENT;
    }

  int res;
  if (!err)
    {
      nohandle (true);
      set_open_status ();
      res = 1;
    }
  else
    {
      if (err > 0)
	set_errno (err);
      res = 0;
    }

  syscall_printf ("%d = fhandler_dev_disk::open(%y, 0%o)", res, flags, mode);
  return res;
}

bool
fhandler_dev_disk::fill_filebuf ()
{
  debug_printf ("fill_filebuf (%s)", get_name ());
  ensure_inited ();
  if (!(loc_is_link && drive_from_id >= 0))
    return false;

  char buf[32];
  int len;
  if (drive_from_id + 'a' <= 'z')
    len = __small_sprintf (buf, "../../sd%c", drive_from_id + 'a');
  else
    len = __small_sprintf (buf, "../../sd%c%c",
			   drive_from_id / ('z' - 'a' + 1) - 1 + 'a',
			   drive_from_id % ('z' - 'a' + 1) + 'a');
  if (part_from_id)
    __small_sprintf (buf + len, "%d", part_from_id);

  if (filebuf)
    cfree (filebuf);
  filebuf = cstrdup (buf);
  return true;
}
