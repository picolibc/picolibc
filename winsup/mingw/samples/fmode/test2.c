/*
 * A sample program demonstrating how to use fmode to change the default
 * file opening mode to binary.  Compare this file, which sets _fmode
 * at file level to test.c, which sets the dll variable directly within
 * main.
 * NOTE: Does not change stdin, stdout or stderr.
 *
 * THIS CODE IS IN THE PUBLIC DOMAIN.
 *
 * Colin Peters <colin@fu.is.saga-u.ac.jp>
 * Danny Smith <dannysmith@users.sourceforge.net>
 */

#include <stdio.h>
#include <stdlib.h>	/*  _fmode */
#include <fcntl.h>	/*  _O_BINARY */

#undef _fmode
int _fmode = _O_BINARY;

main ()
{
    
	char* sz = "This is line one.\nThis is line two.\n";
	FILE*	fp;

	printf (sz);

	/* Note how this fopen does NOT indicate "wb" to open the file in
	 * binary mode. */
	fp = fopen ("test2.out", "w");

	fprintf (fp, sz);

	fclose (fp);
}

