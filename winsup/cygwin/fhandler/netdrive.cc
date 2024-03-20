/* fhandler_netdrive.cc: fhandler for // and //MACHINE handling

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygthread.h"

#include <shobjidl.h>
#include <shlobj.h>
#include <lm.h>

#include <stdlib.h>
#include <dirent.h>
#include <wctype.h>

/* SMBv1 is deprectated and not even installed by default anymore on
   Windows 10 and 11 machines or their servers.

   So this fhandler class now uses Network Discovery, which, unfortunately,
   requires to use the shell API. */

/* Define the required GUIDs here to avoid linking with libuuid.a */
const GUID FOLDERID_NetworkFolder = {
  0xd20beec4, 0x5ca8, 0x4905,
  { 0xae, 0x3b, 0xbf, 0x25, 0x1e, 0xa0, 0x9b, 0x53 }
};

const GUID BHID_StorageEnum = {
  0x4621a4e3, 0xf0d6, 0x4773,
  { 0x8a, 0x9c, 0x46, 0xe7, 0x7b, 0x17, 0x48, 0x40 }
};

const GUID BHID_EnumItems = {
  0x94f60519, 0x2850, 0x4924,
  { 0xaa, 0x5a, 0xd1, 0x5e, 0x84, 0x86, 0x80, 0x39 }
};

class dir_cache
{
  size_t max_entries;
  size_t num_entries;
  wchar_t **entry;
public:
  dir_cache () : max_entries (0), num_entries (0), entry (NULL) {}
  ~dir_cache ()
  {
    while (num_entries > 0)
      free (entry[--num_entries]);
    free (entry);
  }
  void add (wchar_t *str)
  {
    if (num_entries >= max_entries)
      {
	wchar_t **newentry;

	newentry = (wchar_t **) realloc (entry, (max_entries + 10)
						* sizeof (wchar_t *));
	if (!newentry)
	  return;
	entry = newentry;
	max_entries += 10;
      }
    entry[num_entries] = wcsdup (str);
    if (entry[num_entries])
      {
	wchar_t *p = entry[num_entries];
	while ((*p = towlower (*p)))
	  ++p;
	++num_entries;
      }
  }
  inline wchar_t *operator [](size_t idx) const
  {
    if (idx < num_entries)
      return entry[idx];
    return NULL;
  }
};

#define DIR_cache	(*reinterpret_cast<dir_cache *> (dir->__handle))

struct netdriveinf
{
  DIR *dir;
  int err;
  bool test_only;
  HANDLE sem;
};

