/*
 * arc-main-helper.c -- provide custom __setup_argv_and_call_main();
 * This function uses _argc(), _argvlen(), _argv() and _setup_low_level().
 * Description for these functions can be found below in this file.
 *
 * Copyright (c) 2024 Synopsys Inc.
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
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <alloca.h>

/* Return number of arguments passed to host executable.  */
extern int _argc (void);
/*
 * Return buffer length to be used for a given argument number.
 * Buffer length includes '\0' character.
 */
extern uint32_t _argvlen (int a);
/*
 * Copy argument into buffer arg.
 * arg must be no less than argvlen(a) length.
 */
extern int _argv (int a, char *arg);
/* Custom setup.  Can be used to setup some port-specific stuff.  */
extern int _setup_low_level (void);
/* main function to call.  */
extern int main (int argc, char **argv);

/* Copy arguments from host to local stack and call main.  */
int
__setup_argv_and_call_main (void)
{
  int i;
  int argc;
  char **argv = NULL;

  argc = _argc ();
  if (argc > 0)
    argv = alloca ((argc + 1) * sizeof (char *));

  for (i = 0; i < argc; i++)
    {
      uint32_t arg_len;

      arg_len = _argvlen (i);
      if (arg_len == 0)
	break;

      argv[i] = alloca (arg_len);

      if (_argv (i, argv[i]) < 0)
	break;
    }

  if (argv)
    argv[i] = NULL;

  _setup_low_level ();

  return main (i, argv);
}
