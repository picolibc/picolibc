/* registry.h: shared info for cygwin

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
   2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

class reg_key
{
private:

  HANDLE key;
  NTSTATUS key_is_invalid;
  DWORD _disposition;

public:

  reg_key (HKEY toplev, REGSAM access, ...);
  reg_key (bool isHKLM, REGSAM access, ...);

  void *operator new (size_t, void *p) {return p;}
  void build_reg (HKEY key, REGSAM access, va_list av);

  bool error () {return key == NULL;}

  DWORD get_dword (PCWSTR, DWORD);
  NTSTATUS get_string (PCWSTR, PWCHAR, size_t, PCWSTR);

  NTSTATUS set_dword (PCWSTR, DWORD);
  NTSTATUS set_string (PCWSTR, PCWSTR);

  bool created () const {return _disposition & REG_CREATED_NEW_KEY;}

  ~reg_key ();
};

/* Evaluates path to the directory of the local user registry hive */
PWCHAR __stdcall get_registry_hive_path (PCWSTR name, PWCHAR path);
void __stdcall load_registry_hive (PCWSTR name);
