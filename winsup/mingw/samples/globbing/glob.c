
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

int
main (int argc, char* argv[])
{
	int	i;

	printf ("Command line (via GetCommandLine) \"%s\"\n",
		GetCommandLine());

	for (i = 0; i < argc; i++)
	{
		printf ("Argv[%d] \"%s\"\n", i, argv[i]);
	}

	return 0;
}
