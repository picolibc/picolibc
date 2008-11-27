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

Author: Ken Werner <ken.werner@de.ibm.com>
*/

/* _mcount extracts the address of the function just entered and the address
    of the caller of that function and then calls __mcount_internal. The
    prologue calls mcount without saving any registers. The return address is
    stored in $75. The _mcount function has to:
     - create a new stack frame
     - save registers $2 to $75 on the stack
     - copy the two addresses ($0 and $75) into the argument registers $3 and $4
     - call __mcount_internal
     - restore registers
     - return to $75  */

/* The following two convenience macros assist in the coding of the
   saving and restoring the register.

   saveregs     first, last    Saves registers from first to the last.
   restoreregs  first, last    Restores registers from last down to first.

   Note:       first must be less than or equal to last.  */
.macro  saveregs        first, last
        stqd            $\first, \first*16($SP)
.if     \last-\first
        saveregs        "(\first+1)",\last
.endif
.endm

.macro  restoreregs     first, last
        lqd             $\last, \last*16($SP)
.if     \last-\first
        restoreregs     \first,"(\last-1)"
.endif
.endm

/* _mcount needs to be resident since the overlay manager uses the scratch
   registers too.  */
.text
  .align 3 /* 8 byte alignment.  */
  .global _mcount
  .type _mcount, @function

_mcount:
  stqd $lr, 16($sp)    /* Save link register in the callers stack frame.  */
  stqd $lr, -1216($sp) /* Store back pointer.  */
  il   $lr, -1216      /* Push a new stack frame.  */
  a    $sp, $sp, $lr   /* Frame size: 16 * (74 + 2) = 1216.  */

  /* Save registers $2 to $75 on the stack.  */
  saveregs 2, 75

  /* Bring the __mcount_internal arguments in place.  */
  lqd $3, 1232($sp) /* frompc (the link register).  */
  ori $4, $75, 0    /* selfpc (the gcc prologue puts "brsl $75, _mcount" in
                       front of every function).  */
  brsl  $lr, __mcount_internal

  /* Restore register $2 to $75 from the stack.  */
  restoreregs 2, 75

  il   $lr, 1216
  a    $sp, $sp, $lr   /* Pop the stack frame.  */
  lqd  $lr, 16($sp)    /* Restore link register.  */
  bi   $75             /* Branch to the called function.  */
