/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*---------------------------------------------------------------------------
  void *memset(void *ptr, int value, size_t n)

  Fills n bytes starting at ptr with the byte value (value & 0xFF).
  Returns ptr.

  Algorithm overview
  ------------------
  The fill value is first replicated to all 8 bytes of a register pair
  using vsplatb (vector splat byte), giving splat_reg (32-bit) and
  splat_pair (64-bit).

  Three paths based on n:

    n == 0          → immediate return.

    n <= 32         → byte loop: two bytes per iteration using a
                      zero-overhead hardware loop, with an optional
                      leading single-byte store for odd n.

    n > 32          → general path:
                        1. Alignment prolog: up to 7 sub-word stores
                           (byte, halfword, word, dword, 2×dword) to
                           bring cur_ptr to a 32-byte boundary.
                        2. Cache-line loop: 32 bytes per iteration.
                           Two variants:
                             value == 0 : dczeroa only (no stores needed —
                                          dczeroa zeroes the line in cache)
                             value != 0 : dczeroa + 4 × 16-byte stores
                        3. Alignment epilog: mirrors the prolog, writing
                           the remaining 0–31 bytes as 2×dword, dword,
                           word, halfword, byte stores.

  Cache optimisation: dczeroa
  ---------------------------
  dczeroa(ptr) allocates a 32-byte cache line at ptr without reading its
  current contents from memory.  For the zero-fill case this is sufficient
  on its own — the line is allocated as all-zero.  For non-zero fills,
  dczeroa is still issued first to avoid the read-for-ownership miss, then
  the line is overwritten with splat_pair stores.

  Register aliases
  ----------------
    r0  ptr        original destination pointer (returned unchanged)
    r1  value      fill byte (only low 8 bits used)
    r2  n          byte count
    r3  cur_ptr    working pointer, advances through the buffer
    r6  splat_reg  value replicated to all 4 bytes of a 32-bit register
    r7:6 splat_pair value replicated to all 8 bytes of a 64-bit pair
    r8  loop_count number of 32-byte cache-line iterations
---------------------------------------------------------------------------*/

#define ptr               r0
#define value             r1
#define n                 r2
#define cur_ptr           r3
#define splat_reg         r6
#define splat_pair        r7:6
#define loop_count        r8

	.text
	.p2align 4
	.globl memset
	.type  memset, @function
memset:

/* On arch > v82, insert a return-buffer NOP for the branch predictor. */
#if __HEXAGON_ARCH__ > 82
	nop_rb
#endif

/*===========================================================================
  ENTRY: handle n==0, replicate fill byte, choose path
===========================================================================*/

