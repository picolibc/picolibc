/*
 * A sample program demonstrating how to use _CRT_fmode to change the default
 * file opening mode to binary AND change stdin, stdout and stderr. Redirect
 * stdout to a file from the command line to see the difference.
 *
 * Also try directing a file into stdin. If you type into stdin you will get
 * \r\n at the end of every line... unlike UNIX. But at least if you
 * redirect a file in you will get exactly the characters in the file as input.
 *
 * THIS CODE IS IN THE PUBLIC DOMAIN.
 *
 * Colin Peters <colin@fu.is.saga-u.ac.jp>
 */

#include <stdio.h>
#include <fcntl.h>

unsigned int _CRT_fmode = _O_BINARY;

main ()
{
	char* sz = "This is line one.\nThis is line two.\n";
	FILE*	fp;
	int	c;

	printf (sz);

	/* Note how this fopen does NOT indicate "wb" to open the file in
	 * binary mode. */
	fp = fopen ("all.out", "w");

	fprintf (fp, sz);

	fclose (fp);

	if (_isatty (_fileno(stdin)))
	{
		fprintf (stderr, "Waiting for input, press Ctrl-Z to finish.\n");
	}

	while ((c = fgetc(stdin)) != EOF)
	{
		printf ("\'%c\' %02X\n", (char) c, c);
	}
}

