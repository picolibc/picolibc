/* cygkeycheck.cc

   Copyright 2000 Cygnus Solutions.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <stdio.h>
#include <windows.h>

char *myname;

int
eprint (const char *name)
{
  fprintf (stderr, "%s: %s failed: %lu\n", myname, name, GetLastError ());
  return 1;
}

int
main (int argc, char **argv)
{
  myname = argv[0];

  HANDLE h = CreateFileA ("CONIN$", GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (h == INVALID_HANDLE_VALUE || h == NULL)
    return eprint ("Opening CONIN$");

  DWORD mode;

  if (!GetConsoleMode (h, &mode))
    eprint ("GetConsoleMode");
  else
    {
      mode &= ~ENABLE_PROCESSED_INPUT;
      if (!SetConsoleMode (h, mode))
        eprint ("GetConsoleMode");
    }

  fputs ("\nThis key checker works only in a console window,", stderr);
  fputs (" _NOT_ in a terminal session!\n", stderr);
  fputs ("Abort with Ctrl+C if in a terminal session.\n\n", stderr);
  fputs ("Press `q' to exit.\n", stderr);

  INPUT_RECORD in, prev_in;

  // Drop first <RETURN> key
  ReadConsoleInput (h, &in, 1, &mode);

  memset (&in, 0, sizeof in);

  do
    {
      prev_in = in;
      if (!ReadConsoleInput (h, &in, 1, &mode))
        eprint ("ReadConsoleInput");

      if (!memcmp (&in, &prev_in, sizeof in))
        continue;

      switch (in.EventType)
        {
        case KEY_EVENT:
          printf ("%s %ux VK: 0x%02x VS: 0x%02x A: 0x%02x CTRL: ",
                  in.Event.KeyEvent.bKeyDown ? "Pressed " : "Released",
                  in.Event.KeyEvent.wRepeatCount,
                  in.Event.KeyEvent.wVirtualKeyCode,
                  in.Event.KeyEvent.wVirtualScanCode,
                  (unsigned char) in.Event.KeyEvent.uChar.AsciiChar);
          fputs (in.Event.KeyEvent.dwControlKeyState & CAPSLOCK_ON ?
                 "CL " : "-- ", stdout);
          fputs (in.Event.KeyEvent.dwControlKeyState & ENHANCED_KEY ?
                 "EK " : "-- ", stdout);
          fputs (in.Event.KeyEvent.dwControlKeyState & LEFT_ALT_PRESSED ?
                 "LA " : "-- ", stdout);
          fputs (in.Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED ?
                 "LC " : "-- ", stdout);
          fputs (in.Event.KeyEvent.dwControlKeyState & NUMLOCK_ON ?
                 "NL " : "-- ", stdout);
          fputs (in.Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED ?
                 "RA " : "-- ", stdout);
          fputs (in.Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED ?
                 "RC " : "-- ", stdout);
          fputs (in.Event.KeyEvent.dwControlKeyState & SCROLLLOCK_ON ?
                 "SL " : "-- ", stdout);
          fputs (in.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED ?
                 "SH " : "-- ", stdout);
          fputc ('\n', stdout);
          break;

        }
    }
  while (in.EventType != KEY_EVENT ||
         in.Event.KeyEvent.bKeyDown != FALSE ||
         in.Event.KeyEvent.uChar.AsciiChar != 'q');

  CloseHandle (h);
  return 0;
}
