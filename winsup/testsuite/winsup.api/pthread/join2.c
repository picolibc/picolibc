/*
 * Test for pthread_join() returning return value from threads.
 *
 * Depends on API functions: pthread_create().
 */

#include "test.h"

void *
func(void * arg)
{
	Sleep(1000);
	return arg;
}

int
main(int argc, char * argv[])
{
	pthread_t id[4];
	int i;
	int result;

	/* Create a few threads and then exit. */
	for (i = 0; i < 4; i++)
	  {
	    assert(pthread_create(&id[i], NULL, func, (void *) i) == 0);
	  }

	for (i = 0; i < 4; i++)
	  {
	    assert(pthread_join(id[i], (void **) &result) == 0);
#if ! defined (__MINGW32__) || defined (__MSVCRT__)
	    /* CRTDLL _beginthread doesn't support return value, so
	       the assertion is guaranteed to fail. */
	    assert(result == i);
#endif
	  }

	/* Success. */
	return 0;
}
