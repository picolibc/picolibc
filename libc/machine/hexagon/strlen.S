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
  size_t strlen(const char *s)

  Returns the number of bytes in the null-terminated string s, not
  including the terminating null byte.

  Algorithm overview
  ------------------
  Two paths based on pointer alignment:

  Fast path (s is 8-byte aligned):
    Scan 8 bytes at a time using vcmpb.eq and any8.
    any8(vcmpb.eq(r5:4, r7:6)) is true if any byte in the 8-byte block
    equals zero (null found).  Loop until a null is found, then use
    vcmpb.eq + ct0 to find the exact byte position within the block.

  Slow path (s is not 8-byte aligned):
    Advance one byte at a time until the pointer is 8-byte aligned,
    checking each byte for null.  Then jump into the fast path.

  Key Hexagon instructions used
  -----------------------------
  bitsclr(r, #7)       True if bits 0-2 of r are all clear (8-byte aligned).
  vcmpb.eq(Rdd, Rss)   Vector byte compare: produces a predicate with one
                       bit per byte, set if the bytes are equal.
  any8(p)              True if any of the 8 bits of predicate p is set.
  ct0(R)               Count trailing zeros in a 32-bit register.
                       Used here on the predicate-to-register result to
                       find the position of the first set bit (= first
                       null byte).
  r4 = p0              Move predicate register to general register, so
                       ct0 can operate on it.

  Register usage
  --------------
    r0   s (input pointer, also base for indexed loads; returned as length)
    r1   scratch (current pointer in slow path)
    r2   byte offset from r0 (length accumulator)
    r4   current byte (slow path); predicate-to-register result (fast path)
    r5:4 current 8-byte block being scanned
    r7:6 zero register pair (null sentinel, all bytes 0x00)
---------------------------------------------------------------------------*/

// FUNCTION: strlen
	.text
        .p2align 5
	.globl strlen
	.type  strlen, @function
strlen:

/*===========================================================================
  ENTRY: check alignment and handle first byte
  =============================================
  Load the first byte of s unconditionally (needed for both paths).
  Test whether s is 8-byte aligned.
  If not aligned, jump to the slow path.
  r2 = 0 is the byte offset accumulator (length so far).
===========================================================================*/

/* Packet: load first byte; test 8-byte alignment; jump to slow path if not aligned;
   initialise offset r2 = 0. */
	{
		r4 = memub(r0)                 // r4 = first byte of s (unsigned)
		p1 = bitsclr(r0,#7)            // p1 = (s is 8-byte aligned)
		if (!p1.new) jump:nt .Lnot_aligned // not aligned: go to slow path
		r2 = #0                        // r2 = byte offset (length accumulator)
	}

/*===========================================================================
  FAST PATH ENTRY: s is 8-byte aligned
  ======================================
  Check if the very first byte is null (empty string).
  If not, load the first 8-byte block and enter the scan loop.
===========================================================================*/

/* Packet: check if first byte is null. */
	{
		p0 = cmp.eq(r4,#0)             // p0 = first byte is null
		if (p0.new) jump:nt .Lzero     // empty string: return 0
	}
/* Packet: load first 8-byte block (guarded: only if first byte was not null);
   set up null sentinel r7:6 = 0; advance offset by 8.
   r0+r2<<#0 is an indexed load: address = r0 + (r2 << 0) = r0 + r2. */
	// COMBINE THESE PACKETS AFTER BRANCH/JUMP MERGING
	{
		if (!p0) r5:4 = memd(r0+r2<<#0) // load 8 bytes at r0+r2
		r7:6 = #0                       // null sentinel: all bytes 0x00
		if (!p0) r2 = add(r2,#8)        // advance offset by 8
	}

/*===========================================================================
  8-BYTE SCAN LOOP
  =================
  Each iteration checks whether any byte in the current 8-byte block is
  null.  If not, load the next block and loop.  If yes, fall through to
  find the exact null position.

  The loop body is a single 4-slot packet combining:
    - any8(vcmpb.eq(r5:4, r7:6)): null check
    - conditional load of next block
    - conditional branch back
    - conditional offset advance
  This is a zero-branch-overhead loop on v65+.
===========================================================================*/

	.falign                            // align loop top to fetch-packet boundary
1:
/* Packet: check for null; if none, load next block, advance offset, and loop. */
	{
		p0 = any8(vcmpb.eq(r5:4,r7:6))  // p0 = null byte found in current block
		if (!p0.new) r5:4 = memd(r0+r2<<#0) // no null: load next 8 bytes
		if (!p0.new) jump:t 1b           // no null: loop back
		if (!p0.new) r2 = add(r2,#8)    // no null: advance offset
	}

/*===========================================================================
  NULL FOUND: compute exact position
  ====================================
  r5:4 contains the 8-byte block with the null byte.
  r2 was advanced past this block, so we subtract 8 to get the base offset.

  Steps:
    1. vcmpb.eq(r5:4, r7:6) → p0: one bit set per null byte in the block.
    2. r4 = p0: move predicate to register (so ct0 can operate on it).
    3. ct0(r4): count trailing zeros = bit index of first set bit
                = byte index of first null within the block.
    4. r0 = (r2 - 8) + r4 = total length.
===========================================================================*/

/* Standalone instruction: find which bytes in the block are null. */
	p0 = vcmpb.eq(r5:4,r7:6)          // p0: bit set for each null byte
/* Standalone instruction: move predicate to register for ct0. */
	r4 = p0                            // r4 = predicate as 8-bit integer
/* Packet: find first null byte index; adjust r2 back by 8. */
	{
		r4 = ct0(r4)                   // r4 = bit index of first null byte (0..7)
		r2 = add(r2,#-8)               // undo the last +8 (r2 = base of this block)
	}
/* Packet: compute total length = block base + null byte index; return. */
	{
		r0 = add(r2,r4)                // r0 = length = offset + null position
		jumpr r31
	}

/*===========================================================================
  EMPTY STRING: return 0
===========================================================================*/

	.falign
.Lzero:
	{
		r0 = #0
		jumpr r31
	}

/*===========================================================================
  SLOW PATH: s is not 8-byte aligned
  =====================================
  Advance one byte at a time until the pointer is 8-byte aligned, checking
  each byte for null.  Then load the first aligned 8-byte block and jump
  into the fast scan loop.

  r1 = r0 + r2 is the current byte pointer (r0 is the base, r2 the offset).
  r4 holds the current byte (loaded in the entry packet above).
===========================================================================*/

	.falign
.Lnot_aligned:
/* Packet: compute current pointer r1 = r0 + r2; check if current byte is null. */
	{
		r1 = add(r0,r2)                // r1 = current byte pointer
		p1 = cmp.eq(r4,#0)             // p1 = current byte is null
		if (p1.new) jump:nt .Lna_found_zero // null found: return r2 as length
	}
/* Packet: check if r1 is now 8-byte aligned; if not, load next byte and increment.
   bitsclr(r1, #7): true if r1 is 8-aligned (time to switch to fast path). */
	{
		p0 = bitsclr(r1,#7)            // p0 = r1 is now 8-byte aligned
		if (!p0.new) jump:nt .Lnot_aligned // not yet aligned: loop
		if (!p0.new) r4 = memub(r1+#1) // load next byte (at r1+1)
		if (!p0.new) r2 = add(r2,#1)   // advance offset
	}
/* r1 is now 8-byte aligned.  Load the first aligned 8-byte block and
   jump into the fast scan loop.  r2 is advanced by 8 to account for
   the block we are about to scan. */
	{
		r5:4 = memd(r0+r2<<#0)         // load first aligned 8-byte block
		r7:6 = #0                      // null sentinel
		r2 = add(r2,#8)                // advance offset by 8
		jump 1b                        // jump into fast scan loop
	}

/*===========================================================================
  NULL FOUND IN SLOW PATH: return current offset as length
===========================================================================*/

	.falign
.Lna_found_zero:
	{
		r0 = r2                        // length = current byte offset
		jumpr r31
	}
	.size strlen, . - strlen
