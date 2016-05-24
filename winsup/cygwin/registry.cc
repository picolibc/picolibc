/* registry.cc: registry interface

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
