/* registry.cc: registry interface

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"

char cygnus_class[] = "cygnus";

reg_key::reg_key (HKEY top, REGSAM access, ...)
{
  va_list av;
  va_start (av, access);
  build_reg (top, access, av);
  va_end (av);
}

reg_key::reg_key (REGSAM access, ...)
{
  va_list av;

  new (this) reg_key (HKEY_CURRENT_USER, access, "SOFTWARE",
		 CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
		 CYGWIN_INFO_CYGWIN_REGISTRY_NAME, NULL);

  HKEY top = key;
  va_start (av, access);
  build_reg (top, KEY_READ, av);
  va_end (av);
  if (top != key)
    RegCloseKey (top);
}

reg_key::reg_key (REGSAM access)
{
  new (this) reg_key (HKEY_CURRENT_USER, access, "SOFTWARE",
		 CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
		 CYGWIN_INFO_CYGWIN_REGISTRY_NAME,
		 CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME, NULL);
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
      DWORD disp;
      int res = RegCreateKeyExA (r,
				 name,
				 0,
				 cygnus_class,
				 REG_OPTION_NON_VOLATILE,
				 access,
				 &sec_none_nih,
				 &key,
				 &disp);
      if (r != top)
	RegCloseKey (r);
      r = key;
      if (res != ERROR_SUCCESS)
	{
	  key_is_invalid = res;
	  debug_printf ("failed to create key %s in the registry", name);
	  break;
	}

      /* If we're considering the mounts key, check if it had to
	 be created and set had_to_create appropriately. */
      if (strcmp (name, CYGWIN_INFO_CYGWIN_MOUNT_REGISTRY_NAME) == 0)
	if (disp == REG_CREATED_NEW_KEY)
	  cygwin_shared->mount.had_to_create_mount_areas++;
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

reg_key::~reg_key ()
{
  if (!key_is_invalid)
    RegCloseKey (key);
  key_is_invalid = 1;
}
