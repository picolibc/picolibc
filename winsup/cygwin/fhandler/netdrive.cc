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
#include "tls_pbuf.h"

#define USE_SYS_TYPES_FD_SET
#include <shobjidl.h>
#include <shlobj.h>
#include <lm.h>
#include <ws2tcpip.h>

#include <stdlib.h>
#include <dirent.h>
#include <wctype.h>

/* SMBv1 is deprectated and not even installed by default anymore on
   Windows 10 and 11 machines or their servers.

   As a result, neither WNetOpenEnumW() nor NetServerEnum() work as
   expected anymore.

   So this fhandler class now uses Network Discovery to enumerate
   the "//" directory, which, unfortunately, requires to use the
   shell API. */

/* There's something REALLY fishy going on in Windows.  If the NFS
   enumeration via WNet functions is called *before* the share enumeration
   via Shell function, the Shell functions will enumerate the NFS shares
   instead of the SMB shares.  Un-be-lie-va-ble!
   FWIW, we reverted the SMB share enumeration using WNet. */

#ifndef WNNC_NET_9P
#define WNNC_NET_9P 0x00480000
#endif
#define TERMSRV_DIR "tsclient"
#define PLAN9_DIR   "wsl$"

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
  size_t count () const { return num_entries; }
  void add (const wchar_t *str, bool downcase = false)
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
	if (downcase)
	  for (wchar_t *p = entry[num_entries]; (*p = towlower (*p)); ++p)
	    ;
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
  DWORD provider;
  HANDLE sem;
};

