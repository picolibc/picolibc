/*
 * Test for pthread_join().
 *
 * Depends on API functions: pthread_create(), pthread_join(), pthread_exit().
 */

#include "test.h"

void *
func(void * arg)
{
    int i = (int) arg;

    Sleep(i * 500);

    pthread_exit(arg);

    /* Never reached. */
    exit(1);
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

	/* Some threads will finish before they are joined, some after. */
	Sleep(1000);

	for (i = 0; i < 4; i++)
	  {
	    assert(pthread_join(id[i], (void **) &result) == 0);
#if ! defined (__MINGW32__) || defined (__MSVCRT__)
	    assert(result == i);
#else
# warning pthread_join not fully supported in this configuration.
	    assert(result == 0);
#endif
	  }

	/* Success. */
	return 0;
}
