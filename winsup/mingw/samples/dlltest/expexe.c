
#include <stdio.h>

int
ExportedFromExe ()
{
	printf ("This output produced by ExportedFromExe.\n");
	return 0;
}

int main()
{
	printf ("Hello, world\n");

	return 0;
}

