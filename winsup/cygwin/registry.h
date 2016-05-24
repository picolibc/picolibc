/* registry.h: shared info for cygwin

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
