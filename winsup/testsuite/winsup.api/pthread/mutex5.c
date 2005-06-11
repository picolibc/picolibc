/* 
 * mutex5.c
 *
 * Confirm the equality/inequality of the various mutex types,
 * and the default not-set value.
 */

#include "test.h"

static pthread_mutexattr_t mxAttr;

int
main()
{
  int mxType = -1;
  int success = 0;   /* Use to quell GNU compiler warnings. */

  assert(success = PTHREAD_MUTEX_DEFAULT == PTHREAD_MUTEX_NORMAL);
  assert(success = PTHREAD_MUTEX_DEFAULT != PTHREAD_MUTEX_RECURSIVE);
  assert(success = PTHREAD_MUTEX_RECURSIVE != PTHREAD_MUTEX_ERRORCHECK);

  if (success == success)
    {
      assert(pthread_mutexattr_init(&mxAttr) == 0);
      assert(pthread_mutexattr_gettype(&mxAttr, &mxType) == 0);
      assert(mxType == PTHREAD_MUTEX_ERRORCHECK);
    }

  return 0;
}
