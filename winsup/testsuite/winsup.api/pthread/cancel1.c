/*
 * File: cancel1.c
 *
 * Test Synopsis: Test setting cancel state and cancel type.
 * - 
 *
 * Test Method (Validation or Falsification):
 * - 
 *
 * Requirements Tested:
 * - pthread_setcancelstate function
 * - pthread_setcanceltype function
 *
 * Features Tested:
 * - 
 *
 * Cases Tested:
 * - 
 *
 * Description:
 * - 
 *
 * Environment:
 * - 
 *
 * Input:
 * - None.
 *
 * Output:
 * - File name, Line number, and failed expression on failure.
 * - No output on success.
 *
 * Assumptions:
 * - pthread_create, pthread_self work.
 *
 * Pass Criteria:
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - Process returns non-zero exit status.
 */

#include "test.h"

/*
 * Create NUMTHREADS threads in addition to the Main thread.
 */
enum {
  NUMTHREADS = 10
};

typedef struct bag_t_ bag_t;
struct bag_t_ {
  int threadnum;
  int started;
  /* Add more per-thread state variables here */
};

static bag_t threadbag[NUMTHREADS + 1];

void *
mythread(void * arg)
{
  bag_t * bag = (bag_t *) arg;

  assert(bag == &threadbag[bag->threadnum]);
  assert(bag->started == 0);
  bag->started = 1;

  /* ... */
  {
    int oldstate;
    int oldtype;

    assert(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate) == 0);
    assert(oldstate == PTHREAD_CANCEL_ENABLE); /* Check default */
    assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);
    assert(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) == 0);
    assert(pthread_setcancelstate(oldstate, &oldstate) == 0);
    assert(oldstate == PTHREAD_CANCEL_DISABLE); /* Check setting */

    assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype) == 0);
    assert(oldtype == PTHREAD_CANCEL_DEFERRED); /* Check default */
    assert(pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) == 0);
    assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);
    assert(pthread_setcanceltype(oldtype, &oldtype) == 0);
    assert(oldtype == PTHREAD_CANCEL_ASYNCHRONOUS); /* Check setting */
  }

  return 0;
}

int
main()
{
  int failed = 0;
  int i;
  pthread_t t[NUMTHREADS + 1];

  assert((t[0] = pthread_self()) != NULL);

  for (i = 1; i <= NUMTHREADS; i++)
    {
      threadbag[i].started = 0;
      threadbag[i].threadnum = i;
      assert(pthread_create(&t[i], NULL, mythread, (void *) &threadbag[i]) == 0);
    }

  /*
   * Code to control or munipulate child threads should probably go here.
   */

  /*
   * Give threads time to run.
   */
  Sleep(NUMTHREADS * 1000);

  /*
   * Standard check that all threads started.
   */
  for (i = 1; i <= NUMTHREADS; i++)
    { 
      failed = !threadbag[i].started;

      if (failed)
	{
	  fprintf(stderr, "Thread %d: started %d\n", i, threadbag[i].started);
	}
    }

  assert(!failed);

  /*
   * Check any results here. Set "failed" and only print ouput on failure.
   */
  for (i = 1; i <= NUMTHREADS; i++)
    { 
      /* ... */
    }

  assert(!failed);

  /*
   * Success.
   */
  return 0;
}
