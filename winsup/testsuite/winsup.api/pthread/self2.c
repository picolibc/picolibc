/*
 * self2.c
 *
 * Test for pthread_self().
 *
 * Depends on API functions:
 *	pthread_create()
 *	pthread_self()
 *
 * Implicitly depends on:
 *	pthread_getspecific()
 *	pthread_setspecific()
 */

#include "test.h"
#include <string.h>

static pthread_t me;

void *
entry(void * arg)
{
  me = pthread_self();

  return arg;
}

int
main()
{
  pthread_t t;

  assert(pthread_create(&t, NULL, entry, NULL) == 0);

  Sleep(2000);

  /*
   * Not much more we can do here but bytewise compare t with
   * what pthread_self returned.
   */
  assert(t == me);
  assert(memcmp((const void *) t, (const void *) me, sizeof t) == 0);

  /* Success. */
  return 0;
}