static inline int
hresult_to_errno (HRESULT wres, bool test_only = false)
{
  if (SUCCEEDED (wres))
    return 0;
  /* IEnumShellItems::Reset returns E_NOTIMPL when called for share
     enumeration.  However, if the machine doesn't exist, the Win32
     error ERROR_BAD_NETPATH (converted into a HRESULT) is returned.  In
     test_only mode, we exploit this.  Also, E_ACCESSDENIED is a funny
     one.  It means, the machine exists, you just have no right to
     access the share list, or SMB doesn't run. */
  if (test_only && (wres == E_NOTIMPL || wres == E_ACCESSDENIED))
    return 0;
  if (((ULONG) wres & 0xffff0000)
      == (ULONG) MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
    return geterrno_from_win_error ((ULONG) wres & 0xffff);
  return EACCES;
}

static DWORD
thread_netdrive (void *arg)
{
  netdriveinf *ndi = (netdriveinf *) arg;
  DIR *dir = ndi->dir;
  IEnumShellItems *netitem_enum;
  IShellItem *netparent;
  HRESULT wres;

  ReleaseSemaphore (ndi->sem, 1, NULL);

  size_t len = strlen (dir->__d_dirname);
  wres = CoInitialize (NULL);
  if (FAILED (wres))
    {
      ndi->err = hresult_to_errno (wres);
      goto out;
    }

  if (len == 2) /* // */
    {
      wres = SHGetKnownFolderItem (FOLDERID_NetworkFolder, KF_FLAG_DEFAULT,
				   NULL, IID_PPV_ARGS (&netparent));
      if (FAILED (wres))
	{
	  ndi->err = hresult_to_errno (wres);
	  goto out;
	}

    }
  else
    {
      wchar_t name[CYG_MAX_PATH];

      sys_mbstowcs (name, CYG_MAX_PATH, dir->__d_dirname);
      name[0] = L'\\';
      name[1] = L'\\';
      wres = SHCreateItemFromParsingName (name, NULL,
					  IID_PPV_ARGS (&netparent));
      if (FAILED (wres))
	{
	  ndi->err = hresult_to_errno (wres);
	  goto out;
	}

    }

  wres = netparent->BindToHandler (NULL, len == 2 ? BHID_StorageEnum
						  : BHID_EnumItems,
				   IID_PPV_ARGS (&netitem_enum));
  if (FAILED (wres))
    {
      ndi->err = hresult_to_errno (wres);
      netparent->Release ();
      goto out;
    }

  if (len == 2 || ndi->test_only)
    {
      wres = netitem_enum->Reset ();

      if (FAILED (wres) || ndi->test_only)
	{
	  ndi->err = hresult_to_errno (wres, ndi->test_only);
	  netitem_enum->Release ();
	  netparent->Release ();
	  goto out;
	}

      /* Don't look at me!

	 Network discovery is very unreliable and the list of machines
	 returned is just fly-by-night, if the enumerator doesn't have
	 enough time.  The fact that you see *most* (but not necessarily
	 *all*) machines on the network in Windows Explorer is a result of
	 the enumeration running in a loop.  You can observe this when
	 rebooting a remote machine and it disappears and reappears in the
	 Explorer Network list.

	 However, this is no option for the command line. We need to be able
	 to enumerate in a single go, since we can't just linger during
	 readdir() and reset the enumeration multiple times until we have a
	 supposedly full list.

	 This makes the following Sleep necessary.  Sleeping ~3secs after
	 Reset fills the enumeration with high probability with almost all
	 available machines. */
      Sleep (3000L);
    }

  dir->__handle = (char *) new dir_cache ();

  do
    {
      IShellItem *netitem[10] = { 0 };
      LPWSTR item_name = NULL;
      ULONG count;

      wres = netitem_enum->Next (10, netitem, &count);
      if (SUCCEEDED (wres) && count > 0)
	{
	  for (ULONG idx = 0; idx < count; ++idx)
	    {
	      if (netitem[idx]->GetDisplayName (SIGDN_PARENTRELATIVEPARSING,
						&item_name) == S_OK)
		{
		  DIR_cache.add (item_name);
		  CoTaskMemFree (item_name);
		}
	      netitem[idx]->Release ();
	    }
	}
    }
  while (wres == S_OK);

  netitem_enum->Release ();
  netparent->Release ();

  ndi->err = 0;

out:
  CoUninitialize ();
  ReleaseSemaphore (ndi->sem, 1, NULL);
  return 0;
}

static DWORD
create_thread_and_wait (DIR *dir, bool test_only)
{
  netdriveinf ndi = { dir, 0, test_only,
		      CreateSemaphore (&sec_none_nih, 0, 2, NULL) };

  cygthread *thr = new cygthread (thread_netdrive, &ndi, "netdrive");
  if (thr->detach (ndi.sem))
    ndi.err = EINTR;
  CloseHandle (ndi.sem);
  return ndi.err;
}

virtual_ftype_t
fhandler_netdrive::exists ()
{
  if (strlen (get_name ()) == 2)
    return virt_rootdir;

  DIR dir = { 0 };
  dir.__d_dirname = (char *) get_name ();
  int ret = create_thread_and_wait (&dir, true);

  return ret ? virt_none : virt_directory;
}

fhandler_netdrive::fhandler_netdrive ():
  fhandler_virtual ()
{
}

int
fhandler_netdrive::fstat (struct stat *buf)
{
  const char *path = get_name ();
  debug_printf ("fstat (%s)", path);

  fhandler_base::fstat (buf);

  buf->st_mode = S_IFDIR | STD_RBITS | STD_XBITS;
  buf->st_ino = get_ino ();

  return 0;
}

DIR *
fhandler_netdrive::opendir (int fd)
{
  DIR *dir;
  int ret;

  dir = fhandler_virtual::opendir (fd);
  if (dir && (ret = create_thread_and_wait (dir, false)))
    {
      free (dir->__d_dirname);
      free (dir->__d_dirent);
      free (dir);
      dir = NULL;
      set_errno (ret);
      syscall_printf ("%p = opendir (%s)", dir, get_name ());
    }
  return dir;
}

int
fhandler_netdrive::readdir (DIR *dir, dirent *de)
{
  int ret;

  if (!DIR_cache[dir->__d_position])
    {
      ret = ENMFILE;
      goto out;
    }

  if (strlen (dir->__d_dirname) == 2)
    {
      sys_wcstombs (de->d_name, sizeof de->d_name,
		    DIR_cache[dir->__d_position] + 2);
      de->d_ino = hash_path_name (get_ino (), de->d_name);
    }
  else
    {
      char full[2 * CYG_MAX_PATH];
      char *s;

      sys_wcstombs (de->d_name, sizeof de->d_name,
		    DIR_cache[dir->__d_position]);
      s = stpcpy (full, dir->__d_dirname);
      *s++ = '/';
      stpcpy (s, de->d_name);
      de->d_ino = readdir_get_ino (full, false);
    }
  dir->__d_position++;
  de->d_type = DT_DIR;
  ret = 0;

out:
  syscall_printf ("%d = readdir(%p, %p)", ret, dir, de);
  return ret;
}

void
fhandler_netdrive::seekdir (DIR *dir, long pos)
{
  ::rewinddir (dir);
  if (pos < 0)
    return;
  while (dir->__d_position < pos)
    if (readdir (dir, dir->__d_dirent))
      break;
}

void
fhandler_netdrive::rewinddir (DIR *dir)
{
  dir->__d_position = 0;
}

int
fhandler_netdrive::closedir (DIR *dir)
{
  if (dir->__handle != INVALID_HANDLE_VALUE)
    delete &DIR_cache;
  return fhandler_virtual::closedir (dir);
}

int
fhandler_netdrive::open (int flags, mode_t mode)
{
  if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
    {
      set_errno (EEXIST);
      return 0;
    }
  if (flags & O_WRONLY)
    {
      set_errno (EISDIR);
      return 0;
    }
  /* Open a fake handle to \\Device\\Null */
  return open_null (flags);
}

int
fhandler_netdrive::close ()
{
  /* Skip fhandler_virtual::close, which is a no-op. */
  return fhandler_base::close ();
}
