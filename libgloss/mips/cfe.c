/* cfe.c -- I/O code for the MIPS boards running CFE.  */

/*
 * Copyright 2001, 2002
 * Broadcom Corporation. All rights reserved.
 * 
 * This software is furnished under license and may be used and copied only
 * in accordance with the following terms and conditions.  Subject to these
 * conditions, you may download, copy, install, use, modify and distribute
 * modified or unmodified copies of this software in source and/or binary
 * form. No title or ownership is transferred hereby.
 * 
 * 1) Any source code used, modified or distributed must reproduce and
 *    retain this copyright notice and list of conditions as they appear in
 *    the source file.
 * 
 * 2) No right is granted to use any trade name, trademark, or logo of
 *    Broadcom Corporation.  The "Broadcom Corporation" name may not be
 *    used to endorse or promote products derived from this software
 *    without the prior written permission of Broadcom Corporation.
 * 
 * 3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR IMPLIED
 *    WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED WARRANTIES OF
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR
 *    NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT SHALL BROADCOM BE LIABLE
 *    FOR ANY DAMAGES WHATSOEVER, AND IN PARTICULAR, BROADCOM SHALL NOT BE
 *    LIABLE FOR DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 */

#include "cfe_api.h"

char inbyte (void);
int outbyte (char c);

/* Make sure cfe_prestart is used.  It doesn't look like setting the
   entry symbol in the linker script to a symbol from that fiel will do
   this!  */
extern int _prestart;
static void *force_prestart = &_prestart;

/* The following variables are initialized to non-zero so that they'll be
   in data, rather than BSS.  Used to be that you could init variables to
   any value to put them into initialized data sections rather than BSS,
   but that decades-old idiom went out the window with gcc 3.2.  Now,
   either you compile specially (with -fno-zero-initialized-in-bss), or
   you init to non-zero.  In this case, initting to non-zero is OK (and
   even beneficial; alignment fault via jump to odd if not properly
   set up by _prestart()), so we do the latter.  */
unsigned int __cfe_handle = 0xdeadbeef;
unsigned int __cfe_entrypt = 0xdeadbeef;

/* Echo input characters?  */
int	__cfe_echo_input = 0;

/* CFE handle used to access console device.  */
static int cfe_conshandle;

char
inbyte (void)
{
  unsigned char c;
  int rv;

  while (cfe_read (cfe_conshandle, &c, 1) != 1)
    ;
  if (c == '\r')
    c = '\n';
  if (__cfe_echo_input)
    outbyte (c);
  return c;
}

int
outbyte (char c)
{
  int res;

  do
    {
      res = cfe_write (cfe_conshandle, &c, 1);
    }
  while (res == 0);
  if (c == '\n')
    outbyte ('\r');
  return 0;
}

/* Initialize hardware.  Called from crt0.  */
void
hardware_init_hook(void)
{
  cfe_init (__cfe_handle, __cfe_entrypt);
  cfe_conshandle = cfe_getstdhandle(CFE_STDHANDLE_CONSOLE);
}

/* Avoid worst-case execution hazards.  This is targetted at the SB-1
   pipe, and is much worse than it needs to be (not even counting
   the subroutine call and return).  */
void
hardware_hazard_hook(void)
{
  __asm__ __volatile__ ("	.set push		\n"
			"	.set mips32		\n"
			"	.set noreorder		\n"
			"	ssnop			\n"
			"	ssnop			\n"
			"	ssnop			\n"
			"	bnel	$0, $0, .+4	\n"
			"	ssnop			\n"
			"	.set pop		\n");
}

/* Exit back to monitor, with the given status code.  */
void
hardware_exit_hook (int status)
{
  	outbyte ('\r');
  	outbyte ('\n');
	cfe_exit (CFE_FLG_WARMSTART, status);
}

/* Structure filled in by get_mem_info.  Only the size field is
   actually used (by sbrk), so the others aren't even filled in.  */
struct s_mem
{
  unsigned int size;
  unsigned int icsize;
  unsigned int dcsize;
};

void
get_mem_info (mem)
     struct s_mem *mem;
{
  /* XXX FIXME: Fake this for now.  Should invoke cfe_enummem, but we
     don't have enough stack to do that (yet).  */
  mem->size = 0x4000000;	/* Assume 64 MB of RAM */
}
