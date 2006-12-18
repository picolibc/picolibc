/*
(C) Copyright IBM Corp. 2006

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
    * Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

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

Author: Joel Schopp <jschopp@austin.ibm.com>
*/

#include <errno.h>
#include <spu_intrinsics.h>

#define SPE_C99_SIGNALCODE 0x2100

#define SPE_C99_OP_SHIFT    	24
#define SPE_C99_OP_MASK	    	0xff
#define SPE_C99_DATA_MASK   	0xffffff

enum {
	SPE_C99_CLEARERR=0x01,
	SPE_C99_FCLOSE,
	SPE_C99_FEOF,
	SPE_C99_FERROR,
	SPE_C99_FFLUSH,
	SPE_C99_FGETC,
	SPE_C99_FGETPOS,
	SPE_C99_FGETS,
	SPE_C99_FILENO,
	SPE_C99_FOPEN, //implemented
	SPE_C99_FPUTC,
	SPE_C99_FPUTS,
	SPE_C99_FREAD,
	SPE_C99_FREOPEN,
	SPE_C99_FSEEK,
	SPE_C99_FSETPOS,
	SPE_C99_FTELL,
	SPE_C99_FWRITE,
	SPE_C99_GETC,
	SPE_C99_GETCHAR,
	SPE_C99_GETS,
	SPE_C99_PERROR,
	SPE_C99_PUTC,
	SPE_C99_PUTCHAR,
	SPE_C99_PUTS,
	SPE_C99_REMOVE,
	SPE_C99_RENAME,
	SPE_C99_REWIND,
	SPE_C99_SETBUF,
	SPE_C99_SETVBUF,
	SPE_C99_SYSTEM, //not yet implemented in newlib
	SPE_C99_TMPFILE,
	SPE_C99_TMPNAM,
	SPE_C99_UNGETC,
	SPE_C99_VFPRINTF,
	SPE_C99_VFSCANF,
	SPE_C99_VPRINTF,
	SPE_C99_VSCANF,
	SPE_C99_VSNPRINTF,
	SPE_C99_VSPRINTF,
	SPE_C99_VSSCANF,
	SPE_C99_LAST_OPCODE,
};
#define SPE_C99_NR_OPCODES 	((SPE_C99_LAST_OPCODE - SPE_C99_CLEARERR) + 1)

#define SPE_STDIN                   1
#define SPE_STDOUT                  2
#define SPE_STDERR                  3
#define SPE_FOPEN_MAX               (FOPEN_MAX+1)
#define SPE_FOPEN_MIN               4

struct spe_reg128{
  unsigned int slot[4];
};

static void
send_to_ppe(int signalcode, int opcode, void *data)
{

	unsigned int	combined = ( ( opcode<<24 )&0xff000000 ) | ( ( unsigned int )data & 0x00ffffff );
	struct spe_reg128* ret = data;

        vector unsigned int stopfunc = {
                signalcode,     /* stop 0x210x*/
                (unsigned int) combined,
                0x4020007f,     /* nop */
                0x35000000      /* bi $0 */
        };

        void (*f) (void) = (void *) &stopfunc;
        asm ("sync":::"memory");
        f();
	errno = ret->slot[3];
	return;
}
