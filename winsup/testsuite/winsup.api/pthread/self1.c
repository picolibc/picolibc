/*
 * self1.c
 *
 * Test for pthread_self().
 *
 * Depends on API functions:
 *	pthread_self()
 *
 * Implicitly depends on:
 *	pthread_getspecific()
 *	pthread_setspecific()
 */

#include "test.h"

int
main(int argc, char * argv[])
{
	/*
	 * This should always succeed unless the system has no
	 * resources (memory) left.
	 */
	assert(pthread_self() != NULL);

	return 0;
}
