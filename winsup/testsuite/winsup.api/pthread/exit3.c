/*
 * Test for pthread_exit().
 *
 * Depends on API functions: pthread_create().
 */

#include "test.h"

void *
func(void * arg)
{
	pthread_exit(arg);

	/* Never reached. */
	assert(0);
}

int
main(int argc, char * argv[])
{
	pthread_t id[4];
	int i;

	/* Create a few threads and then exit. */
	for (i = 0; i < 4; i++)
	  {
	    assert(pthread_create(&id[i], NULL, func, (void *) i) == 0);
	  }

	Sleep(1000);

	/* Success. */
	return 0;
}
