/*
 * An example showing how you can obtain the UNIX-ish file number from a
 * FILE* and in turn how you can get the Win32 HANDLE of the file from
 * the file number.
 *
 * This code is in the PUBLIC DOMAIN and has NO WARRANTY.
 *
 * Colin Peters <colin@fu.is.saga-u.ac.jp>
 */

#include <stdio.h>
#include <io.h>
#include <windows.h>

int
main (int argc, char* argv[])
{
	char*	szFileName;
	FILE*	fileIn;
	int	fnIn;
	HANDLE	hFileIn;
	char	caBuf[81];
	int	nRead;

	if (argc >= 2)
	{
		szFileName = argv[1];
	}
	else
	{
		szFileName = "junk.txt";
	}

	fileIn = fopen (szFileName, "r");

	if (!fileIn)
	{
		printf ("Could not open %s for reading\n", szFileName);
		exit(1);
	}

	fnIn = fileno (fileIn);
	hFileIn = (HANDLE) _get_osfhandle (fnIn);

	printf ("OS file handle %d\n", (int) hFileIn);

	ReadFile (hFileIn, caBuf, 80, &nRead, NULL);

	printf ("Read %d bytes using ReadFile.\n", nRead);

	caBuf[nRead] = '\0';

	printf ("\"%s\"\n", caBuf);

	fclose (fileIn);
}

