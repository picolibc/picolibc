
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>


#if 0
#define	STREAMS_VERSION
#endif

#if	defined(STREAMS_VERSION)
#include <iostream.h>
#endif

#include "silly.h"

extern "C"
BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    return TRUE;
}

CSilly::
CSilly()
{
	szName = NULL;
}

CSilly::
CSilly(char* new_szName)
{
	szName = new char[strlen(new_szName)+1];

	if (szName)
	{
		strcpy (szName, new_szName);
	}
}

CSilly::
~CSilly()
{
	printf ("In CSilly destructor.\n");
	if (szName)
	{
		delete szName;
	}
}

CSilly::
Poke ()
{
#ifndef	STREAMS_VERSION
	printf ("Ouch!\n");
#else
	cout << "Ouch!" << endl;
#endif
}

CSilly::
Stab (int nTimes)
{
#ifndef	STREAMS_VERSION
	printf ("Ugh");
#else
	cout << "Ugh";
#endif

	int i;
	for (i = 0; i < nTimes; i++)
	{
#ifndef	STREAMS_VERSION
		putchar('!');
#else
		cout << '!' ;
#endif
	}

#ifndef	STREAMS_VERSION
	putchar('\n');
#else
	cout << endl;
#endif
}

CSilly::
WhatsYourName ()
{
	if (szName)
	{
#ifndef	STREAMS_VERSION
		printf ("I'm %s.\n", szName);
#else
		cout << "I'm " << szName << "." << endl;
#endif
	}
	else
	{
#ifndef	STREAMS_VERSION
		printf ("I have no name.\n");
#else
		cout << "I have no name." << endl;
#endif
	}
}

