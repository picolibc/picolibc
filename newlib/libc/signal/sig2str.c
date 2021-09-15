/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2021 Matthew Joyce
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
FUNCTION
<<sig2str>>, <<str2sig>>---Translate between signal number and name

INDEX
          sig2str
INDEX
          str2sig

SYNOPSIS
          #include <signal.h>
          int sig2str(int <[signum]>, char *<[str]>);

          int str2sig(const char *restrict <[str]>, int *restrict <[pnum]>);

DESCRIPTION
The <<sig2str>> function translates the signal number specified by <[signum]> to
a signal name and stores this string in the location specified by <[str]>. The
application must ensure that <[str]> points to a location that can store the
string including the terminating null byte. The symbolic constant
<[SIG2STR_MAX]> defined in `<<signal.h>>' gives the maximum number of bytes
required.

The <<str2sig>> function translates the signal name in the string pointed to by
<[str]> to a signal number and stores this value in the location specified by
<[pnum]>.

RETURNS
<<sig2str>> returns <<0>> if <[signum]>> is a valid, supported signal number.
Otherwise, it returns <<-1>>.

<<str2sig>> returns <<0>> if it stores a value in the location pointed to by
<[pnum]>. Otherwise it returns <<-1>>.
*/

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SPACES_TO_N 6 /* Allows indexing to RT Signal number in str2sig */
#define NUM_OF_SIGS (sizeof(sig_array) / sizeof(sig_name_and_num))

typedef struct sig_name_and_num {
  const char *sig_name;
  const int  sig_num;
} sig_name_and_num;

static const sig_name_and_num sig_array[] = {
      { "EXIT", 0 },
    #ifdef SIGHUP
      { "HUP", SIGHUP },
    #endif
    #ifdef SIGINT
      { "INT", SIGINT },
    #endif
    #ifdef SIGQUIT
      { "QUIT", SIGQUIT },
    #endif
    #ifdef SIGILL
      { "ILL", SIGILL },
    #endif
    #ifdef SIGTRAP
      { "TRAP", SIGTRAP },
    #endif
    #ifdef SIGABRT
      { "ABRT", SIGABRT },
    #endif
    #ifdef SIGIOT
      { "IOT", SIGIOT},
    #endif
    #ifdef SIGEMT
      { "EMT", SIGEMT },
    #endif
    #ifdef SIGFPE
      { "FPE", SIGFPE },
    #endif
    #ifdef SIGKILL
      { "KILL", SIGKILL },
    #endif
    #ifdef SIGBUS
      { "BUS", SIGBUS },
    #endif
    #ifdef SIGSEGV
      { "SEGV", SIGSEGV },
    #endif
    #ifdef SIGSYS
      { "SYS", SIGSYS },
    #endif
    #ifdef SIGPIPE
      { "PIPE", SIGPIPE },
    #endif
    #ifdef SIGALRM
      { "ALRM", SIGALRM },
    #endif
    #ifdef SIGTERM
      { "TERM", SIGTERM },
    #endif
    #ifdef SIGURG
      { "URG", SIGURG },
    #endif
    #ifdef SIGSTOP
      { "STOP", SIGSTOP },
    #endif
    #ifdef SIGTSTP
      { "TSTP", SIGTSTP },
    #endif
    #ifdef SIGCONT
      { "CONT", SIGCONT },
    #endif
    #ifdef SIGCHLD
      { "CHLD", SIGCHLD },
    #endif
    #ifdef SIGCLD
      { "CLD", SIGCLD },
    #endif
    #ifdef SIGTTIN
      { "TTIN", SIGTTIN },
    #endif
    #ifdef SIGTTOU
      { "TTOU", SIGTTOU },
    #endif
    #ifdef SIGIO
      { "IO", SIGIO },
    #endif
    #ifdef SIGPOLL
      { "POLL", SIGPOLL },
    #endif
    #ifdef SIGWINCH
      { "WINCH", SIGWINCH },
    #endif
    #ifdef SIGUSR1
      { "USR1", SIGUSR1 },
    #endif
    #ifdef SIGUSR2
      { "USR2", SIGUSR2 },
    #endif
    #ifdef SIGPWR
      { "PWR", SIGPWR },
    #endif
    #ifdef SIGXCPU
      { "XCPU", SIGXCPU },
    #endif
    #ifdef SIGXFSZ
      { "XFSZ", SIGXFSZ },
    #endif
    #ifdef SIGVTALRM
      { "VTALRM", SIGVTALRM },
    #endif
    #ifdef SIGPROF
      { "PROF", SIGPROF },
    #endif
    #ifdef SIGLOST
      { "LOST", SIGLOST },
    #endif
    /* The Issue 8 standard requires that SIGRTMIN and SIGRTMAX be included
     * as valid results to be saved from calls to sig2str/str2sig.  */
    #ifdef SIGRTMIN
      { "RTMIN", SIGRTMIN },
    #endif
    #ifdef SIGRTMAX
      { "RTMAX", SIGRTMAX }
    #endif
};

int
sig2str(int signum, char *str)
{
  const sig_name_and_num *sptr;

#if defined (SIGRTMIN) && defined (SIGRTMAX)
  /* If signum falls in lower half of the real time signals range, define
   * the saved str value as "RTMIN+n" according to the Issue 8 standard */
  if ((SIGRTMIN + 1) <= signum &&
      signum <= (SIGRTMIN + SIGRTMAX) / 2) {
    sprintf(str, "RTMIN+%d", (signum-SIGRTMIN));
    return 0;
  }

  /* If signum falls in upper half of the real time signals range, define
   * the saved str value as "RTMAX-m" according to the Issue 8 standard */
  if ((((SIGRTMIN + SIGRTMAX) / 2) + 1) <= signum &&
         signum <= (SIGRTMAX - 1)) {
    sprintf(str, "RTMAX-%d", (SIGRTMAX - signum));
    return 0;
  }
#endif

  /* Otherwise, search for signal matching signum in sig_array. If found,
   * save its string value in str. */
  for (sptr = sig_array; sptr < &sig_array[NUM_OF_SIGS]; sptr++) {
    if (sptr->sig_num == signum) {
      strcpy(str, sptr->sig_name);
      return 0;
    }
  }

  /* If signum is not a recognized signal number, return -1 */
  return -1;
}

int
str2sig(const char *__restrict str, int *__restrict pnum)
{
  unsigned long j = 0;
  char *endp;
  const sig_name_and_num *sptr;
  unsigned long is_valid_decimal;

#if defined (SIGRTMIN) && defined (SIGRTMAX)
  /* i686 Cygwin only supports one RT signal. For this case, skip checks
   * for "RTMIN+n" and "RTMAX-m". */
  if (SIGRTMIN != SIGRTMAX) {

    /* If str is in RT signal range, get number of of RT signal, save it as an
    * integer. */
    if (strncmp(str, "RTMIN+", SPACES_TO_N) == 0) {
      j = strtoul(&str[SPACES_TO_N], &endp, 10);

      /* If number is valid, save it in pnum. */
      if (*endp == '\0') {
        if (1 <= j &&
            j <= ((SIGRTMAX - SIGRTMIN)-1)) {
          *pnum = (SIGRTMIN + j);
          return 0;
        }
        return -1;
      }
      return -1;
    }

    /* If str is in RT signal range, get number of of RT signal, save it as an
    * integer. */
    if (strncmp(str, "RTMAX-", SPACES_TO_N) == 0) {
      j = strtoul(&str[SPACES_TO_N], &endp, 10); // and endptr null check

      /* If number is valid, save it in pnum. */
      if (*endp == '\0') {
        if (1 <= j &&
            j <= ((SIGRTMAX - SIGRTMIN)-1)) {
          *pnum = (SIGRTMAX - j);
          return 0;
        }
        return -1;
      }
      return -1;
    }
  }
#endif

  /*If str is a valid signal name, save its corresponding number in pnum. */
  for (sptr = sig_array; sptr < &sig_array[NUM_OF_SIGS]; sptr++) {
    if (strcmp(sptr->sig_name, str) == 0) {
      *pnum = sptr->sig_num;
      return 0;
    }
  }

  /* str was not found in sig_array. Check whether str is a string
   * representation of a valid integer. */
  is_valid_decimal = strtoul(str, &endp, 10);

  if (*endp != '\0') {
    return -1;
  }

  /* If str is a representation of a decimal value, save its integer value
   * in pnum. */
  if (1 <= is_valid_decimal &&
      is_valid_decimal <= (NSIG - 1)) {
    *pnum = is_valid_decimal;
    return 0;
  }

  return -1;
}
