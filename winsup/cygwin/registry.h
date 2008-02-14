/* registry.h: shared info for cygwin

   Copyright 2000, 2001, 2004, 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

class reg_key
{
private:

  HKEY key;
  LONG key_is_invalid;
  DWORD _disposition;

public:

  reg_key (HKEY toplev, REGSAM access, ...);
  reg_key (bool isHKLM, REGSAM access, ...);

  void *operator new (size_t, void *p) {return p;}
  void build_reg (HKEY key, REGSAM access, va_list av);

  int error () {return key == (HKEY) INVALID_HANDLE_VALUE;}

  int kill (const char *child);
  int killvalue (const char *name);

  HKEY get_key ();
  int get_int (const char *,int def);
  int get_string (const char *, char *buf, size_t len, const char *def);
  int set_string (const char *,const char *);
  int set_int (const char *, int val);
  bool created () const {return _disposition & REG_CREATED_NEW_KEY;}

  ~reg_key ();
};

/* Evaluates path to the directory of the local user registry hive */
PWCHAR __stdcall get_registry_hive_path (const PWCHAR name, PWCHAR path);
void __stdcall load_registry_hive (const PWCHAR name);
