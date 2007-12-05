/* registry.cc: registry interface

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "shared_info.h"
#include "registry.h"
#include "security.h"
#include <cygwin/version.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
static const char cygnus_class[] = "cygnus";

reg_key::reg_key (HKEY top, REGSAM access, ...): _disposition (0)
{
  va_list av;
  va_start (av, access);
  build_reg (top, access, av);
  va_end (av);
}

/* Opens a key under the appropriate Cygwin key.
   Do not use HKCU per MS KB 199190  */

reg_key::reg_key (bool isHKLM, REGSAM access, ...): _disposition (0)
{
  va_list av;
  HKEY top;

  if (isHKLM)
    top = HKEY_LOCAL_MACHINE;
  else
    {
      char name[128];
      const char *names[2] = {cygheap->user.get_windows_id (name), ".DEFAULT"};
      for (int i = 0; i < 2; i++)
	{
	  key_is_invalid = RegOpenKeyEx (HKEY_USERS, names[i], 0, access, &top);
	  if (key_is_invalid == ERROR_SUCCESS)
	    goto OK;
	  debug_printf ("HKU\\%s failed, Win32 error %ld", names[i], key_is_invalid);
	}
      return;
    }
OK:
  new (this) reg_key (top, access, "SOFTWARE",
		      CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
		      CYGWIN_INFO_CYGWIN_REGISTRY_NAME, NULL);
  if (top != HKEY_LOCAL_MACHINE)
    RegCloseKey (top);
  if (key_is_invalid)
    return;

  top = key;
  va_start (av, access);
  build_reg (top, access, av);
  va_end (av);
  if (top != key)
    RegCloseKey (top);
}

void
reg_key::build_reg (HKEY top, REGSAM access, va_list av)
{
  char *name;
  HKEY r = top;
  key_is_invalid = 0;

  /* FIXME: Most of the time a valid mount area should exist.  Perhaps
     we should just try an open of the correct key first and only resort
     to this method in the unlikely situation that it's the first time
     the current mount areas are being used. */

  while ((name = va_arg (av, char *)) != NULL)
    {
      int res = RegCreateKeyExA (r,
				 name,
				 0,
				 (char *) cygnus_class,
				 REG_OPTION_NON_VOLATILE,
				 access,
				 &sec_none_nih,
				 &key,
				 &_disposition);
      if (r != top)
	RegCloseKey (r);
      r = key;
      if (res != ERROR_SUCCESS)
	{
	  key_is_invalid = res;
	  debug_printf ("failed to create key %s in the registry", name);
	  break;
	}
    }
}

/* Given the current registry key, return the specific int value
   requested.  Return def on failure. */

int
reg_key::get_int (const char *name, int def)
{
  DWORD type;
  DWORD dst;
  DWORD size = sizeof (dst);

  if (key_is_invalid)
    return def;

  LONG res = RegQueryValueExA (key, name, 0, &type, (unsigned char *) &dst,
			       &size);

  if (type != REG_DWORD || res != ERROR_SUCCESS)
    return def;

  return dst;
}

/* Given the current registry key, set a specific int value. */

int
reg_key::set_int (const char *name, int val)
{
  DWORD value = val;
  if (key_is_invalid)
    return key_is_invalid;

  return (int) RegSetValueExA (key, name, 0, REG_DWORD,
			       (unsigned char *) &value, sizeof (value));
}

/* Given the current registry key, return the specific string value
   requested.  Return zero on success, non-zero on failure. */

int
reg_key::get_string (const char *name, char *dst, size_t max, const char * def)
{
  DWORD size = max;
  DWORD type;
  LONG res;

  if (key_is_invalid)
    res = key_is_invalid;
  else
    res = RegQueryValueExA (key, name, 0, &type, (unsigned char *) dst, &size);

  if ((def != 0) && ((type != REG_SZ) || (res != ERROR_SUCCESS)))
    strcpy (dst, def);
  return (int) res;
}

/* Given the current registry key, set a specific string value. */

int
reg_key::set_string (const char *name, const char *src)
{
  if (key_is_invalid)
    return key_is_invalid;
  return (int) RegSetValueExA (key, name, 0, REG_SZ, (unsigned char*) src,
			       strlen (src) + 1);
}

/* Return the handle to key. */

HKEY
reg_key::get_key ()
{
  return key;
}

/* Delete subkey of current key.  Returns the error code from the
   RegDeleteKeyA invocation. */

int
reg_key::kill (const char *name)
{
  if (key_is_invalid)
    return key_is_invalid;
  return RegDeleteKeyA (key, name);
}

/* Delete the value specified by name of current key.  Returns the error code
   from the RegDeleteValueA invocation. */

int
reg_key::killvalue (const char *name)
{
  if (key_is_invalid)
    return key_is_invalid;
  return RegDeleteValueA (key, name);
}

reg_key::~reg_key ()
{
  if (!key_is_invalid)
    RegCloseKey (key);
  key_is_invalid = 1;
}

char *
get_registry_hive_path (const char *name, char *path)
{
  char key[256];
  HKEY hkey;

  if (!name || !path)
    return NULL;
  __small_sprintf (key, "SOFTWARE\\Microsoft\\WindowsNT\\CurrentVersion\\"
  			"ProfileList\\%s", name);
  if (!RegOpenKeyExA (HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hkey))
    {
      char buf[PATH_MAX];
      DWORD type, siz;

      path[0] = '\0';
      if (!RegQueryValueExA (hkey, "ProfileImagePath", 0, &type,
			     (BYTE *)buf, (siz = sizeof (buf), &siz)))
	ExpandEnvironmentStringsA (buf, path, PATH_MAX);
      RegCloseKey (hkey);
      if (path[0])
	return path;
    }
  debug_printf ("HKLM\\%s not found", key);
  return NULL;
}

void
load_registry_hive (const char * name)
{
  char path[PATH_MAX];
  HKEY hkey;
  LONG ret;

  if (!name)
    return;
  /* Check if user hive is already loaded. */
  if (!RegOpenKeyExA (HKEY_USERS, name, 0, KEY_READ, &hkey))
    {
      debug_printf ("User registry hive for %s already exists", name);
      RegCloseKey (hkey);
      return;
    }
  if (get_registry_hive_path (name, path))
    {
      strcat (path, "\\NTUSER.DAT");
      if ((ret = RegLoadKeyA (HKEY_USERS, name, path)) != ERROR_SUCCESS)
	debug_printf ("Loading user registry hive for %s failed: %d", name, ret);
    }
}

