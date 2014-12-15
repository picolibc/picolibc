/*
Copyright (c) 2013 Andes Technology Corporation.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of the company may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
  SYNOPSIS
    #include <unistd.h>
    int isatty(int fildes);
  RETURN VALUE
    The isatty() function shall return 1 if fildes is associated with
    a terminal;	 otherwise, it shall return 0 and may set errno
    to indicate the error.
*/
#ifdef __NDS32_VH__

#include "vh.h"
.extern _impure_ptr
TYPE0 _isatty, VH_ISATTY

#else	/* not __NDS32_VH__ */

#include "../syscall.h"
#include "syscall_extra.h"
	.text
	.global	_isatty
	.type	_isatty, @function
	.align	2
_isatty:
	syscall	SYS_isatty	/* Make syscall with SWID=`SYS_isatty'.
				   Reture value `0' stored in $r0 means
				   there is something wrong.  */
	bnez	$r0, 1f		/* Branch if success.  */
	syscall	SYS_geterr	/* There is something wrong.  */
	l.w     $r15, _impure_ptr
	swi     $r0, [$r15]     /* Set errno.  */
	move	$r0, #0
1:
	ret
	.size   _isatty, .-_isatty

#endif	/* not __NDS32_VH__ */
