/*
 * once1.c
 *
 * Create a static pthread_once and test that it calls myfunc once.
 *
 * Depends on API functions:
 *	pthread_once()
 *	pthread_create()
 */

#include "test.h"

pthread_once_t once = PTHREAD_ONCE_INIT;

static int washere = 0;

void
myfunc(void)
{
  washere++;
}

void *
mythread(void * arg)
{
   assert(pthread_once(&once, myfunc) == 0);

   return 0;
}

int
main()
{
  pthread_t t1, t2;
  
  assert(pthread_create(&t1, NULL, mythread, NULL) == 0);

  assert(pthread_create(&t2, NULL, mythread, NULL) == 0);

  Sleep(2000);

  assert(washere == 1);

  return 0;
}
