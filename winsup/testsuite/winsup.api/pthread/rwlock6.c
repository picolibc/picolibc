/*
 * rwlock6.c
 *
 * Check writer and reader locking
 *
 * Depends on API functions:
 *      pthread_rwlock_rdlock()
 *      pthread_rwlock_wrlock()
 *      pthread_rwlock_unlock()
 */

#include "test.h"

static pthread_rwlock_t rwlock1 = PTHREAD_RWLOCK_INITIALIZER;

static int bankAccount = 0;

void * wrfunc(void * arg)
{
  int ba;

  assert(pthread_rwlock_wrlock(&rwlock1) == 0);
  Sleep(2000);
  bankAccount += 10;
  ba = bankAccount;
  assert(pthread_rwlock_unlock(&rwlock1) == 0);

  return ((void *) ba);
}

void * rdfunc(void * arg)
{
  int ba;

  assert(pthread_rwlock_rdlock(&rwlock1) == 0);
  ba = bankAccount;
  assert(pthread_rwlock_unlock(&rwlock1) == 0);

  return ((void *) ba);
}

int
main()
{
  pthread_t wrt1;
  pthread_t wrt2;
  pthread_t rdt;
  int wr1Result = 0;
  int wr2Result = 0;
  int rdResult = 0;

  bankAccount = 0;

  assert(pthread_create(&wrt1, NULL, wrfunc, NULL) == 0);
  Sleep(500);
  assert(pthread_create(&rdt, NULL, rdfunc, NULL) == 0);
  Sleep(500);
  assert(pthread_create(&wrt2, NULL, wrfunc, NULL) == 0);

  assert(pthread_join(wrt1, (void **) &wr1Result) == 0);
  assert(pthread_join(rdt, (void **) &rdResult) == 0);
  assert(pthread_join(wrt2, (void **) &wr2Result) == 0);

  assert(wr1Result == 10);
  assert(rdResult == 20);
  assert(wr2Result == 20);

  return 0;
}
