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
  int strcmp(const char *s1, const char *s2)

  Compares two null-terminated strings byte by byte.
  Returns:
    < 0  if s1 < s2
      0  if s1 == s2
    > 0  if s1 > s2

  Algorithm overview
  ------------------
  Two paths are selected based on pointer alignment:

  Fast path (both s1 and s2 are 8-byte aligned):
    Compare 8 bytes at a time using Hexagon vector byte instructions.
    The loop is unrolled 3× to reduce branch overhead.
    On each group of 3 iterations, if no null byte is found in s2 and
    s1 == s2 so far, load the next 8 bytes and continue.
    When a difference or null is detected, find the first differing byte
    using ct0/ct1 (count trailing zeros/ones) on predicate masks, then
    extract and subtract the differing bytes.

  Slow path (either pointer is not 8-byte aligned):
    Compare one byte at a time until a difference or null is found.

  Key Hexagon instructions used
  -----------------------------
  bitsclr(r, #7)      True if bits 0-2 of r are all clear (8-byte aligned).
  vcmpb.eq(Rdd, Rss)  Vector byte compare: produces a predicate with one bit
                      per byte, set if the bytes are equal.
  any8(p)             True if any of the 8 bits of predicate p is set.
  mask(p)             Expand predicate p: each set bit becomes 0xFF in the
                      corresponding byte of the result register pair.
  ct0(Rdd)            Count trailing zeros in a 64-bit register pair.
  ct1(Rdd)            Count trailing ones in a 64-bit register pair.
  extractu(Rdd,Rss)   Unsigned bit-field extract: extracts a field of width
                      r[12:8] starting at bit r[4:0] from Rdd.

  Architecture variants
  ---------------------
  Three implementations are selected by preprocessor conditionals:

  __HEXAGON_PHYSICAL_SLOTS__ == 3  (3-slot VLIW, e.g. v65 with 3 slots)
    The 8-byte loop uses a slightly different packet structure because the
    3-slot machine cannot issue as many operations in parallel.  The load
    of the next s1 block is split into a separate packet.

  __HEXAGON_ARCH__ >= 65  (v65 and later, 4-slot)
    The 8-byte loop uses a single 4-slot packet per iteration that combines
    the null check, equality check, conditional load of next s2, and
    conditional branch.

  else  (older architectures)
    Uses any8/vmux instead of !any8/mask because older cores lack some of
    the newer predicate instructions.  The loop structure is otherwise the
    same.

  Register usage (all three variants)
  ------------------------------------
    r0   s1 pointer (advances through s1; also return value)
    r1   s2 pointer (advances through s2)
    r3:2 current 8-byte block of s1
    r5:4 current 8-byte block of s2
    r7:6 zero register pair (all bytes 0x00), used as null sentinel
    r9:8 scratch for mask/extract operations
    r4   scratch: saved s2 pointer (slow path), or extract result
    r5   scratch: saved s1 pointer (slow path), or extract result
    r2   current byte of s1 (slow path)
    r3   current byte of s2 (slow path)
---------------------------------------------------------------------------*/

// FUNCTION: strcmp
#if  __HEXAGON_PHYSICAL_SLOTS__ == 3

/*===========================================================================
  VARIANT 1: 3-slot VLIW machine
===========================================================================*/

	.text
	.p2align 5
	.globl strcmp
	.type  strcmp, @function
strcmp:

/*===========================================================================
  ALIGNMENT CHECK AND FAST-PATH ENTRY
  =====================================
  Test whether both s1 (r0) and s2 (r1) are 8-byte aligned.
  If so, load the first 8 bytes of s2 speculatively and enter the fast loop.
  If not, fall through to the byte-by-byte slow path.
===========================================================================*/

/* Packet: test alignment of both pointers; speculatively load first 8 bytes of s2.
   Both bitsclr writes go to p0: the second AND's with the first, so p0 is true
   only if both r0 and r1 have bits 0-2 clear (both 8-aligned). */
	{
		p0 = bitsclr(r0,#7)            // p0 = (s1 is 8-aligned)
		p0 = bitsclr(r1,#7)            // p0 &= (s2 is 8-aligned)
		if (p0.new) r5:4 = memd(r1++#8) // if aligned: load first 8 bytes of s2
	}
/* Packet: if aligned, load first 8 bytes of s1; if not, jump to byte loop.
   r7:6 = 0 is the null sentinel used in vcmpb.eq comparisons. */
	{
		if (p0) r3:2 = memd(r0++#8)    // load first 8 bytes of s1
		if (!p0) jump 3f               // not aligned: go to byte-by-byte loop
		r7:6 = #0                      // null sentinel: all bytes 0x00
	}

/*===========================================================================
  8-BYTE COMPARISON LOOP (3-slot variant, unrolled 3×)
  ======================================================
  Each group of 3 iterations checks 8 bytes of each string.
  p0 is true when we should continue looping (no null found, no difference).

  !any8(vcmpb.eq(r5:4, r7:6)):  true if NO byte of s2 is zero (no null yet)
  cmp.eq(r3:2, r5:4):           true if the 8-byte blocks are equal

  Both conditions must hold to continue.  The two p0 assignments in the
  same packet implement a logical AND.

  On the 3-slot machine, the load of the next s1 block cannot be combined
  with the branch in the same packet, so it is split into a separate packet.
===========================================================================*/

	//.falign
//  unroll loop 3x:
0:
/* 1st iteration: check null in s2 AND equality; if continuing, load next s2 block. */
	{
		p0 = !any8(vcmpb.eq(r5:4,r7:6)) // p0 = no null byte in s2 block
		p0 = cmp.eq(r3:2,r5:4)          // p0 &= s1 block == s2 block
		if (p0.new) r5:4 = memd(r1++#8) // if continuing: load next 8 bytes of s2
	}
	{
		if (!p0) jump:t 1f             // difference or null found: go to result
		if (p0) r3:2 = memd(r0)        // if continuing: load next 8 bytes of s1
		if (p0) r0 = add(r0,#8)        // advance s1 pointer
	}
/* 2nd iteration: same structure. */
	{
		p0 = !any8(vcmpb.eq(r5:4,r7:6))
		p0 = cmp.eq(r3:2,r5:4)
		if (p0.new) r5:4 = memd(r1++#8)
	}
	{
		if (!p0) jump:t 1f
		if (p0) r3:2 = memd(r0)
		if (p0) r0 = add(r0,#8)
	}
/* 3rd iteration: if still equal and no null, jump back to top of loop (label 2). */
	{
		p0 = !any8(vcmpb.eq(r5:4,r7:6))
		p0 = cmp.eq(r3:2,r5:4)
		if (p0.new) jump:nt 2f         // continue loop: load next s1 block at label 2
	}

/*===========================================================================
  RESULT COMPUTATION
  ===================
  A difference or null byte was found in the current 8-byte blocks r3:2
  (s1) and r5:4 (s2).  Find the first byte position where they differ or
  where s1 has a null, and return s1[i] - s2[i].

  Steps:
    1. vcmpb.eq(r3:2, #0)  → p0: bitmask of null bytes in s1
       vcmpb.eq(r3:2, r5:4) → p1: bitmask of equal bytes
    2. mask(p0) → r7:6: 0xFF in each byte position that is null in s1
       mask(p1) → r9:8: 0xFF in each byte position where s1 == s2
    3. ct1(r9:8) → r8: index of first differing byte (count trailing 0xFF)
       ct0(r7:6) → r6: index of first null byte (count trailing 0x00)
    4. If the first difference comes before the first null (r8 > r6 is false),
       extract and subtract the bytes at that position.
       If the first null comes first, return 0 (strings are equal up to null).

  extractu(Rdd, r9:8) extracts a field of width r9 (=8 bits) starting at
  bit position r8 from the 64-bit pair Rdd, giving the byte value.
===========================================================================*/

1:
/* Packet: find null positions in s1; find equal positions between s1 and s2. */
	{
		p0 = vcmpb.eq(r3:2,#0)         // p0: bit set for each null byte in s1
		p1 = vcmpb.eq(r3:2,r5:4)       // p1: bit set for each equal byte
	}
/* Packet: expand predicates to byte masks; clear return value. */
	{
		r7:6 = mask(p0)                // r7:6: 0xFF at each null position
		r9:8 = mask(p1)                // r9:8: 0xFF at each equal position
		r0 = #0                        // default return value = 0 (equal)
	}
/* Packet: find index of first difference (ct1 on equal mask = trailing 0xFF count). */
	{
		r8 = ct1(r9:8)                 // r8 = bit index of first differing byte
		r9 = #8                        // r9 = extract width (8 bits = 1 byte)
	}
/* Packet: find index of first null (ct0 on null mask = trailing 0x00 count). */
	{
		r6 = ct0(r7:6)                 // r6 = bit index of first null byte
	}
/* Packet: extract the differing bytes from s1 and s2 at position r8. */
	{
		r3:2 = extractu(r3:2,r9:8)     // r2 = s1 byte at first difference
		p0 = cmp.gtu(r8,r6)            // p0 = difference comes before null?
	}
	{
		r5:4 = extractu(r5:4,r9:8)     // r4 = s2 byte at first difference
	}
/* Packet: if difference before null, return s1[i] - s2[i]; otherwise return 0. */
	{
		if (!p0) r0 = sub(r2,r4)       // return diff if difference before null
		jumpr r31
	}

/* Load next s1 block and loop back to top. */
2:
	{
		r3:2 = memd(r0++#8)            // load next 8 bytes of s1; advance s1
	}
	{
		r5:4 = memd(r1++#8)            // load next 8 bytes of s2; advance s2
		jump 0b                        // loop back
	}

/*===========================================================================
  BYTE-BY-BYTE SLOW PATH
  =======================
  Used when either pointer is not 8-byte aligned.
  r4 and r5 are used as the advancing pointers (r0 and r1 are needed for
  the return value and s2 pointer respectively).
===========================================================================*/

	//.falign
3:
/* Packet: save advancing pointers in r4, r5; load first bytes. */
	{
		r4 = add(r0,#1)                // r4 = s1 + 1 (advancing pointer)
		r5 = add(r1,#1)                // r5 = s2 + 1 (advancing pointer)
		r2 = memub(r0)                 // r2 = first byte of s1 (unsigned)
	}
	{
		r3 = memub(r1)                 // r3 = first byte of s2 (unsigned)
	}
/* Loop: compare one byte at a time. */
4:
/* Packet: check for null in s1; check equality; compute s1[i] - s2[i] speculatively. */
	{
		p0 = cmp.eq(r2,#0)             // p0 = s1 byte is null (end of string)
		p1 = cmp.eq(r2,r3)             // p1 = bytes are equal
		r0 = sub(r2,r3)                // r0 = s1[i] - s2[i] (return value if different)
	}
/* Packet: p0 = (null found OR bytes differ); if still equal, load next s1 byte. */
	{
		p0 = or(p0,!p1)                // p0 = stop condition: null or difference
		if (!p0.new) r2 = memub(r4++#1) // load next s1 byte if continuing
	}
/* Packet: load next s2 byte if continuing; loop back if continuing. */
	{
		if (!p0) r3 = memub(r5++#1)    // load next s2 byte if continuing
		if (!p0) jump:t 4b             // loop back if no stop condition
	}
	{
		jumpr r31                      // return r0 = s1[i] - s2[i] (or 0)
	}

	.size strcmp, . - strcmp

#elif __HEXAGON_ARCH__ >= 65

/*===========================================================================
  VARIANT 2: v65 and later (4-slot VLIW)
  ========================================
  Same algorithm as variant 1 but the 8-byte loop is more compact: the
  null check, equality check, conditional s2 load, and conditional branch
  all fit in a single 4-slot packet per iteration.
===========================================================================*/

	.text
	.p2align 5
	.globl strcmp
	.type  strcmp, @function
strcmp:

/* Packet: test alignment; speculatively load first 8 bytes of s2. */
	{
		p0 = bitsclr(r0,#7)            // p0 = (s1 is 8-aligned)
		p0 = bitsclr(r1,#7)            // p0 &= (s2 is 8-aligned)
		if (p0.new) r5:4 = memd(r1++#8) // if aligned: load first 8 bytes of s2
	}
/* Packet: if aligned, load first 8 bytes of s1 and advance s1; else go to byte loop. */
	{
		if (p0) r3:2 = memd(r0)        // load first 8 bytes of s1
		if (p0) r0 = add(r0,#8)        // advance s1 pointer
		if (!p0) jump 3f               // not aligned: byte-by-byte loop
		r7:6 = #0                      // null sentinel
	}

/*===========================================================================
  8-BYTE COMPARISON LOOP (v65+ variant, unrolled 3×)
  ====================================================
  On v65+, the 4-slot machine can combine the null check, equality check,
  conditional branch, and conditional s2 load into a single packet.
  p0 = !any8(null in s2) AND (s1 == s2): continue if true.
  If !p0 (stop condition), jump to result computation at label 1.
  If p0 (continue), load next s2 block speculatively.
===========================================================================*/

	.falign
0:
/* 1st iteration: single packet combines all checks and the conditional load. */
	{
		p0 = !any8(vcmpb.eq(r5:4,r7:6)) // p0 = no null in s2 block
		p0 = cmp.eq(r3:2,r5:4)          // p0 &= s1 == s2
		if (!p0.new) jump:t 1f           // stop: difference or null found
		if (p0.new) r5:4 = memd(r1++#8) // continue: load next 8 bytes of s2
	}
/* Load next s1 block (separate packet on v65+ to avoid slot conflict). */
	{
		r3:2 = memd(r0)                // load next 8 bytes of s1
		r0 = add(r0,#8)                // advance s1 pointer
	}
/* 2nd iteration. */
	{
		p0 = !any8(vcmpb.eq(r5:4,r7:6))
		p0 = cmp.eq(r3:2,r5:4)
		if (!p0.new) jump:t 1f
		if (p0.new) r5:4 = memd(r1++#8)
	}
	{
		r3:2 = memd(r0)
		r0 = add(r0,#8)
	}
/* 3rd iteration: if still equal and no null, jump back to top (label 2). */
	{
		p0 = !any8(vcmpb.eq(r5:4,r7:6))
		p0 = cmp.eq(r3:2,r5:4)
		if (p0.new) jump:nt 2f         // continue: load next s1 at label 2
	}

/* Result computation: identical to variant 1. */
1:
	{
		p0 = vcmpb.eq(r3:2,#0)         // null positions in s1
		p1 = vcmpb.eq(r3:2,r5:4)       // equal positions
	}
	{
		r7:6 = mask(p0)                // byte mask of null positions
		r9:8 = mask(p1)                // byte mask of equal positions
		r0 = #0
	}
	{
		r6 = ct0(r7:6)                 // index of first null byte
		r8 = ct1(r9:8)                 // index of first differing byte
		r9 = #8                        // extract width
	}
	{
		r3:2 = extractu(r3:2,r9:8)     // s1 byte at first difference
		r5:4 = extractu(r5:4,r9:8)     // s2 byte at first difference
		p0 = cmp.gtu(r8,r6)            // difference before null?
	}
	{
		if (!p0) r0 = sub(r2,r4)       // return diff if difference before null
		jumpr r31
	}

/* Load next s1 block and loop. */
2:
	{
		r3:2 = memd(r0++#8)            // load next 8 bytes of s1; advance s1
		r5:4 = memd(r1++#8)            // load next 8 bytes of s2; advance s2
		jump 0b
	}

/* Byte-by-byte slow path: identical to variant 1. */
	.falign
3:
	{
		r2 = memub(r0)                 // first byte of s1
		r3 = memub(r1)                 // first byte of s2
		r4 = add(r0,#1)                // advancing s1 pointer
		r5 = add(r1,#1)                // advancing s2 pointer
	}
	.falign
4:
	{
		p0 = cmp.eq(r2,#0)             // null in s1?
		p1 = cmp.eq(r2,r3)             // bytes equal?
		r0 = sub(r2,r3)                // speculative return value
	}
	{
		p0 = or(p0,!p1)                // stop if null or difference
		if (!p0.new) jump:t 4b         // loop if continuing
		if (!p0.new) r2 = memub(r4++#1) // load next s1 byte
		if (!p0.new) r3 = memub(r5++#1) // load next s2 byte
	}
	{
		jumpr r31
	}

	.size strcmp, . - strcmp

#else

/*===========================================================================
  VARIANT 3: older architectures (pre-v65)
  ==========================================
  Uses any8 (not !any8) and vmux instead of mask, because older cores lack
  the !any8 and mask instructions.  The logic is inverted accordingly:
    p0 = any8(null in s2) OR (s1 != s2): stop condition (true = stop)
  vmux(p, a, b) selects a where p is set, b where p is clear.
  Used here to build the null/diff masks without the mask instruction.
===========================================================================*/

	.text
	.p2align 5
	.globl strcmp
	.type  strcmp, @function
strcmp:

/* Packet: test alignment; speculatively load first 8 bytes of both strings. */
	{
		p0 = bitsclr(r0,#7)            // p0 = (s1 is 8-aligned)
		p0 = bitsclr(r1,#7)            // p0 &= (s2 is 8-aligned)
		if (p0.new) r5:4 = memd(r1++#8) // load first 8 bytes of s2
		if (p0.new) r3:2 = memd(r0++#8) // load first 8 bytes of s1
	}
/* Packet: if not aligned, go to byte loop; set up null sentinel and all-ones mask.
   r9:8 = -1 (all 0xFF) is used as the "set" value for vmux. */
	{
		if (!p0) jump 3f               // not aligned: byte-by-byte loop
		r7:6 = #0                      // null sentinel (all 0x00)
		r9:8 = #-1                     // all-ones mask for vmux
	}

/*===========================================================================
  8-BYTE COMPARISON LOOP (pre-v65 variant, unrolled 3×)
  =======================================================
  p0 = any8(null in s2) OR (s1 != s2): stop condition.
  Two-packet structure per iteration: compute p0, then branch or load.
===========================================================================*/

	.falign
0:
/* 1st iteration: compute stop condition. */
	{
		p0 = any8(vcmpb.eq(r5:4,r7:6)) // p0 = null found in s2 block
		p1 = cmp.eq(r3:2,r5:4)         // p1 = s1 block == s2 block
	}
	{
		p0 = or(p0,!p1)                // p0 = stop: null found OR blocks differ
		if (p0.new) jump:t 1f          // stop condition met: go to result
		if (!p0.new) r5:4 = memd(r1++#8) // continue: load next s2 block
		if (!p0.new) r3:2 = memd(r0++#8) // continue: load next s1 block
	}
/* 2nd iteration. */
	{
		p0 = any8(vcmpb.eq(r5:4,r7:6))
		p1 = cmp.eq(r3:2,r5:4)
	}
	{
		p0 = or(p0,!p1)
		if (p0.new) jump:t 1f
		if (!p0.new) r5:4 = memd(r1++#8)
		if (!p0.new) r3:2 = memd(r0++#8)
	}
/* 3rd iteration: if no stop, jump back to top (label 2). */
	{
		p0 = any8(vcmpb.eq(r5:4,r7:6))
		p1 = cmp.eq(r3:2,r5:4)
	}
	{
		p0 = or(p0,!p1)
		if (!p0.new) jump:nt 2f        // no stop: loop back
	}

/*===========================================================================
  RESULT COMPUTATION (pre-v65 variant)
  ======================================
  Uses vmux instead of mask to build the null/diff byte masks.
  vmux(p, a, b): selects a[byte] where p[bit] is set, b[byte] where clear.
  vmux(p0, r9:8, r7:6): 0xFF where null, 0x00 elsewhere → null mask
  vmux(p1, r9:8, r7:6): 0xFF where equal, 0x00 elsewhere → equal mask
===========================================================================*/

1:
/* Packet: find null and equal positions; clear return value. */
	{
		p0 = vcmpb.eq(r3:2,#0)         // p0: null positions in s1
		p1 = vcmpb.eq(r3:2,r5:4)       // p1: equal positions
		r0 = #0
	}
/* Packet: build byte masks using vmux. */
	{
		r7:6 = vmux(p0,r9:8,r7:6)      // r7:6: 0xFF at null positions
		r9:8 = vmux(p1,r9:8,r7:6)      // r9:8: 0xFF at equal positions
	}
/* Packet: find first null and first difference indices. */
	{
		r6 = ct0(r7:6)                 // index of first null byte
		r8 = ct1(r9:8)                 // index of first differing byte
		r9 = #8                        // extract width
	}
/* Packet: extract bytes at first difference position. */
	{
		r3:2 = extractu(r3:2,r9:8)     // s1 byte at first difference
		r5:4 = extractu(r5:4,r9:8)     // s2 byte at first difference
		p0 = cmp.gtu(r8,r6)            // difference before null?
	}
/* Packet: return diff if difference before null; else return 0. */
	{
		if (!p0) r0 = sub(r2,r4)
		jumpr r31
	}

/* Load next blocks and loop. */
2:
	{
		r3:2 = memd(r0++#8)
		r5:4 = memd(r1++#8)
		jump 0b
	}

/* Byte-by-byte slow path: identical to other variants. */
	.falign
3:
	{
		r2 = memub(r0)
		r3 = memub(r1)
		r4 = add(r0,#1)
		r5 = add(r1,#1)
	}
	.falign
4:
	{
		p0 = cmp.eq(r2,#0)
		p1 = cmp.eq(r2,r3)
		r0 = sub(r2,r3)
	}
	{
		p0 = or(p0,!p1)
		if (!p0.new) jump:t 4b
		if (!p0.new) r2 = memub(r4++#1)
		if (!p0.new) r3 = memub(r5++#1)
	}
	{
		jumpr r31
	}

	.size strcmp, . - strcmp
#endif
