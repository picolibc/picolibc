
#include <windows.h>

BOOL WINAPI
DllMain (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			printf ("DLL Attached.\n");
			break;

		case DLL_PROCESS_DETACH:
			printf ("DLL Detached.\n");
			break;

		case DLL_THREAD_ATTACH:
			printf ("DLL Thread Attached.\n");
			break;

		case DLL_THREAD_DETACH:
			printf ("DLL Thread Detached.\n");
			break;
	}
	return TRUE;
}

void
Test ()
{
	printf ("Test Function called!\n");
}

