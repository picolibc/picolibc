/*
 * This version attempts to load dll.dll dynamically, get the address of the
 * Add function, and then call it.
 */

#include <stdio.h>
#include <windows.h>

int (*Add)(int x, int y);

int main()
{
	HINSTANCE	hDll;
	int i, j, k;

	hDll = LoadLibrary ("dll.dll");
	if (!hDll)
	{
		printf ("Error %d loading dll.\n", GetLastError());
		exit (-1);
	}

	if (!(Add = GetProcAddress (hDll, "Add")))
	{
		printf ("Error %d getting Add function.\n", GetLastError());
		exit (-1);
	}

	i = 10;
	j = 13;

	k = Add(i, j);

	printf ("i %d, j %d, k %d\n", i, j, k);

	FreeLibrary (hDll);
    
	return 0;
}

