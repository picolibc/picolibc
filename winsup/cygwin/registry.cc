/* registry.cc: registry interface

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "registry.h"
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
#include "ntdll.h"
#include <wchar.h>
#include <alloca.h>

/* Opens a key under the appropriate Cygwin key.
   Do not use HKCU per MS KB 199190  */
static NTSTATUS
top_key (bool isHKLM, REGSAM access, PHANDLE top)
{
  WCHAR rbuf[PATH_MAX], *p;
  UNICODE_STRING rpath;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  InitializeObjectAttributes (&attr, &rpath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  if (isHKLM)
    {
      wcpcpy (rbuf, L"\\Registry\\Machine");
      RtlInitUnicodeString (&rpath, rbuf);
      status = NtOpenKey (top, access, &attr);
    }
  else
    {
      WCHAR name[128];
      PCWSTR names[2] = {cygheap->user.get_windows_id (name),
			 L".DEFAULT"};

      p = wcpcpy (rbuf, L"\\Registry\\User\\");
      for (int i = 0; i < 2; i++)
	{
	  wcpcpy (p, names[i]);
	  RtlInitUnicodeString (&rpath, rbuf);
	  status = NtOpenKey (top, access, &attr);
	  if (NT_SUCCESS (status))
	    break;
	}
    }
  return status;
}

reg_key::reg_key (HKEY top, REGSAM access, ...): _disposition (0)
{
  va_list av;
  va_start (av, access);
  build_reg (top, access, av);
  va_end (av);
}

reg_key::reg_key (bool isHKLM, REGSAM access, ...): _disposition (0)
{
  va_list av;
  HANDLE top;

  key_is_invalid = top_key (isHKLM, access, &top);
  if (NT_SUCCESS (key_is_invalid))
    {
      new (this) reg_key ((HKEY) top, access, L"SOFTWARE",
			  _WIDE (CYGWIN_INFO_CYGWIN_REGISTRY_NAME), NULL);
      NtClose (top);
      if (key_is_invalid)
	return;
      top = key;
      va_start (av, access);
      build_reg ((HKEY) top, access, av);
      va_end (av);
      if (top != key)
	NtClose (top);
    }
}

void
reg_key::build_reg (HKEY top, REGSAM access, va_list av)
{
  PWCHAR name;
  HANDLE r;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  if (top != HKEY_LOCAL_MACHINE && top != HKEY_CURRENT_USER)
    r = (HANDLE) top;
  else if (!NT_SUCCESS (top_key (top == HKEY_LOCAL_MACHINE, access, &r)))
    return;
  key_is_invalid = 0;
  while ((name = va_arg (av, PWCHAR)) != NULL)
    {
      RtlInitUnicodeString (&uname, name);
      InitializeObjectAttributes (&attr, &uname,
				  OBJ_CASE_INSENSITIVE | OBJ_OPENIF, r, NULL);

      status = NtCreateKey (&key, access, &attr, 0, NULL,
			    REG_OPTION_NON_VOLATILE, &_disposition);
      if (r != (HANDLE) top)
	NtClose (r);
      r = key;
      if (!NT_SUCCESS (status))
	{
	  key_is_invalid = status;
	  debug_printf ("failed to create key %S in the registry", &uname);
	  break;
	}
    }
}

/* Given the current registry key, return the specific DWORD value
   requested.  Return def on failure. */

DWORD
reg_key::get_dword (PCWSTR name, DWORD def)
{
  if (key_is_invalid)
    return def;

  NTSTATUS status;
  UNICODE_STRING uname;
  ULONG size = sizeof (KEY_VALUE_PARTIAL_INFORMATION) + sizeof (DWORD);
  ULONG rsize;
  PKEY_VALUE_PARTIAL_INFORMATION vbuf = (PKEY_VALUE_PARTIAL_INFORMATION)
				      alloca (size);

  RtlInitUnicodeString (&uname, name);
  status = NtQueryValueKey (key, &uname, KeyValuePartialInformation, vbuf,
			    size, &rsize);
  if (status != STATUS_SUCCESS || vbuf->Type != REG_DWORD)
    return def;
  DWORD *dst = (DWORD *) vbuf->Data;
  return *dst;
}

/* Given the current registry key, set a specific DWORD value. */

NTSTATUS
reg_key::set_dword (PCWSTR name, DWORD val)
{
  if (key_is_invalid)
    return key_is_invalid;

  DWORD value = (DWORD) val;
  UNICODE_STRING uname;
  RtlInitUnicodeString (&uname, name);
  return NtSetValueKey (key, &uname, 0, REG_DWORD, &value, sizeof (value));
}

/* Given the current registry key, return the specific string value
   requested.  Return zero on success, non-zero on failure. */

NTSTATUS
reg_key::get_string (PCWSTR name, PWCHAR dst, size_t max, PCWSTR def)
{
  NTSTATUS status;

  if (key_is_invalid)
    {
      status = key_is_invalid;
      if (def != NULL)
	wcpncpy (dst, def, max);
    }
  else
    {
      UNICODE_STRING uname;
      ULONG size = sizeof (KEY_VALUE_PARTIAL_INFORMATION) + max * sizeof (WCHAR);
      ULONG rsize;
      PKEY_VALUE_PARTIAL_INFORMATION vbuf = (PKEY_VALUE_PARTIAL_INFORMATION)
					  alloca (size);

      RtlInitUnicodeString (&uname, name);
      status = NtQueryValueKey (key, &uname, KeyValuePartialInformation, vbuf,
				size, &rsize);
      if (status != STATUS_SUCCESS || vbuf->Type != REG_SZ)
	wcpncpy (dst, def, max);
      else
	wcpncpy (dst, (PWCHAR) vbuf->Data, max);
    }
  return status;
}

/* Given the current registry key, set a specific string value. */

NTSTATUS
reg_key::set_string (PCWSTR name, PCWSTR src)
{
  if (key_is_invalid)
    return key_is_invalid;

  UNICODE_STRING uname;
  RtlInitUnicodeString (&uname, name);
  return NtSetValueKey (key, &uname, 0, REG_SZ, (PVOID) src,
			(wcslen (src) + 1) * sizeof (WCHAR));
}

reg_key::~reg_key ()
{
  if (!key_is_invalid)
    NtClose (key);
  key_is_invalid = 1;
}

/* The buffer path points to should be at least MAX_PATH bytes. */
PWCHAR
get_registry_hive_path (PCWSTR name, PWCHAR path)
{
  if (!name || !path)
    return NULL;

  WCHAR key[256];
  UNICODE_STRING buf;
  tmp_pathbuf tp;
  tp.u_get (&buf);
  NTSTATUS status;

  RTL_QUERY_REGISTRY_TABLE tab[2] = {
    { NULL, RTL_QUERY_REGISTRY_NOEXPAND | RTL_QUERY_REGISTRY_DIRECT
	    | RTL_QUERY_REGISTRY_REQUIRED,
      L"ProfileImagePath", &buf, REG_NONE, NULL, 0 },
    { NULL, 0, NULL, NULL, 0, NULL, 0 }
  };
  wcpcpy (wcpcpy (key, L"ProfileList\\"), name);
  status = RtlQueryRegistryValues (RTL_REGISTRY_WINDOWS_NT, key, tab,
				   NULL, NULL);
  if (!NT_SUCCESS (status) || buf.Length == 0)
    {
      debug_printf ("ProfileImagePath for %W not found, status %y", name,
		    status);
      return NULL;
    }
  ExpandEnvironmentStringsW (buf.Buffer, path, MAX_PATH);
  debug_printf ("ProfileImagePath for %W: %W", name, path);
  return path;
}

void
load_registry_hive (PCWSTR name)
{
  if (!name)
    return;

  /* Fetch the path. Prepend native NT path prefix. */
  tmp_pathbuf tp;
  PWCHAR path = tp.w_get ();
  if (!get_registry_hive_path (name, wcpcpy (path, L"\\??\\")))
    return;

  WCHAR key[256];
  PWCHAR path_comp;
  UNICODE_STRING ukey, upath;
  OBJECT_ATTRIBUTES key_attr, path_attr;
  NTSTATUS status;

  /* Create keyname and path strings and object attributes. */
  wcpcpy (wcpcpy (key, L"\\Registry\\User\\"), name);
  RtlInitUnicodeString (&ukey, key);
  InitializeObjectAttributes (&key_attr, &ukey, OBJ_CASE_INSENSITIVE,
			      NULL, NULL);
  /* First try to load the "normal" registry hive, which is what the user
     is supposed to see under HKEY_CURRENT_USER. */
  path_comp = wcschr (path, L'\0');
  wcpcpy (path_comp, L"\\ntuser.dat");
  RtlInitUnicodeString (&upath, path);
  InitializeObjectAttributes (&path_attr, &upath, OBJ_CASE_INSENSITIVE,
			      NULL, NULL);
  status = NtLoadKey (&key_attr, &path_attr);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("Loading user registry hive %S into %S failed: %y",
		    &upath, &ukey, status);
      return;
    }
  debug_printf ("Loading user registry hive %S into %S SUCCEEDED: %y",
		&upath, &ukey, status);
  /* If loading the normal hive worked, try to load the classes hive into
     the sibling *_Classes subkey, which is what the user is supposed to
     see under HKEY_CLASSES_ROOT, merged with the machine-wide classes. */
  wcscat (key, L"_Classes");
  RtlInitUnicodeString (&ukey, key);
  /* Path to UsrClass.dat changed in Vista to
     \\AppData\\Local\\Microsoft\\Windows\\UsrClass.dat
     but old path is still available via symlinks. */
  wcpcpy (path_comp, L"\\Local Settings\\Application Data\\Microsoft\\"
		      "Windows\\UsrClass.dat");
  RtlInitUnicodeString (&upath, path);
  /* Load UsrClass.dat file into key. */
  status = NtLoadKey (&key_attr, &path_attr);
  if (!NT_SUCCESS (status))
    debug_printf ("Loading user classes hive %S into %S failed: %y",
		  &upath, &ukey, status);
  else
    debug_printf ("Loading user classes hive %S into %S SUCCEEDED: %y",
		  &upath, &ukey, status);
}
