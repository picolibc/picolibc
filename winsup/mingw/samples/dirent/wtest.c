/*
 * A test which demonstrates the use of _wopendir and related
 * wide	char functions declared in dirent.h. 
 *
 * TODO: Make this _UNICODE neutral using tchar.h mappings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

int
main (int argc, char* argv[])
{
	int 		i;
	struct _wdirent*	de;
	_WDIR*		dir;
	long		lPos;

	if (argc == 2)
	{
		size_t len = strlen(argv[1]) + 1;
	        wchar_t* wpath = (wchar_t*) malloc(len *sizeof(wchar_t));
		mbstowcs(wpath, argv[1], len); 
		wprintf (L"Opening directory \"%s\"\n", wpath);
		dir = _wopendir(wpath);
		free (wpath);
	}
	else
	{
		wprintf (L"Opening \".\"\n");
		dir = _wopendir(L".");
	}

	if (!dir)
	{
		wprintf (L"Directory open failed!\n");
		if (errno)
		{
			wprintf (L"Error : %S\n", strerror(errno));
		}
		return 1;
	}

	i = 0;
	lPos = -1;

	while ((de = _wreaddir (dir)))
	{
		i++;
		wprintf (L"%d : \"%s\" (tell %ld)\n", i, de->d_name,
			_wtelldir(dir));

		if (i == 3)
		{
			wprintf (L"We will seek here later.\n");
			lPos = _wtelldir (dir);
		}
	}

	printf ("Rewind directory.\n");
	_wrewinddir (dir);

	if ((de = _wreaddir (dir)))
	{
		wprintf (L"First entry : \"%s\"\n", de->d_name);
	}
	else
	{
		wprintf (L"Empty directory.\n");
	}

	if (lPos != -1)
	{
		wprintf (L"Seeking to fourth entry.\n");
		_wseekdir (dir, lPos);

		if ((de = _wreaddir (dir)))
		{
			wprintf (L"Fourth entry : \"%s\"\n", de->d_name);
		}
		else
		{
			wprintf (L"No fourth entry.\n");
		}
	}
	else
	{
		wprintf (L"Seek position is past end of directory.\n");
	}

	wprintf (L"Closing directory.\n");
	_wclosedir (dir);
return 0;
}