static inline int
hresult_to_errno (HRESULT wres)
{
  if (SUCCEEDED (wres))
    return 0;
  if (((ULONG) wres & 0xffff0000)
      == (ULONG) MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
    return geterrno_from_win_error ((ULONG) wres & 0xffff);
  return EACCES;
}

/* Workaround incompatible definitions. */
#define u_long		__ms_u_long
#define WS_FIONBIO	_IOW('f', 126, u_long)
#define WS_POLLOUT	0x10

static bool
server_is_running_nfs (const wchar_t *servername)
{
  /* Hack alarm: Only test TCP port 2049 */
  struct addrinfoW hints = { 0 };
  struct addrinfoW *ai = NULL, *aip;
  bool ret = false;
  INT wres;

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  /* The services contains "nfs" only as UDP service... sigh. */
  wres = GetAddrInfoW (servername, L"2049", &hints, &ai);
  if (wres)
    return false;
  for (aip = ai; !ret && aip; aip = aip->ai_next)
    {
      SOCKET sock = ::socket (aip->ai_family, aip->ai_socktype,
			      aip->ai_flags);
      if (sock != INVALID_SOCKET)
	{
	  __ms_u_long nonblocking = 1;
	  ::ioctlsocket (sock, WS_FIONBIO, &nonblocking);
	  wres = ::connect (sock, aip->ai_addr, aip->ai_addrlen);
	  if (wres == 0)
	    ret = true;
	  else if (WSAGetLastError () == WSAEWOULDBLOCK)
	    {
	      WSAPOLLFD fds = { .fd = sock,
				.events = WS_POLLOUT };
	      wres = WSAPoll (&fds, 1, 1500L);
	      if (wres > 0 && fds.revents == WS_POLLOUT)
		ret = true;
	    }
	  ::closesocket (sock);
	}
    }
  FreeAddrInfoW (ai);
  return ret;
}

/* Use only to enumerate the Network top level. */
static DWORD
thread_netdrive_wsd (void *arg)
{
  netdriveinf *ndi = (netdriveinf *) arg;
  DIR *dir = ndi->dir;
  IEnumShellItems *netitem_enum;
  IShellItem *netparent;
  HRESULT wres;

  ReleaseSemaphore (ndi->sem, 1, NULL);

  wres = CoInitialize (NULL);
  if (FAILED (wres))
    {
      ndi->err = hresult_to_errno (wres);
      goto out;
    }

  wres = SHGetKnownFolderItem (FOLDERID_NetworkFolder, KF_FLAG_DEFAULT,
			       NULL, IID_PPV_ARGS (&netparent));
  if (FAILED (wres))
    {
      ndi->err = hresult_to_errno (wres);
      goto out;
    }

  wres = netparent->BindToHandler (NULL, BHID_StorageEnum,
				   IID_PPV_ARGS (&netitem_enum));
  if (FAILED (wres))
    {
      ndi->err = hresult_to_errno (wres);
      netparent->Release ();
      goto out;
    }

  netitem_enum->Reset ();
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

  do
    {
      IShellItem *netitem = NULL;
      LPWSTR item_name = NULL;

      wres = netitem_enum->Next (1, &netitem, NULL);
      if (wres == S_OK)
	{
	  if (netitem->GetDisplayName (SIGDN_PARENTRELATIVEPARSING,
					    &item_name) == S_OK)
	    {
	      /* Skip "\\" on server names and downcase */
	      DIR_cache.add (item_name + 2, true);
	      CoTaskMemFree (item_name);
	    }
	  netitem->Release ();
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
thread_netdrive_wnet (void *arg)
{
  netdriveinf *ndi = (netdriveinf *) arg;
  DIR *dir = ndi->dir;
  DWORD wres;

  size_t entry_cache_size = DIR_cache.count ();
  WCHAR provider[256], *dummy = NULL;
  wchar_t srv_name[CYG_MAX_PATH];
  NETRESOURCEW nri = { 0 };
  LPNETRESOURCEW nro;
  tmp_pathbuf tp;
  HANDLE dom = NULL;
  DWORD cnt, size;

  ReleaseSemaphore (ndi->sem, 1, NULL);

  wres = WNetGetProviderNameW (ndi->provider, provider, (size = 256, &size));
  if (wres != NO_ERROR)
    {
      ndi->err = geterrno_from_win_error (wres);
      goto out;
    }

  sys_mbstowcs (srv_name, CYG_MAX_PATH, dir->__d_dirname);
  srv_name[0] = L'\\';
  srv_name[1] = L'\\';

  if (ndi->provider == WNNC_NET_MS_NFS
      && !server_is_running_nfs (srv_name + 2))
    {
      ndi->err = ENOENT;
      goto out;
    }

  nri.lpRemoteName = srv_name;
  nri.lpProvider = provider;
  nri.dwType = RESOURCETYPE_DISK;
  nro = (LPNETRESOURCEW) tp.c_get ();
  wres = WNetGetResourceInformationW (&nri, nro,
				      (size = NT_MAX_PATH, &size), &dummy);
  if (wres != NO_ERROR)
    {
      ndi->err = geterrno_from_win_error (wres);
      goto out;
    }
  wres = WNetOpenEnumW (RESOURCE_GLOBALNET, RESOURCETYPE_DISK,
			RESOURCEUSAGE_ALL, nro, &dom);
  if (wres != NO_ERROR)
    {
      ndi->err = geterrno_from_win_error (wres);
      goto out;
    }

  while ((wres = WNetEnumResourceW (dom, (cnt = 1, &cnt), nro,
				    (size = NT_MAX_PATH, &size))) == NO_ERROR)
    {
      size_t cache_idx;

      /* Skip server name and trailing backslash */
      wchar_t *name = nro->lpRemoteName + 2;
      name = wcschr (name, L'\\');
      if (!name)
	continue;
      ++name;

      if (ndi->provider == WNNC_NET_MS_NFS)
	{
	  wchar_t *nm = name;
	  /* Convert from "ANSI embedded in widechar" to multibyte and convert
	     back to widechar. */
	  char mbname[wcslen (name) + 1];
	  char *mb = mbname;
	  while ((*mb++ = *nm++))
	    ;
	  sys_mbstowcs_alloc (&name, HEAP_NOTHEAP, mbname);
	  if (!name)
	    {
	      ndi->err = ENOMEM;
	      goto out;
	    }
	  /* NFS has deep links so convert embedded '\\' to '/' here */
	  for (wchar_t *bs = name; (bs = wcschr (bs, L'\\')); *bs++ = L'/')
	    ;
	}
      /* If we already collected shares, drop duplicates. */
      for (cache_idx = 0; cache_idx < entry_cache_size; ++ cache_idx)
	if (!wcscmp (name, DIR_cache[cache_idx]))	// wcscasecmp?
	  break;
      if (cache_idx >= entry_cache_size)
	DIR_cache.add (name);
      if (ndi->provider == WNNC_NET_MS_NFS)
	free (name);
    }
out:
  if (dom)
    WNetCloseEnum (dom);
  ReleaseSemaphore (ndi->sem, 1, NULL);
  return 0;
}

static DWORD
create_thread_and_wait (DIR *dir)
{
  netdriveinf ndi = { dir, 0, 0, NULL };
  cygthread *thr;

  /* For the Network root, fetch WSD info. */
  if (strlen (dir->__d_dirname) == 2)
    {
      WCHAR provider[256];
      DWORD size;

      ndi.provider = WNNC_NET_SMB;
      ndi.sem = CreateSemaphore (&sec_none_nih, 0, 2, NULL);
      thr = new cygthread (thread_netdrive_wsd, &ndi, "netdrive_wsd");
      if (thr->detach (ndi.sem))
	ndi.err = EINTR;
      CloseHandle (ndi.sem);
      /* Add wsl$ if Plan 9 is installed */
      if (WNetGetProviderNameW (WNNC_NET_9P, provider, (size = 256, &size))
	  == NO_ERROR)
	DIR_cache.add (__CONCAT(L,PLAN9_DIR));
      goto out;
    }

  /* For shares, use WNet functions. */

  /* Try NFS first, if the name contains a dot (i. e., supposedly is a FQDN
     as used in NFS server enumeration). */
  if (strchr (dir->__d_dirname, '.'))
    {
      ndi.provider = WNNC_NET_MS_NFS;
      ndi.sem = CreateSemaphore (&sec_none_nih, 0, 2, NULL);
      thr = new cygthread (thread_netdrive_wnet, &ndi, "netdrive_nfs");
      if (thr->detach (ndi.sem))
	ndi.err = EINTR;
      CloseHandle (ndi.sem);

      if (ndi.err == EINTR)
	goto out;

    }

  /* Eventually, try TERMSRV/P9/SMB via WNet for share enumeration,
     depending on "server" name. */
  if (!strcmp (dir->__d_dirname + 2, TERMSRV_DIR))
    ndi.provider = WNNC_NET_TERMSRV;
  else if (!strcmp (dir->__d_dirname + 2, PLAN9_DIR))
    ndi.provider = WNNC_NET_9P;
  else
    ndi.provider = WNNC_NET_SMB;
  ndi.sem = CreateSemaphore (&sec_none_nih, 0, 2, NULL);
  thr = new cygthread (thread_netdrive_wnet, &ndi, "netdrive_smb");
  if (thr->detach (ndi.sem))
    ndi.err = EINTR;
  CloseHandle (ndi.sem);

out:
  return DIR_cache.count() > 0 ? 0 : ndi.err;
}

virtual_ftype_t
fhandler_netdrive::exists ()
{
  if (strlen (get_name ()) == 2)
    return virt_rootdir;

  wchar_t name[MAX_PATH];
  struct addrinfoW *ai;
  INT ret;
  DWORD protocol = 0;

  /* Handle "tsclient" (Microsoft Terminal Services) and
     "wsl$" (Plan 9 Network Provider) explicitely.
     Both obviously don't resolve with GetAddrInfoW. */
  if (!strcmp (get_name () + 2, TERMSRV_DIR))
    protocol = WNNC_NET_TERMSRV;
  else if (!strcmp (get_name () + 2, PLAN9_DIR))
    protocol = WNNC_NET_9P;
  if (protocol)
    {
      WCHAR provider[256];
      DWORD size = 256;

      if (WNetGetProviderNameW (protocol, provider, &size) == NO_ERROR)
	return virt_directory;
      return virt_none;
    }
  /* Hopefully we are allowed to assume an IP network with existing name
     resolution these days.  Therefore, just try to resolve the name
     into IP addresses.  This may take up to about 3 secs if the name
     doesn't exist, or about 8 secs if DNS is unavailable. */
  sys_mbstowcs (name, CYG_MAX_PATH, get_name ());
  ret = GetAddrInfoW (name + 2, NULL, NULL, &ai);
  if (!ret)
    FreeAddrInfoW (ai);

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
  dir->__handle = (char *) new dir_cache ();
  if (dir && (ret = create_thread_and_wait (dir)))
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

  sys_wcstombs (de->d_name, sizeof de->d_name, DIR_cache[dir->__d_position]);
  if (strlen (dir->__d_dirname) == 2)
    de->d_ino = hash_path_name (get_ino (), de->d_name);
  else
    {
      char full[2 * CYG_MAX_PATH];
      char *s;

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
