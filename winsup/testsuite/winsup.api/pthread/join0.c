/*
 * Test for pthread_join().
 *
 * Depends on API functions: pthread_create(), pthread_exit().
 */

#include "test.h"

void *
func(void * arg)
{
  Sleep(2000);

  pthread_exit(arg);

  /* Never reached. */
  exit(1);
}

int
main(int argc, char * argv[])
{
  pthread_t id;
  int result;

  /* Create a single thread and wait for it to exit. */
  assert(pthread_create(&id, NULL, func, (void *) 123) == 0);

  assert(pthread_join(id, (void **) &result) == 0);

#if ! defined (__MINGW32__) || defined (__MSVCRT__)
  assert(result == 123);
#else
# warning pthread_join not fully supported in this configuration.
  assert(result == 0);
#endif

  /* Success. */
  return 0;
}
