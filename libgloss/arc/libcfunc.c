/*
   Copyright (c) 2015, Synopsys, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1) Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2) Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   3) Neither the name of the Synopsys, Inc., nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

/* This file has been copied from libgloss/arm/libcfuncs.c.

   Support files for GNU libc.  Files in the C namespace go here.
   Files in the system namespace (ie those that start with an underscore)
   go in syscalls.c.

   Note: These functions are in a seperate file so that OS providers can
   overrride the system call stubs (defined in syscalls.c) without having
   to provide libc functions as well.  */

#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

unsigned __attribute__((weak))
alarm (unsigned seconds)
{
	(void)seconds;
	return 0;
}

clock_t _clock (void);
clock_t __attribute__((weak))
clock (void)
{
      return _clock ();
}

int _isatty (int fildes);
int __attribute__((weak))
isatty (int fildes)
{
	return _isatty (fildes);
}

int __attribute__((weak))
pause (void)
{
	errno = ENOSYS;
	return -1;
}

unsigned __attribute__((weak))
sleep (unsigned seconds)
{
	clock_t t0 = _clock ();
	clock_t dt = seconds * CLOCKS_PER_SEC;

	while (_clock () - t0  < dt);
	return 0;
}

int __attribute__((weak))
usleep (useconds_t useconds)
{
	clock_t t0 = _clock ();
	clock_t dt = useconds / (1000000/CLOCKS_PER_SEC);

	while (_clock () - t0  < dt);
	return 0;
}
