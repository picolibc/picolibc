/*
 * Test for pthread_exit().
 *
 * Depends on API functions:
 *	pthread_create()
 *	pthread_exit()
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
  pthread_t t;

  assert(pthread_create(&t, NULL, func, (void *) NULL) == 0);

  Sleep(1000);

  return 0;
}
