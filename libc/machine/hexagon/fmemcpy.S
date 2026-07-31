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
  void *fmemcpy_asm(void *dst, const void *src, size_t len)

  Forward memory copy helper called by memmove when the copy direction is
  safe to go low-to-high (src > dst, or regions do not overlap).

  This is structurally identical to memcpy with one deliberate difference:
  dczeroa ("allocate cache line without fetch") is omitted.  dczeroa is
  safe in memcpy because the destination is guaranteed not to overlap the
  source, so pre-zeroing a destination cache line cannot destroy source
  data that has not yet been read.  memmove cannot make that guarantee for
  adjacent regions, so fmemcpy_asm uses plain stores instead.

  All other aspects — the prolog/kernel/epilog structure, the valignb
  sliding window, the over-read guard, the prefetch burst, and the fast
  paths — are identical to memcpy.  See memcpy.S for the full explanation
  of each section.

  Differences from memcpy.S
  -------------------------
  1. No dczeroa calls anywhere.
  2. Uses `endstop` (R21) instead of `stop` as the over-read guard name
     (same register, different alias name).
  3. Uses `c31` (R17) as a temporary to hold the constant 31 during the
     dalign masking step (memcpy inlines this differently).
  4. The kernel loop variant for dalign 25-31 omits the dczeroa and the
     ptr_out_p_32 advance that memcpy uses to pre-allocate output lines.
---------------------------------------------------------------------------*/

	.text
	.global fmemcpy_asm
	.type fmemcpy_asm, @function
fmemcpy_asm:

/*---------------------------------------------------------------------------
  Register aliases  (identical to memcpy except endstop replaces stop,
  and c31 is an extra temporary not present in memcpy)

  C ABI arguments / return value:
    R0  ptr_out    destination pointer (returned unchanged)
    R1  ptr_in     source pointer
    R2  len        byte count

  Sliding source window:
    R13:12  data70   older 8-byte half of the source window
    R11:10  dataF8   newer 8-byte half of the source window
    valignb(dataF8, data70, offset) extracts 8 bytes at byte `offset`.

  Output staging:
    R7:6   ldata0   even 8-byte output chunk
    R19:18 ldata1   odd  8-byte output chunk (25-31 kernel variant only)
    R6     data0    low word of ldata0 (sub-word stores)
    R7     data1    high word of ldata0

  Predicates (shared physical registers, never simultaneously live):
    p0  ifbyte / ifhword / ifword / noprolog / noepilog / kernel1
    p1  nokernel
    p2  align

  Scalar temporaries:
    R19  dalign      (dst - src) & 31
    R16  star3       sub-dword byte count in prolog (prolog & 7)
    R17  c31         constant 31, used to mask dalign when kernel > 0
    R8   rest / mask / shift   (reused across phases)
    R7   back        scratch
    R3   epilog      bytes after last full 32-byte chunk
    R15:14 inc       packed increment: R15=+32 (prefetch), R14=-1 (kernel)
    R4   kernel      number of 32-byte chunks
    R5   ptr_in_p_128 / shift2   prefetch pointer (setup), epilog shift (epilog)
    R5:4 ptr_in_p_128kernel      paired prefetch ptr + kernel count for vaddw
    R15  prolog / epilogdws      dword count (setup), epilog dword count (epilog)
    R14  shiftb / align888       sub-word shift amount (prolog/epilog), alignment probe
    R9   offset / len8           src byte misalignment (setup), dword count (fast path)
    R20  over        (src+len) & 7 — bytes src end extends past a dword boundary
    R21  endstop     last address at which a memd on src is safe (over-read guard)
---------------------------------------------------------------------------*/
#define ptr_out            R0
#define ptr_in             R1
#define len                R2

#define data70             R13:12
#define dataF8             R11:10
#define ldata0             R7:6
#define ldata1             R19:18
#define data1              R7
#define data0              R6

#define ifbyte             p0
#define ifhword            p0
#define ifword             p0
#define noprolog           p0
#define nokernel           p1
#define noepilog           p0
#define align              p2
#define kernel1            p0

