/*
 * A sample program demonstrating how to use fmode to change the default
 * file opening mode to binary. NOTE: Does not change stdin, stdout or
 * stderr.
 *
 * THIS CODE IS IN THE PUBLIC DOMAIN.
 *
 * Colin Peters <colin@fu.is.saga-u.ac.jp>
 */

#include <stdio.h>
#include <fcntl.h>	/* Required to get _fmode and _O_BINARY */

main ()
{
	char* sz = "This is line one.\nThis is line two.\n";
	FILE*	fp;

	_fmode = _O_BINARY;

	printf (sz);

	/* Note how this fopen does NOT indicate "wb" to open the file in
	 * binary mode. */
	fp = fopen ("test.out", "w");

	fprintf (fp, sz);

	fclose (fp);
}

