/*
 * tsd1.c
 *
 * Test Thread Specific Data (TSD) key creation and destruction.
 *
 * Description:
 * - 
 *
 * Test Method (validation or falsification):
 * - validation
 *
 * Requirements Tested:
 * - keys are created for each existing thread including the main thread
 * - keys are created for newly created threads
 * - keys are thread specific
 * - destroy routine is called on each thread exit including the main thread
 *
 * Features Tested:
 * - 
 *
 * Cases Tested:
 * - 
 *
 * Environment:
 * - 
 *
 * Input:
 * - none
 *
 * Output:
 * - text to stdout
 *
 * Assumptions:
 * - already validated:     pthread_create()
 *                          pthread_once()
 * - main thread also has a POSIX thread identity
 *
 * Pass Criteria:
 * - stdout matches file reference/tsd1.out
 *
 * Fail Criteria:
 * - fails to match file reference/tsd1.out
 * - output identifies failed component
 */

#include <sched.h>
#include "test.h"

static pthread_key_t key = NULL;
static int accesscount[10];
static int thread_set[10];
static int thread_destroyed[10];

static void
destroy_key(void * arg)
{
  int * j = (int *) arg;

  (*j)++;

  assert(*j == 2);

  thread_destroyed[j - accesscount] = 1;
}

static void
setkey(void * arg)
{
  int * j = (int *) arg;

  thread_set[j - accesscount] = 1;

  assert(*j == 0);

  assert(pthread_getspecific(key) == NULL);

  assert(pthread_setspecific(key, arg) == 0);

  assert(pthread_getspecific(key) == arg);

  (*j)++;

  assert(*j == 1);
}

static void *
mythread(void * arg)
{
  while (key == NULL)
    {
	sched_yield();
    }

  setkey(arg);

  return 0;

  /* Exiting the thread will call the key destructor. */
}

int
main()
{
  int i;
  int fail = 0;
  pthread_t thread[10];

  for (i = 1; i < 5; i++)
    {
	accesscount[i] = thread_set[i] = thread_destroyed[i] = 0;
      assert(pthread_create(&thread[i], NULL, mythread, (void *)&accesscount[i]) == 0);
    }

  Sleep(2000);

  /*
   * Here we test that existing threads will get a key created
   * for them.
   */
  assert(pthread_key_create(&key, destroy_key) == 0);

  /*
   * Test main thread key.
   */
  accesscount[0] = 0;
  setkey((void *) &accesscount[0]);

  /*
   * Here we test that new threads will get a key created
   * for them.
   */
  for (i = 5; i < 10; i++)
    {
	accesscount[i] = thread_set[i] = thread_destroyed[i] = 0;
      assert(pthread_create(&thread[i], NULL, mythread, (void *)&accesscount[i]) == 0);
    }

  /*
   * Wait for all threads to complete.
   */
  for (i = 1; i < 10; i++)
    {
	int result = 0;

	assert(pthread_join(thread[i], (void **) &result) == 0);
    }

  assert(pthread_key_delete(key) == 0);

  for (i = 1; i < 10; i++)
    {
	/*
	 * The counter is incremented once when the key is set to
	 * a value, and again when the key is destroyed. If the key
	 * doesn't get set for some reason then it will still be
	 * NULL and the destroy function will not be called, and
	 * hence accesscount will not equal 2.
	 */
	if (accesscount[i] != 2)
	  {
	    fail++;
	    fprintf(stderr, "Thread %d key, set = %d, destroyed = %d\n",
			i, thread_set[i], thread_destroyed[i]);
	  }
    }

  fflush(stderr);

  return (fail);
}
