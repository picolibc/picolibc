/* Copyright (c) 2002, Joerg Wunsch
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

/* $Id: fread.c 1944 2009-04-01 23:12:20Z arcanum $ */

#include "stdio_private.h"

#ifdef __FAST_BUFIO
#include "../stdlib/mul_overflow.h"
#endif

extern FILE *const stdin __weak;
extern FILE *const stdout __weak;

size_t
fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t i, j;
	uint8_t *cp = (uint8_t *) ptr;
	int c;

        __flockfile(stream);
	if ((stream->flags & __SRD) == 0 || size == 0)
		__funlock_return(stream, 0);

#ifdef __FAST_BUFIO
        size_t bytes;
        if ((stream->flags & __SBUF) != 0 &&
            !mul_overflow(size, nmemb, &bytes) && bytes > 0)
        {
                struct __file_bufio *bf = (struct __file_bufio *) stream;
                __ungetc_t unget;
                bool flushed = false;

        again:
                __bufio_lock(stream);
                __bufio_setdir_locked(stream, __SRD);

                /* Deal with any pending unget */
                if ((unget = __atomic_exchange_ungetc(&stream->unget, 0)) != 0) {
                        *cp++ = (unget - 1);
                        bytes--;
                }

                while (bytes) {
                        int this_time = bf->len - bf->off;

                        if (this_time) {
                                /* Drain any buffered data */
                                if (bytes < (size_t) this_time)
                                        this_time = bytes;
                                memcpy(cp, bf->buf + bf->off, this_time);
                                bf->off += this_time;
                                cp += this_time;
                                bytes -= this_time;
                        } else {
                                /* Flush stdout if reading from stdin */
                                if (!flushed) {
                                        flushed = true;
                                        if (&stdin != NULL && &stdout != NULL && stream == stdin) {
                                                __bufio_unlock(stream);
                                                fflush(stdout);
                                                goto again;
                                        }
                                }
                                if (bytes < (size_t) bf->size) {
                                        /* Small reads go through the buffer */
                                        int ret = __bufio_fill_locked(stream);
                                        if (ret) {
                                                stream->flags |= (ret == _FDEV_ERR)? __SERR: __SEOF; 
                                                break;
                                        }
                                } else {
                                        /* Flush any buffered data */
                                        bf->len = 0;
                                        bf->off = 0;

                                        /* Large reads go directly to the destination */
                                        ssize_t len = bufio_read(bf, cp, bytes);
                                        if (len <= 0) {
                                                stream->flags |= (len < 0) ? __SERR : __SEOF;
                                                break;
                                        }
                                        cp += len;
                                        bytes -= len;
                                        bf->pos += len;
                                }
                        }
                }
                __bufio_unlock(stream);
                __funlock_return(stream, (cp - (uint8_t *) ptr) / size);
        }
#endif
	for (i = 0; i < nmemb; i++)
		for (j = 0; j < size; j++) {
			c = getc_unlocked(stream);
			if (c == EOF)
				__funlock_return(stream, i);
			*cp++ = (uint8_t)c;
		}

	__funlock_return(stream, i);
}
