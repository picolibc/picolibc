/*
 * Test for pthread_exit().
 *
 * Depends on API functions: None.
 */

#include "test.h"

int
main(int argc, char * argv[])
{
	/* A simple test first. */
	pthread_exit((void *) 0);

	/* Not reached */
	assert(0);
	return 0;
}
