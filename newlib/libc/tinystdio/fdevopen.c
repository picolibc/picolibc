/* Copyright (c) 2002,2005, 2007 Joerg Wunsch
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

/* $Id: fdevopen.c 1944 2009-04-01 23:12:20Z arcanum $ */

#include "stdio_private.h"

/** \file */

/** \ingroup avr_stdio
   This function is a replacement for \c fopen().

   It opens a stream for a device where the actual device
   implementation needs to be provided by the application.  If
   successful, a pointer to the structure for the opened stream is
   returned.  Reasons for a possible failure currently include that
   neither the \c put nor the \c get argument have been provided, thus
   attempting to open a stream with no IO intent at all, or that
   insufficient dynamic memory is available to establish a new stream.

   If the \c put function pointer is provided, the stream is opened
   with write intent.  The function passed as \c put shall take two
   arguments, the first a character to write to the device,
   and the second a pointer to FILE, and shall return 0
   if the output was successful, and a nonzero value if the character
   could not be sent to the device.

   If the \c get function pointer is provided, the stream is opened
   with read intent.  The function passed as \c get shall take
   a pointer to FILE as its single argument,
   and return one character from the device, passed as an
   \c int type.  If an error occurs when trying to read from the
   device, it shall return \c _FDEV_ERR.
   If an end-of-file condition was reached while reading from the
   device, \c _FDEV_EOF shall be returned.

   If both functions are provided, the stream is opened with read
   and write intent.

   If the \c flush function pointer is provided, then it will be
   called whenever the application calls the fflush function on the
   file. This allows the underlying I/O implementation to perform
   buffering as needed.

   fdevopen() uses calloc() (und thus malloc()) in order to allocate
   the storage for the new stream.

   \note If the macro __STDIO_FDEVOPEN_COMPAT_12 is declared before
   including <stdio.h>, a function prototype for fdevopen() will be
   chosen that is backwards compatible with avr-libc version 1.2 and
   before.  This is solely intented for providing a simple migration
   path without the need to immediately change all source code.  Do
   not use for new code.
*/

static int
fdevclose(FILE *f)
{
	int ret = 0;
	if  (f->flush)
		ret = (*f->flush)(f);
	free(f);
	return ret;
}

FILE *
fdevopen(int (*put)(char, FILE *), int (*get)(FILE *), int (*flush)(FILE *))
{
	struct __file_close *cf;

	if (put == 0 && get == 0)
		return 0;

	if ((cf = calloc(1, sizeof(*cf))) == 0)
		return 0;

	cf->file.flags = __SCLOSE;
	cf->close = fdevclose;

	if (get != 0) {
		cf->file.get = get;
		cf->file.flags |= __SRD;
	}

	if (put != 0) {
		cf->file.put = put;
		cf->file.flush = flush;
		cf->file.flags |= __SWR;
	}

	return (FILE *) cf;
}