/* Packet: return immediately if n==0; replicate value to all bytes of splat_reg;
   copy ptr to cur_ptr (working pointer).
   vsplatb(value) replicates the low byte of value to all 4 bytes of splat_reg. */
	{
		p0 = cmp.eq(n, #0)
		if (p0.new) jumpr:nt lr        // n==0: return ptr immediately
		splat_reg = vsplatb(value)     // splat_reg = value replicated to 4 bytes
		cur_ptr = ptr                  // cur_ptr is the working write pointer
	}

/*===========================================================================
  SMALL PATH: n <= 32 — byte loop
  =================================
  For small n, use a two-bytes-per-iteration zero-overhead loop.
  If n is odd, store one leading byte first, then loop over n/2 pairs.
===========================================================================*/

/* Packet: copy splat_reg to r7 to form splat_pair; test n > 32; jump to general path.
   r7 = r6 completes the 64-bit pair splat_pair = r7:6 = value in all 8 bytes. */
	{
		r7 = r6                        // complete splat_pair = value in all 8 bytes
		p0 = cmp.gtu(n, #32)           // p0 = (n > 32): use general path?
		if (p0.new) jump:t .Lskip_byte_loop // n > 32: skip to general path
        }

/* Packet: halve n for the pair loop; if n was odd, store one leading byte.
   tstbit(n, #0) tests the original n before the shift. */
        {
                n = lsr(n, #1)                 // n = n / 2 (pair count)
                p0 = tstbit(n, #0)             // p0 = original n was odd
                if (p0.new) memb(cur_ptr++#1) = value // store leading byte if n odd
        }
/* Packet: check if n/2 == 0 (n was 0 or 1); set up hardware loop. */
        {
                p0 = cmp.eq(n, #0)
                if (p0.new) jump:nt .Lbyte_end // n/2 == 0: nothing left to loop
                loop0(.Lbyte_loop, n)          // zero-overhead loop: n/2 iterations
        }
	.falign                                    // align loop top to fetch-packet boundary
/* Loop body: store two bytes per iteration using post-increment addressing.
   memb(cur_ptr+#1) stores to cur_ptr+1 without advancing cur_ptr.
   memb(cur_ptr++#2) stores to cur_ptr and advances by 2.
   Net effect: two bytes written, cur_ptr advances by 2. */
.Lbyte_loop:
	{
		memb(cur_ptr+#1) = value       // store byte at cur_ptr+1
		memb(cur_ptr++#2) = value      // store byte at cur_ptr; advance by 2
	}:endloop0
.Lbyte_end:
	{
		jumpr lr                       // return ptr (r0 unchanged)
	}

/*===========================================================================
  GENERAL PATH: n > 32
  =====================
  Bring cur_ptr to a 32-byte boundary with sub-word stores, then run the
  cache-line loop, then write the remaining tail.
===========================================================================*/

.Lskip_byte_loop:

/*===========================================================================
  ALIGNMENT PROLOG: bring cur_ptr to a 32-byte boundary
  =======================================================
  Test each alignment bit of cur_ptr in order (bit 0, 1, 2, 3, 4) and
  issue the corresponding store if the bit is set.  Each store also
  decrements n by the number of bytes written.

  bitsclr(cur_ptr, #7) is a fast check: if bits 0-2 are all clear then
  cur_ptr is already dword-aligned and we can skip the first three stores.
===========================================================================*/

/* Packet: check if cur_ptr is already dword-aligned (bits 0-2 all clear). */
	{
		p0 = bitsclr(cur_ptr, #7)      // p0 = cur_ptr is 8-byte aligned already
		if (p0.new) jump:t .Lskip_small_accesses // skip byte/hword/word stores
	}

/* Packet: conditionally store 1 byte if cur_ptr bit 0 is set; advance and decrement n. */
	{
		p0 = tstbit(cur_ptr, #0)       // p0 = cur_ptr is not 2-byte aligned
		if (p0.new) n = add(n, #-1)    // n -= 1
		if (p0.new) memb(cur_ptr++#1) = splat_reg // store byte; advance cur_ptr
	}
/* Packet: conditionally store 2 bytes if cur_ptr bit 1 is set. */
	{
		p0 = tstbit(cur_ptr, #1)       // p0 = cur_ptr is not 4-byte aligned
		if (p0.new) n = add(n, #-2)    // n -= 2
		if (p0.new) memh(cur_ptr++#2) = splat_reg // store halfword; advance cur_ptr
	}
/* Packet: conditionally store 4 bytes if cur_ptr bit 2 is set. */
	{
		p0 = tstbit(cur_ptr, #2)       // p0 = cur_ptr is not 8-byte aligned
		if (p0.new) n = add(n, #-4)    // n -= 4
		if (p0.new) memw(cur_ptr++#4) = splat_reg // store word; advance cur_ptr
	}

.Lskip_small_accesses:
/* Packet: conditionally store 8 bytes if cur_ptr bit 3 is set. */
	{
		p0 = tstbit(cur_ptr, #3)       // p0 = cur_ptr is not 16-byte aligned
		if (p0.new) n = add(n, #-8)    // n -= 8
		if (p0.new) memd(cur_ptr++#8) = splat_pair // store dword; advance cur_ptr
	}
/* Packet: conditionally store 16 bytes if cur_ptr bit 4 is set.
   Two 8-byte stores: one with post-increment #16, one at offset +8.
   Net effect: 16 bytes written, cur_ptr advances by 16. */
	{
		p0 = tstbit(cur_ptr, #4)       // p0 = cur_ptr is not 32-byte aligned
		if (p0.new) n = add(n, #-16)   // n -= 16
		if (p0.new) memd(cur_ptr++#16) = splat_pair // store 8 bytes; advance by 16
		if (p0.new) memd(cur_ptr+#8) = splat_pair   // store 8 bytes at cur_ptr+8
	}
/* cur_ptr is now 32-byte aligned. */

/*===========================================================================
  CACHE-LINE LOOP: 32 bytes per iteration
  =========================================
  If n < 32 remaining, skip the loop entirely.
  Otherwise compute loop_count = n / 32 and choose between:
    value == 0 : zero loop  (dczeroa only — the line is zeroed in cache)
    value != 0 : nonzero loop (dczeroa + 4 × 16-byte stores)
===========================================================================*/

/* Packet: check if n < 32 (skip loop); test if value is zero; compute loop count.
   cmp.gtu(r5.new, n) uses a new-value compare: r5 is loaded with #32 in the same
   packet and the comparison uses that new value immediately. */
	{
		r5 = #32
		if (cmp.gtu(r5.new, n)) jump:t .Lskip_dczero_loops // n < 32: skip loop
		p0 = cmp.eq(value, #0)         // p0 = fill value is zero
		loop_count = lsr(n, #5)        // loop_count = n / 32
	}
/* Packet: branch to zero loop if value == 0; otherwise set up nonzero loop. */
	{
		if (p0) jump .Lskip_nonzero_loop // value == 0: use dczeroa-only loop
		loop0(.Lnonzero_loop, loop_count) // zero-overhead loop for nonzero fill
	}

/*===========================================================================
  NONZERO CACHE-LINE LOOP
  ========================
  Each iteration:
    1. dczeroa(cur_ptr): allocate the 32-byte line without reading from memory
       (avoids read-for-ownership miss since we will overwrite the whole line).
    2. Four 8-byte stores to fill the line with splat_pair.
    3. n -= 32.
  The two memd stores use post-increment #16 + offset #8 to write 16 bytes
  each in two packets, covering the full 32-byte line.
===========================================================================*/

	.falign                                    // align loop top to fetch-packet boundary
.Lnonzero_loop:
	{
		dczeroa(cur_ptr)               // allocate 32-byte line without memory read
	}
	{
		memd(cur_ptr++#16) = splat_pair // store 8 bytes; advance by 16
		memd(cur_ptr+#8) = splat_pair   // store 8 bytes at cur_ptr+8
	}
	{
		memd(cur_ptr++#16) = splat_pair // store 8 bytes; advance by 16 (now at +32)
		memd(cur_ptr+#8) = splat_pair   // store 8 bytes at cur_ptr+8
		n = add(n, #-32)               // n -= 32
	}:endloop0
	{
		jump .Lskip_dczero_loops       // skip the zero loop
	}

/*===========================================================================
  ZERO CACHE-LINE LOOP
  =====================
  When value == 0, dczeroa alone is sufficient: it allocates the line as
  all-zero in the cache without any explicit store instructions.
  Each iteration: dczeroa, advance cur_ptr by 32, n -= 32.
===========================================================================*/

.Lskip_nonzero_loop:
	{
		loop0(.Lzero_loop, loop_count) // zero-overhead loop for zero fill
	}
	.falign                                    // align loop top to fetch-packet boundary
.Lzero_loop:
	{
		dczeroa(cur_ptr)               // zero-fill 32-byte line (no stores needed)
		cur_ptr = add(cur_ptr, #32)    // advance cur_ptr by 32
		n = add(n, #-32)               // n -= 32
	}:endloop0

/*===========================================================================
  ALIGNMENT EPILOG: write remaining 0–31 bytes
  ==============================================
  Mirror of the prolog: test bits 4, 3, 2, 1, 0 of n in order and issue
  the corresponding store for each set bit.  cur_ptr is still 32-byte
  aligned at this point.
===========================================================================*/

.Lskip_dczero_loops:
/* Packet: conditionally store 16 bytes if n bit 4 is set. */
       {
                p0 = tstbit(n, #4)             // p0 = 16 bytes remaining
                if (p0.new) n = add(n, #-16)   // n -= 16
                if (p0.new) memd(cur_ptr++#16) = splat_pair // store 8 bytes; advance 16
                if (p0.new) memd(cur_ptr+#8) = splat_pair   // store 8 bytes at +8
        }
/* Packet: conditionally store 8 bytes if n bit 3 is set. */
        {
                p0 = tstbit(n, #3)             // p0 = 8 bytes remaining
                if (p0.new) n = add(n, #-8)    // n -= 8
                if (p0.new) memd(cur_ptr++#8) = splat_pair // store dword; advance 8
        }
/* Packet: conditionally store 4 bytes if n bit 2 is set. */
        {
                p0 = tstbit(n, #2)             // p0 = 4 bytes remaining
                if (p0.new) n = add(n, #-4)    // n -= 4
                if (p0.new) memw(cur_ptr++#4) = splat_reg // store word; advance 4
        }
/* Packet: conditionally store 2 bytes if n bit 1 is set. */
        {
                p0 = tstbit(n, #1)             // p0 = 2 bytes remaining
                if (p0.new) n = add(n, #-2)    // n -= 2
                if (p0.new) memh(cur_ptr++#2) = splat_reg // store halfword; advance 2
        }
/* Packet: conditionally store 1 byte if n bit 0 is set; return.
   jumpr lr returns to the caller.  r0 = ptr is unchanged throughout. */
	{
		p0 = tstbit(n, #0)             // p0 = 1 byte remaining
		if (p0.new) memb(cur_ptr) = splat_reg // store final byte (no advance needed)
		jumpr lr                       // return ptr (r0 unchanged)
	}
	.size memset, . - memset
