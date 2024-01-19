/*
 * Support file for amdgcn in newlib.
 * Copyright (c) 2017 Mentor Graphics.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#ifndef _AMDGCN_EXIT_VALUE_H_
#define _AMDGCN_EXIT_VALUE_H_

static inline void  __attribute__((noreturn))
exit_with_int (int val)
{
  /* Write the exit value to the conventional place.  */
  int *return_value;
#if defined(__has_builtin) && __has_builtin(__builtin_gcn_kernarg_ptr)
  __asm__ ("s_load_dwordx2	%0, %1, 16 glc\n\t"
       "s_waitcnt	0"
       : "=Sg"(return_value) : "r"(__builtin_gcn_kernarg_ptr()));
#else
  __asm__ ("s_load_dwordx2	%0, s[8:9], 16 glc\n\t"
       "s_waitcnt	0" : "=Sg"(return_value));
#endif
  *return_value = val;

  /* Terminate the current kernel.  */
  __asm__ ("s_endpgm");
  __builtin_unreachable ();
}

static inline void  __attribute__((noreturn))
exit_with_status_and_signal (int val, int signal)
{
  if (signal == 0)
    val = val & 0xff;
  else
    {
      val = (128 + signal) & 0xff;
      signal = signal & 0xff;
    }

  exit_with_int ((0xffff << 16) | (signal << 8) | val);
}

#endif
