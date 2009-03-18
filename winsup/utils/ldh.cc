#include <windows.h>
int APIENTRY
WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  LPWSTR cmd = GetCommandLineW ();
  while (*cmd)
    if (*cmd != ' ' || cmd[1] != L'-' || cmd[2] != '-' || cmd[3] != ' ')
      cmd++;
    else
      {
	cmd += 4;
	break;
      }
  if (!*cmd || !LoadLibraryW (cmd))
    ExitProcess (0x0100);
  ExitProcess (0x0000);
}
