/* libc/sys/linux/siglongjmp.c - siglongjmp function */

/* Copyright 2002, Red Hat Inc. */


#include <setjmp.h>
#include <signal.h>

void
siglongjmp (sigjmp_buf env, int val)
{
  if (env.__is_mask_saved)
    sigprocmask (SIG_SETMASK, &env.__saved_mask, NULL);

  longjmp (env.__buf, val);
}
