/*
 *	Common definitions for atexit-like routines
 */

enum __atexit_types
{
  __et_atexit,
  __et_onexit,
  __et_cxa
};

void __call_exitprocs (int, void *);
int __register_exitproc (int, void (*fn) (void), void *, void *);

