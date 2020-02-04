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
#include "headers/acosd2.h"
#include "headers/dom_chkd_negone_one.h"

static __inline double _acos(double x)
{
  double res;
  vector double vx;

  vx = spu_splats(x);
  res = spu_extract(_acosd2(vx), 0);
#ifndef _IEEE_LIBM
  /*
   * Domain error if not in the interval [-1, +1]
   */
  dom_chkd_negone_one(vx);
#endif
  return res;
}
