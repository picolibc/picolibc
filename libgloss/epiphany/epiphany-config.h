/* EPIPHANY builtin and configuration functions ()

   Copyright (c) 2011, Adapteva, Inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Adapteva nor the names of its contributors may be
      used to endorse or promote products derived from this software without
      specific prior written permission.

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
   POSSIBILITY OF SUCH DAMAGE.                                               */


#ifndef _EPIPHANY_CONFIG_H
#define _EPIPHANY_CONFIG_H


extern unsigned _stack_start_;
extern unsigned _heap_start_;
extern unsigned _heap_end_;
extern unsigned _CORE_NUM_;





enum ECORE_SIGNALS {
#ifndef 	SIG_DFL
   SIG_DFL=0,
#endif
#ifndef 	SIG_DFL
   SIG_IGN=1,
#endif
   SIG_RESET=3,
   SIG_SW_EXCEPTION=4,
   SIG_PAGE_MISS=5,
   SIG_TIMER0=6,
   SIG_TIMER1=7,
   SIG_MESSAGE=8,
   SIG_DMA0=9,
   SIG_DMA1=10,
   SIG_WAND=11,
   SIG_USR1=12
#ifndef 	SIG_ERR
   , SIG_ERR=-1
#endif
};



#endif
