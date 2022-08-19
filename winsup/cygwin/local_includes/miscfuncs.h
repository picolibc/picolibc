/* miscfuncs.h: Internal functions not having a better home

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _MISCFUNCS_H
#define _MISCFUNCS_H

#include <dinput.h>

#define likely(X) __builtin_expect (!!(X), 1)
#define unlikely(X) __builtin_expect (!!(X), 0)

/* Check for Alt+Numpad keys in a console input record.  These are used to
   enter codepoints not available in the current keyboard layout  For details
   see http://www.fileformat.info/tip/microsoft/enter_unicode.htm */
static inline bool
is_alt_numpad_key (PINPUT_RECORD pirec)
{
  /* Remove lock key state from ControlKeyState.  Do not remove enhanced key
     state since it helps to distinguish between cursor (EK) and numpad keys
     (non-EK). */
  DWORD ctrl_state = pirec->Event.KeyEvent.dwControlKeyState
		     & ~(CAPSLOCK_ON | NUMLOCK_ON | SCROLLLOCK_ON);

  return pirec->Event.KeyEvent.uChar.UnicodeChar == 0
	 && ctrl_state == LEFT_ALT_PRESSED
	 && pirec->Event.KeyEvent.wVirtualScanCode >= DIK_NUMPAD7
	 && pirec->Event.KeyEvent.wVirtualScanCode <= DIK_NUMPAD0
	 && pirec->Event.KeyEvent.wVirtualScanCode != DIK_SUBTRACT;
}

/* Event for left Alt, with a non-zero character, comes from Alt+Numpad
   key sequence. e.g. <left-alt> 233 => &eacute;  This is typically handled
   as the key up event after releasing the Alt key. */
static inline bool
is_alt_numpad_event (PINPUT_RECORD pirec)
{
  return pirec->Event.KeyEvent.uChar.UnicodeChar != 0
	 && pirec->Event.KeyEvent.wVirtualKeyCode == VK_MENU
	 && pirec->Event.KeyEvent.wVirtualScanCode == 0x38;
}

int winprio_to_nice (DWORD);
DWORD nice_to_winprio (int &);

bool create_pipe (PHANDLE, PHANDLE, LPSECURITY_ATTRIBUTES, DWORD);

BOOL CreatePipeOverlapped (PHANDLE read_handle, PHANDLE write_handle,
			   LPSECURITY_ATTRIBUTES sa);
BOOL ReadPipeOverlapped (HANDLE h, PVOID buf, DWORD len,
			 LPDWORD ret_len, DWORD timeout);
BOOL WritePipeOverlapped (HANDLE h, LPCVOID buf, DWORD len,
			  LPDWORD ret_len, DWORD timeout);

/* class for per-line reading using native functions.  The caller provides
   the file as an POBJECT_ATTRIBUTES, and the buffer space. */
class NT_readline
{
  HANDLE fh;
  PCHAR buf;
  PCHAR got;
  PCHAR end;
  ULONG buflen;
  ULONG len;
  ULONG line;
public:
  NT_readline () : fh (NULL) {}
  bool init (POBJECT_ATTRIBUTES attr, char *buf, ULONG buflen);
  PCHAR gets ();
  void close () { if (fh) NtClose (fh); fh = NULL; }
  ~NT_readline () { close (); }
};

extern "C" void yield ();

void backslashify (const char *, char *, bool);
void slashify (const char *, char *, bool);
#define isslash(c) ((c) == '/')

extern void transform_chars (PWCHAR, PWCHAR);
extern inline void
transform_chars (PUNICODE_STRING upath, USHORT start_idx)
{
  transform_chars (upath->Buffer + start_idx,
		   upath->Buffer + upath->Length / sizeof (WCHAR) - 1);
}

PWCHAR transform_chars_af_unix (PWCHAR, const char *, __socklen_t);

/* Get handle count of an object. */
ULONG get_obj_handle_count (HANDLE h);

ssize_t check_iovec (const struct iovec *, int, bool);
#define check_iovec_for_read(a, b) check_iovec ((a), (b), false)
#define check_iovec_for_write(a, b) check_iovec ((a), (b), true)

void SetThreadName (DWORD dwThreadID, const char* threadName);

WORD __get_cpus_per_group (void);
WORD __get_group_count (void);

#endif /*_MISCFUNCS_H*/
