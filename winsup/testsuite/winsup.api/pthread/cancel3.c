/*
 * File: cancel3.c
 *
 * Test Synopsis: Test asynchronous cancelation.
 *
 * Test Method (Validation or Falsification):
 * - 
 *
 * Requirements Tested:
 * -
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
 * - have working pthread_create, pthread_self, pthread_mutex_lock/unlock
 *   pthread_testcancel, pthread_cancel, pthread_join
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
  int count;
};

static bag_t threadbag[NUMTHREADS + 1];

void *
mythread(void * arg)
{
  int result = ((int)PTHREAD_CANCELED + 1);
  bag_t * bag = (bag_t *) arg;

  assert(bag == &threadbag[bag->threadnum]);
  assert(bag->started == 0);
  bag->started = 1;

  /* Set to known state and type */

  assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);

  assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);

  /*
   * We wait up to 10 seconds, waking every 0.1 seconds,
   * for a cancelation to be applied to us.
   */
  for (bag->count = 0; bag->count < 100; bag->count++)
    Sleep(100);

  return (void *) result;
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
  Sleep(500);

  for (i = 1; i <= NUMTHREADS; i++)
    {
      assert(pthread_cancel(t[i]) == 0);
    }

  /*
   * Give threads time to run.
   */
  Sleep(NUMTHREADS * 100);

  /*
   * Standard check that all threads started.
   */
  for (i = 1; i <= NUMTHREADS; i++)
    { 
      if (!threadbag[i].started)
	{
	  failed |= !threadbag[i].started;
	  fprintf(stderr, "Thread %d: started %d\n", i, threadbag[i].started);
	}
    }

  assert(!failed);

  /*
   * Check any results here. Set "failed" and only print output on failure.
   */
  failed = 0;
  for (i = 1; i <= NUMTHREADS; i++)
    {
      int fail = 0;
      int result = 0;

      /*
       * The thread does not contain any cancelation points, so
       * a return value of PTHREAD_CANCELED confirms that async
       * cancelation succeeded.
       */
      assert(pthread_join(t[i], (void **) &result) == 0);

      fail = (result != (int) PTHREAD_CANCELED);

      if (fail)
	{
	  fprintf(stderr, "Thread %d: started %d: count %d\n",
		  i,
		  threadbag[i].started,
		  threadbag[i].count);
	}
      failed = (failed || fail);
    }

  assert(!failed);

  /*
   * Success.
   */
  return 0;
}
