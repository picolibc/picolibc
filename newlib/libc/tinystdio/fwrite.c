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

/* $Id: fwrite.c 1944 2009-04-01 23:12:20Z arcanum $ */

#include "stdio_private.h"

#ifdef _WANT_FAST_BUFIO
#include "../stdlib/mul_overflow.h"
#endif

size_t
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t i, j;
	const uint8_t *cp = (const uint8_t *) ptr;

	if ((stream->flags & __SWR) == 0 || size == 0)
		return 0;

#ifdef _WANT_FAST_BUFIO
        size_t bytes;
        struct __file_bufio *bf = (struct __file_bufio *) stream;

        if ((stream->flags & __SBUF) != 0 &&
            (bf->bflags & __BLBF) == 0 &&
            !mul_overflow(size, nmemb, &bytes) &&
            bytes > 0)
        {
                __bufio_lock(stream);
                __bufio_setdir_locked(stream, __SWR);

                if (bytes < (unsigned) bf->size) {
                        /* Small writes go through the buffer. */
                        while (bytes) {
                                int this_time = bf->size - bf->len;
                                if (this_time == 0) {
                                        int ret = __bufio_flush_locked(stream);
                                        if (ret) {
                                                stream->flags |= ret;
                                                break;
                                        }
                                }
                                if ((unsigned) this_time > bytes)
                                        this_time = bytes;
                                memcpy(bf->buf + bf->len, cp, this_time);
                                bf->len += this_time;
                                cp += this_time;
                                bytes -= this_time;
                        }
                } else {
                        /* Large writes go direct. */
                        if (__bufio_flush_locked(stream) >= 0) {
                                while (bytes) {
                                        ssize_t len = (bf->write)(bf->fd, cp, bytes);
                                        if (len <= 0) {
                                                stream->flags |= _FDEV_ERR;
                                                break;
                                        }
                                        bytes -= len;
                                        cp += len;
                                        bf->pos += len;
                                }
                        }
                }
                __bufio_unlock(stream);
                return (cp - (uint8_t *) ptr) / size;
        }
#endif
	for (i = 0; i < nmemb; i++)
		for (j = 0; j < size; j++)
			if (stream->put(*cp++, stream) < 0)
				return i;

	return i;
}
