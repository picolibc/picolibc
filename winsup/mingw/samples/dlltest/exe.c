#include <stdio.h>

#include "dll.h"

int main()
{
	int i, j, k;
	double	dk;

	i = 10;
	j = 13;

	k = Add(i, j);

	printf ("%d + %d = %d\n", i, j, k);
    
	dk = Sub(i, j);

	printf ("%d - %d = %f\n", i, j, dk);

	return 0;
}

