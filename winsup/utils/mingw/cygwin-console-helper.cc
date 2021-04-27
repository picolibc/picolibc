#include <windows.h>
#include <stdio.h>
int
main (int argc, char **argv)
{
  char *end;
  if (argc < 3)
    exit (1);
  HANDLE h = (HANDLE) strtoull (argv[1], &end, 0);
  SetEvent (h);
  if (argc == 4) /* Pseudo console helper mode for PTY */
    {
      SetConsoleCtrlHandler (NULL, TRUE);
      HANDLE hPipe = (HANDLE) strtoull (argv[3], &end, 0);
      char buf[64];
      sprintf (buf, "StdHandles=%p,%p\n",
	       GetStdHandle (STD_INPUT_HANDLE),
	       GetStdHandle (STD_OUTPUT_HANDLE));
      DWORD dwLen;
      WriteFile (hPipe, buf, strlen (buf), &dwLen, NULL);
      CloseHandle (hPipe);
    }
  h = (HANDLE) strtoull (argv[2], &end, 0);
  WaitForSingleObject (h, INFINITE);
  exit (0);
}
