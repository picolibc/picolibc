/*
Copyright 2006,2008,                                         
International Business Machines Corporation                     
All Rights Reserved.                                            

Redistribution and use in source and binary forms, with or      
without modification, are permitted provided that the           
following conditions are met:                                   

Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer. 

Redistributions in binary form must reproduce the above       
copyright notice, this list of conditions and the following   
disclaimer in the documentation and/or other materials        
provided with the distribution.                               

Neither the name of IBM Corporation nor the names of its      
contributors may be used to endorse or promote products       
derived from this software without specific prior written     
permission.                                                   

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND          
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,     
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT    
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)        
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN       
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR    
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <errno.h>
#include "headers/truncf4.h"
#include "headers/tgammaf4.h"

static __inline float _tgammaf(float x)
{
  float res;
  vector float vx;
  vector float truncx;
  vector float vc = { 0.0, 0.0 };
  vector unsigned int cmpres;
  vector signed int verrno, ferrno;
  vector signed int fail = { EDOM, EDOM, EDOM, EDOM };

  vx = spu_promote(x, 0);
  res = spu_extract(_tgammaf4(vx), 0);
#ifndef __IEEE_LIBM
  /*
   * use vector truncf4 rather than splat x, and splat truncx.
   */
  truncx = _truncf4(vx);
  cmpres = spu_cmpeq(truncx, vx);
  verrno = spu_splats(errno);
  ferrno = spu_sel(verrno, fail, (vector unsigned int) cmpres);
  cmpres = spu_cmpgt(vc, vx);
  errno = spu_extract(spu_sel(verrno, ferrno, (vector unsigned int) cmpres), 0);
#endif
  return res;
}
