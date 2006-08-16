/*
(C) Copyright IBM Corp. 2005, 2006

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

Author: Andreas Neukoetter (ti95neuk@de.ibm.com)
*/

#include <sys/types.h>
#include <errno.h>

extern int errno;

extern caddr_t  _end;
#define RAMSIZE 262144
#define STACKSIZE 4096

void *
sbrk (ptrdiff_t increment)
{
	static caddr_t heap_ptr = NULL;
	caddr_t base;

	if (heap_ptr == NULL)
	  {
		  heap_ptr = (caddr_t) & _end;
	  }
	if (((RAMSIZE - STACKSIZE) - (int) heap_ptr) >= increment)
	  {
		  base = heap_ptr;
		  heap_ptr += increment;
		  return (base);
	  }
	else
	  {
		  errno = ENOMEM;
		  return ((void *) -1);
	  }
}
