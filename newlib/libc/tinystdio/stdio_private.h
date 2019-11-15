/* Copyright (c) 2002,2005, Joerg Wunsch
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/* $Id: stdio_private.h 847 2005-09-06 18:49:15Z joerg_wunsch $ */

#include <stdio.h>

/* values for PRINTF_LEVEL */
#define PRINTF_MIN 1
#define PRINTF_STD 2
#define PRINTF_FLT 3

/* values for SCANF_LEVEL */
#define SCANF_MIN 1
#define SCANF_STD 2
#define SCANF_FLT 3

struct __file_str {
	struct __file file;	/* main file struct */
	char	*buf;		/* buffer pointer */
	int	size;		/* size of buffer */
};

#ifdef POSIX_IO

struct __file_posix {
	struct __file_close cfile;
	int	fd;
	char	*write_buf;
	int	write_len;
	char	*read_buf;
	int	read_len;
	int	read_off;
};

int
__posix_sflags (const char *mode, int *optr);

int
__posix_flush(FILE *f);

int
__posix_putc(char c, FILE *f);

int
__posix_getc(FILE *f);

int
__posix_close(FILE *f);

#endif