#define dalign             R19
#define star3              R16
#define c31                R17    // constant 31, used to mask dalign
#define rest               R8
#define back               R7
#define epilog             R3
#define inc                R15:14
#define kernel             R4
#define ptr_in_p_128       R5
#define ptr_in_p_128kernel R5:4
#define mask               R8
#define shift              R8
#define shift2             R5
#define prolog             R15
#define epilogdws          R15
#define shiftb             R14
#define offset             R9
#define align888           R14
#define len8               R9
#define over               R20
#define endstop            R21    // over-read guard (same role as `stop` in memcpy)

/*===========================================================================
  ENTRY GUARDS AND FAST-PATH DISPATCH
  ====================================
  Identical to memcpy: trivial return, dword fast path, byte fast path.
  See memcpy.S for the full explanation.
===========================================================================*/

/* On arch > v82, insert a return-buffer NOP for the branch predictor. */
#if __HEXAGON_ARCH__ > 82
	nop_rb
#endif

/* Packet: p1 = (len != 0) AND (src != dst); begin alignment probe; test len > 23. */
{
        p1 = !cmp.eq(len, #0)              // p1 = (len != 0)
        p1 = !cmp.eq(ptr_in, ptr_out)      // p1 &= (src != dst)
        align888 = or(ptr_in, ptr_out)     // alignment probe: OR src and dst
        p0 = cmp.gtu(len, #23)             // p0 = (len > 23)
}
/* Packet: early return if trivial; finish alignment probe; compute dword count. */
{
        if(!p1) jumpr r31                  // return if len==0 or src==dst
        p3 = cmp.gtu(len, #95)             // p3 = (len > 95): too big for dword shortcut
        align888 = or(align888, len)       // OR in len to complete alignment probe
        len8 = lsr(len, #3)                // len8 = len / 8 (dword count)
}
/* Packet: prefetch first source line; test 8-byte alignment of src, dst, and len. */
{
	dcfetch(ptr_in)                    // non-faulting prefetch of first source line
	p2 = bitsclr(align888, #7)         // p2 = all of src, dst, len are 8-aligned
}
/* Packet: take dword fast path if all-aligned AND len <= 95. */
{
	p2 = and(p2,!p3)                   // p2 = all-aligned AND len <= 95
	if(p2.new) len = add(len, #-8)     // adjust len for dword loop entry
	if(p2.new) jump:NT .Ldwordaligned  // take dword fast path
}
/* Packet: take byte fast path if len <= 23; start loading prolog mask. */
{
	if(!p0) jump   .Lbytes23orless     // len <= 23: use byte-by-byte loop
	mask.l = #LO(0x7fffffff)           // lower 16 bits of mask constant
	prolog = sub(#0, ptr_out)          // prolog = -dst (low bits = bytes to 32B boundary)
}

/*===========================================================================
  STACK FRAME SETUP
  =================
  Save callee-saved registers r16-r21 (24 bytes).
===========================================================================*/

/* Packet: allocate frame; finish mask; set up prefetch pointer; count leading zeros. */
{
	allocframe(#24)                    // push LR; allocate 24 bytes for r16-r21
	mask.h = #HI(0x7fffffff)           // upper 16 bits: mask = 0x7FFFFFFF
	ptr_in_p_128 = add(ptr_in, #32)    // prefetch pointer starts 32 bytes ahead
	back = cl0(len)                    // count leading zeros of len (caps prolog)
}
/* Packet: save r16-r17; compute prolog size; extract source byte offset. */
{
	memd(sp+#0) = R17:16               // save r16, r17
	prolog &= lsr(mask, back)          // prolog = (-dst) & (0x7FFFFFFF >> clz(len))
	offset = and(ptr_in, #7)           // offset = src & 7: byte misalignment
}
/* Packet: save r18-r19; compute relative alignment; note if src is misaligned. */
{
	memd(sp+#8) = R19:18               // save r18, r19
	dalign = sub(ptr_out, ptr_in)      // dalign = dst - src (masked to &31 later)
	p3 = cmp.gt(offset, #0)            // p3 = src is not dword-aligned
}
/* Packet: save r20-r21; begin computing the over-read guard (endstop). */
{
	over = add(len, ptr_in)            // over = src + len (one-past-end of source)
	back = add(len, offset)            // back = bytes from aligned src base to src end
	memd(sp+#16) = R21:20              // save r20, r21
	endstop = len                      // endstop will become the over-read guard address
}

/*===========================================================================
  PROLOG SIZE CALCULATION
  =======================
  Split prolog into sub-dword bytes (star3) and full dwords (prolog >> 3).
  Align ptr_in down to its 8-byte boundary.
===========================================================================*/

/* Packet: test if prolog is dword-aligned; mask prolog to 5 bits; prefetch. */
{
	noprolog = bitsclr(prolog, #7)     // noprolog = (prolog & 7 == 0)
	prolog = and(prolog, #31)          // keep only 5 bits (0..31)
	dcfetch(ptr_in_p_128)              // prefetch 32 bytes ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance to 64B ahead
}
/* Packet: compute kernel byte count; bit-shift for sub-word extraction; align src. */
{
	kernel = sub(len, prolog)          // kernel = len - prolog
	shift = asl(prolog, #3)            // shift = prolog_bytes * 8
	star3 = and(prolog, #7)            // star3 = sub-dword byte count (0..7)
	ptr_in = and(ptr_in, #-8)          // align src down to 8-byte boundary
}
/* Packet: convert prolog to dword count; compute epilog; finish over. */
{
	prolog = lsr(prolog, #3)           // prolog = prolog / 8 (dword count)
	epilog = and(kernel, #31)          // epilog = bytes after last 32B chunk
	over = and(over, #7)               // over = (src+len) & 7
}

/*===========================================================================
  OVER-READ GUARD (endstop)
  =========================
  endstop = last address at which a memd on src is safe.
  If src end is not dword-aligned (over > 0), allow one more read.
===========================================================================*/

/* Packet: check if src end is dword-aligned; compute endstop base. */
{
	p3 = cmp.gt(over, #0)              // p3 = src end is not dword-aligned
	endstop += sub(ptr_in, over)       // endstop = ptr_in_aligned + len - over
}
/* Packet: extend endstop if src end is mid-dword; load constant 31 for dalign mask. */
{
	if(p3) endstop = add(endstop, #8)  // allow one more read if src end is mid-dword
	c31 = #31                          // c31 = 31, used below to mask dalign
}

/*===========================================================================
  PREFETCH BURST + KERNEL SIZE
  ============================
  Issue 5 dcfetch instructions (5 × 32 = 160 bytes ahead).
  Compute final kernel count and adjust prolog for sp1loop0.
  NOTE: unlike memcpy, no dczeroa is issued here.
===========================================================================*/

/* Packet: check if back > 8; compute kernel chunks; prefetch 64B ahead. */
{
	p3 = cmp.gtu(back, #8)             // p3 = need to load dataF8 at entry
	kernel = lsr(kernel, #5)           // kernel = (len - prolog) / 32
	dcfetch(ptr_in_p_128)              // prefetch 64B ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance to 96B ahead
}
/* Packet: adjust prolog for sp1loop0 off-by-one; prefetch 96B ahead. */
{
	p1 = cmp.eq(prolog, #0)
	if(!p1.new) prolog = add(prolog, #1)  // sp1loop0 suppresses first write, so +1
	dcfetch(ptr_in_p_128)              // prefetch 96B ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance to 128B ahead
}
/* Packet: set nokernel flag; prefetch 128B ahead; extract first shiftb. */
{
	nokernel = cmp.eq(kernel,#0)       // nokernel = no 32-byte chunks
	dcfetch(ptr_in_p_128)              // prefetch 128B ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance to 160B ahead
	shiftb = and(shift, #8)            // shiftb = shift & 8 (byte-store shift)
}
/* Packet: final prefetch; mask dalign if kernel > 0 (no dczeroa unlike memcpy). */
{
	dcfetch(ptr_in_p_128)              // prefetch 160B ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance to 192B ahead
	p2 = cmp.eq(kernel, #1)            // p2 = only one 32B chunk
	if(!nokernel)dalign = and(dalign, c31) // dalign = (dst-src) & 31 (only if kernel > 0)
}
/* NOTE: no dczeroa here — fmemcpy_asm omits the cache-line pre-allocation
   that memcpy performs, because memmove may call this with adjacent regions. */

/*===========================================================================
  INITIAL SOURCE LOADS
  ====================
  Load the first two 8-byte source dwords into the sliding window.
  ptr_in is already aligned to an 8-byte boundary.
===========================================================================*/

.Lskip64:
/* Packet: load first (and optionally second) source dword; skip sub-dword prolog
   if prolog is already dword-aligned. */
{
	data70 = memd(ptr_in++#16)         // load 8 bytes from aligned src; ptr_in += 16
	if(p3) dataF8 = memd(ptr_in+#8)   // if back>8: also load next 8 bytes
	if(noprolog) jump .Lnoprolog32     // skip sub-dword prolog if prolog & 7 == 0
	align = offset                     // set valignb shift = src byte misalignment
}

/*===========================================================================
  SUB-DWORD PROLOG  (0–7 bytes)
  ==============================
  Write 0–7 bytes to bring ptr_out to a dword boundary.
  Bits 3, 4, 5 of shift (= prolog_bytes*8) encode which stores are needed.
  After each store, ldata0 is right-shifted to consume those bytes.
===========================================================================*/

/* Packet: extract first 8 aligned bytes via valignb; test for byte store. */
{
	ldata0 = valignb(dataF8, data70, align) // extract 8 bytes at byte `offset` of window
	ifbyte = tstbit(shift,#3)          // bit 3 set → need a 1-byte store
	offset = add(offset, star3)        // update running source offset
}
/* Packet: conditionally store byte; shift ldata0; test for halfword. */
{
	if(ifbyte) memb(ptr_out++#1) = data0  // store byte if needed
	ldata0 = lsr(ldata0, shiftb)       // consume byte from ldata0
	shiftb = and(shift, #16)           // shiftb = shift & 16 (halfword shift)
	ifhword = tstbit(shift,#4)         // bit 4 set → need a 2-byte store
}
/* Packet: conditionally store halfword; shift ldata0; test for word; check offset overflow. */
{
	if(ifhword) memh(ptr_out++#2) = data0 // store halfword if needed
	ldata0 = lsr(ldata0, shiftb)       // consume halfword from ldata0
	ifword = tstbit(shift,#5)          // bit 5 set → need a 4-byte store
	p2 = cmp.gtu(offset, #7)           // p2 = running offset crossed a dword boundary
}
/* Packet: conditionally store word; slide window if offset crossed dword boundary. */
{
	if(ifword) memw(ptr_out++#4) = data0  // store word if needed
	if(p2) data70 = dataF8             // slide window: upper becomes lower
	if(p2) dataF8 = memd(ptr_in++#8)  // load next 8 source bytes
	align = offset                     // update valignb shift for dword prolog loop
}

/*===========================================================================
  DWORD PROLOG LOOP  (0–3 × 8-byte stores)
  ==========================================
  Write full 8-byte dwords until ptr_out reaches a 32-byte boundary.
  Software-pipelined: store of previous ldata0 paired with computation of
  next ldata0 and a guarded source load.
===========================================================================*/

.Lnoprolog32:
/* Packet: set up hardware loop for `prolog` iterations. */
{
	p3 = sp1loop0(.Ldword_loop_prolog, prolog) // zero-overhead loop
}
/* Loop body: store previous ldata0; compute next; conditionally load next source dword. */
.Ldword_loop_prolog:
{
	if(p3) memd(ptr_out++#8) = ldata0  // store 8 bytes (suppressed on first iteration)
	ldata0 = valignb(dataF8, data70, align) // extract next 8 bytes from sliding window
	p0 = cmp.ltu(ptr_in, endstop)      // p0 = safe to load more source data
}
{
	data70 = dataF8                    // slide window: upper becomes lower
	if(p0) dataF8 = memd(ptr_in++#8)  // conditionally load next 8 source bytes
}:endloop0

/*===========================================================================
  KERNEL SETUP
  ============
  Peel the last kernel iteration into the epilog path.
  Choose between two loop variants based on dalign.
===========================================================================*/

.Lkernel:
/* Packet: check if kernel > 0; peel last iteration into epilog. */
{
	p3 = cmp.gtu(kernel, #0)              // p3 = at least one 32-byte chunk
	if(p3.new) kernel = add(kernel, #-1)  // peel: last iteration handled in epilog
	if(p3.new) epilog = add(epilog, #32)  // those 32 bytes go into epilog
}
/* Packet: re-check kernel; set up packed increment; choose loop variant.
   inc = combine(#32, #-1): prefetch advance (+32) and kernel decrement (-1)
   applied simultaneously each iteration via vaddw on ptr_in_p_128kernel. */
{
	nokernel = cmp.eq(kernel, #0)         // re-check after peeling
	if(nokernel.new) jump:NT  .Lepilog    // nothing left: go to epilog
	inc = combine(#32, #-1)               // packed increment for vaddw
	p3 = cmp.gtu(dalign, #24)             // p3 = dalign > 24: use two-register pipeline
}
/* Packet: branch to odd-alignment loop if dalign > 24. */
{
	if(p3) jump  .Lodd_alignment          // dalign 25-31: ldata0/ldata1 pipeline
}
/* Packet: set up hardware loop for even-alignment variant. */
{
	loop0(.Loword_loop_25to31, kernel)    // zero-overhead loop
	kernel1 = cmp.gtu(kernel, #1)         // kernel1 = more than one iteration
	rest = kernel                          // first-iter sentinel
}

/*===========================================================================
  KERNEL LOOP — EVEN ALIGNMENT (dalign 0–24)
  ============================================
  32 bytes per iteration (4 × 8-byte stores), two-register pipeline.
  No dczeroa — unlike memcpy, we do not pre-allocate destination cache lines.
  dcfetch still issues to keep source data warm.
===========================================================================*/

     .falign                               // align loop top to fetch-packet boundary
.Loword_loop_25to31:
/* Packet: prefetch 128B ahead of src; check if first iteration. */
{
	dcfetch(ptr_in_p_128)              // non-faulting prefetch 128B ahead
	p3 = cmp.eq(kernel, rest)          // p3 = first iteration (suppress ldata1 store)
}
/* Packet: advance prefetch ptr and decrement kernel via vaddw;
   suppress ldata1 store on first iteration; compute ldata1; load next src dword. */
{
	ptr_in_p_128kernel = vaddw(ptr_in_p_128kernel, inc) // prefetch+=32, kernel-=1
	if(!p3) memd(ptr_out++#8) = ldata1 // store ldata1 (suppressed on first iteration)
	ldata1 = valignb(dataF8, data70, align) // compute ldata1 from sliding window
	data70 = memd(ptr_in++#8)          // load next 8 source bytes
}
/* Packet: store ldata0; compute next ldata0; load next src dword. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(data70, dataF8, align) // compute ldata0 (args swapped)
	dataF8 = memd(ptr_in++#8)          // load next 8 source bytes
}
/* Packet: store ldata1; compute next ldata1; load next src dword. */
{
	memd(ptr_out++#8) = ldata1         // store 8 bytes
	ldata1 = valignb(dataF8, data70, align) // compute ldata1
	data70 = memd(ptr_in++#8)          // load next 8 source bytes
}
/* Packet: store ldata0; compute next ldata0; load next src dword; update kernel1. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes (32 bytes written this iteration)
	ldata0 = valignb(data70, dataF8, align) // compute ldata0 for next iteration
	dataF8 = memd(ptr_in++#8)          // load next 8 source bytes
	kernel1 = cmp.gtu(kernel, #1)      // update: more than one iteration remaining?
}:endloop0
/* Post-loop: store the final ldata1 (peeled last iteration's first 8 bytes). */
{
	memd(ptr_out++#8) = ldata1         // store last ldata1
	jump .Lepilog
}

/*===========================================================================
  KERNEL LOOP — ODD ALIGNMENT (dalign 25–31)
  ============================================
  Same structure but single-register pipeline (ldata0 only).
  No dczeroa.
===========================================================================*/

.Lodd_alignment:
/* Packet: set up hardware loop. */
{
	loop0(.Loword_loop_00to24, kernel)    // zero-overhead loop
	kernel1 = cmp.gtu(kernel, #1)         // kernel1 = more than one iteration
	rest = add(kernel, #-1)               // first-iter sentinel
}
     .falign                               // align loop top to fetch-packet boundary
.Loword_loop_00to24:
/* Packet: prefetch 128B ahead; advance prefetch ptr and decrement kernel via vaddw. */
{
	dcfetch(ptr_in_p_128)              // non-faulting prefetch 128B ahead
	ptr_in_p_128kernel = vaddw(ptr_in_p_128kernel, inc) // prefetch+=32, kernel-=1
}
/* Four store-compute-load packets writing 32 bytes total. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(dataF8, data70, align) // compute next 8 bytes
	data70 = memd(ptr_in++#8)          // load next 8 source bytes
}
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(data70, dataF8, align) // compute next 8 bytes (args swapped)
	dataF8 = memd(ptr_in++#8)          // load next 8 source bytes
}
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(dataF8, data70, align) // compute next 8 bytes
	data70 = memd(ptr_in++#8)          // load next 8 source bytes
}
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes (32 bytes written this iteration)
	ldata0 = valignb(data70, dataF8, align) // compute ldata0 for next iteration
	dataF8 = memd(ptr_in++#8)          // load next 8 source bytes
	kernel1 = cmp.gtu(kernel, #1)      // update: more than one iteration remaining?
}:endloop0

/*===========================================================================
  EPILOG SETUP
  ============
  Split remaining bytes into full dwords (epilogdws) and sub-dword tail.
===========================================================================*/

.Lepilog:
/* Packet: check for empty epilog; split into dword count and sub-dword remainder. */
{
	noepilog = cmp.eq(epilog,#0)       // noepilog = nothing left to write
	epilogdws = lsr(epilog, #3)        // epilogdws = epilog / 8 (full dwords)
	kernel = and(epilog, #7)           // kernel (reused) = epilog & 7 (sub-dword bytes)
}
/* Packet: return early if no epilog; compute bit-shift for sub-word extraction. */
{
	if(noepilog) jump .Lmemcpy_return  // nothing left: restore regs and return
	if(noepilog) ptr_out = sub(ptr_out, len) // (dead path) restore ptr_out
	p3 = cmp.eq(epilogdws, #0)         // p3 = no full dwords in epilog
	shift2 = asl(epilog, #3)           // shift2 = epilog_bytes * 8
}
/* Packet: extract word-store shift; test for word store; skip dword loop if none. */
{
	shiftb = and(shift2, #32)          // shiftb = shift2 & 32 (word-store shift)
	ifword = tstbit(epilog,#2)         // bit 2 set → need a 4-byte store
	if(p3) jump .Lepilog60             // no full dwords: go to sub-dword epilog
}
/* Packet: set up hardware loop for epilogdws iterations. */
{
	loop0(.Ldword_loop_epilog, epilogdws) // zero-overhead loop
}

/*===========================================================================
  DWORD EPILOG LOOP  (0–3 × 8-byte stores)
  ==========================================
  Mirror of the dword prolog loop.  Guarded by endstop to prevent
  over-reading the source.
===========================================================================*/

.Ldword_loop_epilog:
/* Packet: store 8 bytes; compute next 8 bytes; check if safe to load more. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(dataF8, data70, align) // extract next 8 bytes from sliding window
	p3 = cmp.ltu(ptr_in, endstop)      // p3 = ptr_in is within safe read range
}
/* Packet: slide window; conditionally load next source dword. */
{
	data70 = dataF8                    // slide window: upper becomes lower
	if(p3) dataF8 = memd(ptr_in++#8)  // load next 8 source bytes only if safe
}:endloop0

/*===========================================================================
  SUB-DWORD EPILOG  (0–7 bytes)
  ==============================
  Write the final 0–7 bytes as word/halfword/byte stores.
  Restore ptr_out to its original value before returning.
===========================================================================*/

.Lepilog60:
/* Packet: conditionally store word; shift ldata0; test for halfword. */
{
	if(ifword) memw(ptr_out++#4) = data0  // store 4 bytes if needed
	ldata0 = lsr(ldata0, shiftb)       // consume word from ldata0
	ifhword = tstbit(epilog,#1)        // bit 1 set → need a 2-byte store
	shiftb = and(shift2, #16)          // shiftb = shift2 & 16 (halfword shift)
}
/* Packet: conditionally store halfword; shift ldata0; test for byte; adjust len. */
{
	if(ifhword) memh(ptr_out++#2) = data0 // store 2 bytes if needed
	ldata0 = lsr(ldata0, shiftb)       // consume halfword from ldata0
	ifbyte = tstbit(epilog,#0)         // bit 0 set → need a 1-byte store
	if(ifbyte.new) len = add(len, #-1) // adjust len so ptr_out - len = original dst
}
/* Packet: conditionally store byte; restore ptr_out; fall through to return. */
{
	if(ifbyte) memb(ptr_out) = data0   // store 1 byte if needed
	ptr_out = sub(ptr_out, len)        // restore ptr_out = original dst (return value)
	jump .Lmemcpy_return
}

/*===========================================================================
  FAST PATH: BYTE LOOP  (len <= 23)
  ===================================
  Software-pipelined byte-by-byte copy using a zero-overhead hardware loop.
===========================================================================*/

.Lbytes23orless:
/* Packet: set up hardware loop; pre-decrement len for ptr_out restoration. */
{
	p3 = sp1loop0(.Lbyte_copy, len)    // zero-overhead loop: len iterations
	len = add(len, #-1)                // pre-decrement for ptr_out = ptr_out - len
}
/* Loop body: load byte from src; store previous byte to dst. */
.Lbyte_copy:
{
	data0 = memb(ptr_in++#1)           // load 1 byte from src
	if(p3) memb(ptr_out++#1) = data0   // store previous byte (suppressed first iter)
}:endloop0
/* Post-loop: write last byte; restore ptr_out; return (no frame on this path). */
{
	memb(ptr_out) = data0              // write the final byte
	ptr_out = sub(ptr_out, len)        // restore ptr_out = original dst
	jumpr   r31                        // return (no frame allocated on this path)
}

/*===========================================================================
  FAST PATH: DWORD LOOP  (src, dst, len all 8-aligned, len <= 95)
  =================================================================
  No valignb needed.  Simple 8-byte store loop.
===========================================================================*/

.Ldwordaligned:
/* Packet: set up hardware loop for len/8 iterations. */
{
	p3 = sp1loop0(.Ldword_copy, len8)  // zero-overhead loop: len/8 iterations
}
/* Loop body: store previous dword; load next dword. */
.Ldword_copy:
{
	if(p3) memd(ptr_out++#8) = ldata0  // store 8 bytes (suppressed first iteration)
	ldata0 = memd(ptr_in++#8)          // load next 8 source bytes
}:endloop0
/* Post-loop: write last dword; restore ptr_out; return. */
{
	memd(ptr_out) = ldata0             // write the final dword
	ptr_out = sub(ptr_out, len)        // restore ptr_out = original dst
	jumpr r31                          // return (no frame allocated on this path)
}

/*===========================================================================
  FUNCTION RETURN  (general path)
  =================================
  Restore callee-saved registers r16-r21, then dealloc_return.
===========================================================================*/

.Lmemcpy_return:
/* Packet: restore r20-r21 and r18-r19. */
{
	r21:20 = memd(sp+#16)              // restore r20, r21
	r19:18 = memd(sp+#8)               // restore r18, r19
}
/* Packet: restore r16-r17; deallocate frame and return in one instruction. */
	r17:16 = memd(sp+#0)               // restore r16, r17
	dealloc_return                     // restore sp + r31, then jump to r31

	.size fmemcpy_asm, . - fmemcpy_asm
