/*
 * create1.c
 *
 * Description:
 * Create a thread and check that it ran.
 *
 * Depends on API functions: None.
 */

#include "test.h"

static int washere = 0;

void * func(void * arg)
{
  washere = 1;
  return 0; 
}
 
int
main()
{
  pthread_t t;

  assert(pthread_create(&t, NULL, func, NULL) == 0);

  /* A dirty hack, but we cannot rely on pthread_join in this
     primitive test. */
  Sleep(2000);

  assert(washere == 1);

  return 0;
}
