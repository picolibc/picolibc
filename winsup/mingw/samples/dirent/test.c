/*
 * A test which demonstrates the use of opendir and related functions
 * declared in dirent.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <dirent.h>

int
main (int argc, char* argv[])
{
	int 		i;
	struct dirent*	de;
	DIR*		dir;
	long		lPos;

	if (argc == 2)
	{
		printf ("Opening directory \"%s\"\n", argv[1]);
		dir = opendir(argv[1]);
	}
	else
	{
		printf ("Opening \".\"\n");
		dir = opendir(".");
	}

	if (!dir)
	{
		printf ("Directory open failed!\n");
		if (errno)
		{
			printf ("Error : %s\n", strerror(errno));
		}
		return 1;
	}

	i = 0;
	lPos = -1;

	while (de = readdir (dir))
	{
		i++;
		printf ("%d : \"%s\" (tell %ld)\n", i, de->d_name,
			telldir(dir));

		if (i == 3)
		{
			printf ("We will seek here later.\n");
			lPos = telldir (dir);
		}
	}

	printf ("Rewind directory.\n");
	rewinddir (dir);

	if (de = readdir (dir))
	{
		printf ("First entry : \"%s\"\n", de->d_name);
	}
	else
	{
		printf ("Empty directory.\n");
	}

	if (lPos != -1)
	{
		printf ("Seeking to fourth entry.\n");
		seekdir (dir, lPos);

		if (de = readdir (dir))
		{
			printf ("Fourth entry : \"%s\"\n", de->d_name);
		}
		else
		{
			printf ("No fourth entry.\n");
		}
	}
	else
	{
		printf ("Seek position is past end of directory.\n");
	}

	printf ("Closing directory.\n");
	closedir (dir);
}

