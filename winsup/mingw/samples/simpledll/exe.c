#include <stdio.h>

extern void Test();

int main()
{
	printf ("Program started.\n");
	Test ();
	printf ("Program ends.\n");

	return 0;
}

