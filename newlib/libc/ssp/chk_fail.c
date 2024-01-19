/* Copyright 2017 Yaakov Selkowitz <yselkowi@redhat.com> */
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#undef puts

static void (*fortify_handler)(int sig);

#define CHK_FAIL_MSG "*** overflow detected ***: terminated"

void
__attribute__((__noreturn__))
__chk_fail(void)
{
#ifdef TINY_STDIO
  puts(CHK_FAIL_MSG);
#else
  static const char msg[] = CHK_FAIL_MSG "\n";
  write (2, msg, sizeof(msg)-1);
#endif
  if (fortify_handler)
      (*fortify_handler)(SIGABRT);
  abort();
}

void
set_fortify_handler (void (*handler) (int sig))
{
    fortify_handler = handler;
}
