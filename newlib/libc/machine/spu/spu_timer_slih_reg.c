/*
(C) Copyright IBM Corp. 2008

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
*/


/* Services for SLIH registration.  */
#include <spu_intrinsics.h>
#include <spu_timer.h>

#define SPU_EVENT_ID(_mask) \
    (spu_extract(spu_cntlz(spu_promote(_mask, 0)), 0))
typedef unsigned (*spu_slih_t) (unsigned);

extern spu_slih_t __spu_slih_handlers[];

/* This function is called whenever an event occurs for which no second level
   event handler was registered. The default event handler does nothing and
   zeros the most significant event bit indicating that the event was processed
   (when in reality, it was discarded).  */
unsigned
__spu_default_slih (unsigned events)
{
  unsigned int mse;

  mse = 0x80000000 >> SPU_EVENT_ID (events);
  events &= ~mse;

  return (events);
}

/* Registers a SPU second level interrupt handler for the events specified by
   mask. The event mask consists of a set of bits corresponding to the event
   status bits (see channel 0 description). A mask containing multiple  1 bits
    will set the second level event handler for each of the events.  */
void
spu_slih_register (unsigned mask, spu_slih_t func)
{
  unsigned int id;

  while (mask)
    {
      id = SPU_EVENT_ID (mask);
      __spu_slih_handlers[id] = (func) ? func : __spu_default_slih;
      mask &= ~(0x80000000 >> id);
    }
}
