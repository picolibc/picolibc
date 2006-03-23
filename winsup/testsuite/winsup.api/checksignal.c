#include <errno.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"

const char *TCID = "checksignal";    	/* Test program identifier. */
int TST_TOTAL = 3;              /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */

void
sig_handler (int signo)
{
  errno = EINVAL;
}

int
main ()
{
  int n, ret;
  int fds[2];
  char buf[10];
  struct sigaction act;
  int i;

  Tst_count = 0;

  /* Reset SA_RESTART flag. */
  while ((ret = sigaction (SIGALRM, NULL, &act)) == EINTR)
    ;
  if (ret)
    tst_brk (TBROK, NULL, NULL, "Get signal action structure");
  act.sa_flags &= ~SA_RESTART;
  while ((ret = sigaction (SIGALRM, &act, NULL)) == EINTR)
    ;
  if (ret)
    tst_brk (TBROK, NULL, NULL, "Reset SA_RESTART");

  for (i = 0; i < 2; i++)
    {
      if (pipe (fds) < 0)
	tst_brk (TBROK, NULL, NULL, "Create pipe");
      /* Set signal handler using signal(2) call... */
      if (signal (SIGALRM, sig_handler) < 0)
	tst_brk (TBROK, NULL, NULL, "Call signal() to install signal handler");
      /* ...and check if signal(2) sets SA_RESTART again. */
      while ((ret = sigaction (SIGALRM, NULL, &act)) == EINTR)
	;
      if (ret)
	tst_brk (TBROK, NULL, NULL, "Get signal action structure");
      tst_resm (act.sa_flags & SA_RESTART ? TPASS : TFAIL,
		"signal() sets SA_RESTART");

      const char *msg1, *msg2;
      if (i == 1)
	{
	  siginterrupt (SIGALRM, 1);
	  msg1 = "Reset SA_RESTART via siginterrupt";
	  msg2 = "Set EINTR on interrupted read() call via siginterrupt";
	}
      else
	{
	  /* Reset SA_RESTART flag again. */
	  act.sa_handler = sig_handler;
	  act.sa_flags &= ~SA_RESTART;
	  while ((ret = sigaction (SIGALRM, &act, NULL)) == EINTR)
	    ;
	  msg1 = "Reset SA_RESTART via sigaction";
	  msg2 = "Set EINTR on interrupted read() call via sigaction";
	}
      if (ret)
	tst_brk (TBROK, NULL, NULL, msg1);

      /* Start timer to force a SIGALRM. */
      alarm (1);

      /* Call read(2) to check if the EINTR errno is correctly preserved,
	 even if the signal handler routine changes errno. */
      n = read(fds[0], buf, 10);
      tst_resm (n < 0 && errno == EINTR ? TPASS : TFAIL, msg2);
      close (fds[0]);
      close (fds[1]);
    }

  /* Check if another errno is correctly returned (here EBADF). */
  n = read(fds[0], buf, 10);
  tst_resm (n < 0 && errno == EBADF ? TPASS : TFAIL,
  	    "Set EBADF on closed file descriptor");

  tst_exit ();
}
