#include <windows.h>
int
main (int argc, char **argv)
{
  char *end;
  if (argc != 3)
    exit (1);
  HANDLE h = (HANDLE) strtoul (argv[1], &end, 0);
  SetEvent (h);
  h = (HANDLE) strtoul (argv[2], &end, 0);
  WaitForSingleObject (h, INFINITE);
  exit (0);
}
