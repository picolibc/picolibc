/*
 * This source code is in the PUBLIC DOMAIN and has NO WARRANTY.
 *
 * Colin Peters <colin@bird.fu.is.saga-u.ac.jp>, April 15, 1997.
 */

#include <windows.h>
#include <stdio.h>

int STDCALL
WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
	char text[80];

	printf ("Enter message box text:");
	fgets(text, 80, stdin);
	MessageBox (NULL, text, "Test", MB_OK);
	printf ("\nHello after message box.\n");
	return 0;
}
