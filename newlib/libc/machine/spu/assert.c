#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * SPU specific assert: just directly call exit(6), and use fprintf. That
 * way we do not pull in the abort, signal.o code, nor (the likely
 * otherwise unused) fiprintf.
 */

/* func can be NULL, in which case no function information is given.  */
void
_DEFUN (__assert_func, (file, line, func, failedexpr),
	const char *file _AND
	int line _AND
	const char *func _AND
	const char *failedexpr)
{
  fprintf(stderr,
	   "assertion \"%s\" failed: file \"%s\", line %d%s%s\n",
	   failedexpr, file, line,
	   func ? ", function: " : "", func ? func : "");
  /*
   * On the SPU, we do not have signaling. Previously, standard newlib
   * abort code was used. That eventually leads to a kill(SIGABRT), and
   * for SPU too an exit(SIGABRT). SIGABRT was 6, so just use that value
   * here.
   */
  exit(6);
  /* NOTREACHED */
}

void
_DEFUN (__assert, (file, line, failedexpr),
	const char *file _AND
	int line _AND
	const char *failedexpr)
{
   __assert_func (file, line, NULL, failedexpr);
  /* NOTREACHED */
}
