/*
 * Source code of the functions inside our test DLL. Note that DllMain is
 * not required (it will be provided by the stub in libmingw32.a).
 */

#if 0
#include <windows.h>
#endif

int Add (int x, int y)
{
	printf ("In add!\nx = %d\ny = %d\n", x, y);
	return (x + y);
}


double __attribute__((stdcall)) Sub (double x, double y) 
{
	printf ("In sub!\nx = %f\ny = %f\n", x, y);
	return (x - y);
}

