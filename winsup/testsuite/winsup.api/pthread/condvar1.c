/*
 * File: condvar1.c
 *
 * Test Synopsis:
 * - Test initialisation and destruction of a CV.
 *
 * Test Method (Validation or Falsification):
 * - Validation
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
 * - Creates and then imediately destroys a CV. Does not
 *   test the CV.
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
 * - 
 *
 * Pass Criteria:
 * - pthread_cond_init returns 0, and
 * - pthread_cond_destroy returns 0.
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - pthread_cond_init returns non-zero, or
 * - pthread_cond_destroy returns non-zero.
 * - Process returns non-zero exit status.
 */

#include "test.h"

static pthread_cond_t cv = NULL;

int
main()
{
  assert(cv == NULL);

  assert(pthread_cond_init(&cv, NULL) == 0);

  assert(cv != NULL);

  assert(pthread_cond_destroy(&cv) == 0);

  assert(cv == NULL);

  return 0;
}
