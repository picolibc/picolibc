/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/*
 *	Common definitions for atexit-like routines
 */

#include <stdint.h>

enum __atexit_types
{
  __et_atexit,
  __et_onexit,
  __et_cxa
};

#ifdef _REENT_GLOBAL_ATEXIT
#define NEWLIB_THREAD_LOCAL_ATEXIT
#else
#define  NEWLIB_THREAD_LOCAL_ATEXIT NEWLIB_THREAD_LOCAL
#endif

#define	_ATEXIT_SIZE 32	/* must be at least 32 to guarantee ANSI conformance */

struct _on_exit_args {
	void *  _fnargs[_ATEXIT_SIZE];	        /* user fn args */
	void *	_dso_handle[_ATEXIT_SIZE];
	/* Bitmask is set if user function takes arguments.  */
	uint32_t _fntypes;           	        /* type of exit routine -
				   Must have at least _ATEXIT_SIZE bits */
	/* Bitmask is set if function was registered via __cxa_atexit.  */
	uint32_t _is_cxa;
};

struct _atexit {
	struct	_atexit *_next;			/* next in list */
	int	_ind;				/* next index in this table */
	/* Some entries may already have been called, and will be NULL.  */
	void	(*_fns[_ATEXIT_SIZE])(void);	/* the table itself */
        struct _on_exit_args _on_exit_args;
};

extern NEWLIB_THREAD_LOCAL_ATEXIT struct _atexit _atexit0;
extern NEWLIB_THREAD_LOCAL_ATEXIT struct _atexit *_atexit;

void __call_exitprocs (int, void *);
int __register_exitproc (int, void (*fn) (void), void *, void *);

