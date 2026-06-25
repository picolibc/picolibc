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
  void *rmemcpy_asm(void *dst, const void *src, size_t len)

  Reverse memory copy: copies len bytes from src to dst, working from
  high addresses to low addresses.  Used by memmove when src < dst and
  the regions overlap, to avoid overwriting source bytes before they are
  read.

  Caller contract
  ---------------
  This function may read up to 15 bytes BELOW the start of the source
  buffer (due to the aligned-read strategy).  The caller (memmove) must
  therefore call rmemcpy_asm twice when len >= 15:
    rmemcpy_asm(dst+15, src+15, len-15)  — bulk
    rmemcpy_asm(dst,    src,    15)      — first 15 bytes (known safe page)

  Relationship to memcpy / fmemcpy_asm
  -------------------------------------
  The algorithm is the mirror image of fmemcpy_asm:
  - All pointer arithmetic uses decrement (#-8) instead of increment (#8).
  - ptr_in starts at src+len (one past the end) and works backwards.
  - ptr_out starts at dst+len and works backwards.
  - The "prolog" aligns the HIGH end of dst to a 32-byte boundary.
  - The "epilog" writes the LOW end (the first bytes of the destination).
  - valignb is still used for misalignment, but the window slides in
    reverse: dataF8 is the "older" (higher-address) half, data70 is the
    "newer" (lower-address) half.
  - Sub-word stores use left-shift (lsl) to extract bytes from the high
    end of ldata0, and store without post-increment (ptr_out is
    pre-decremented before each store).
  - No dczeroa: same reason as fmemcpy_asm (regions may be adjacent).

  Structure
  ---------
    prolog  : 0–31 bytes written to bring the HIGH end of dst down to a
              32-byte boundary.  Sub-divided into sub-dword (0–7 bytes,
              stored as byte/halfword/word from the high end) and dword
              (0–3 × 8-byte stores, working downward).

    kernel  : One or more 32-byte chunks written in a tight VLIW loop,
              working downward.  Two variants based on dalign.

    epilog  : Remaining 0–31 bytes at the LOW end, mirroring the prolog.

  Fast paths (no frame):
    len == 0 or src == dst  → immediate return
    len <= 23               → reverse byte-by-byte loop
    src, dst, len all 8-aligned AND len <= 95 → reverse dword loop

  Register aliases
  ----------------
  C ABI arguments / return value:
    R0  ptr_out    destination pointer (returned unchanged — original dst)
    R1  ptr_in     source pointer
    R2  len        byte count

  Both ptr_in and ptr_out are advanced to their respective end addresses
  (ptr_in += len, ptr_out += len) before the main copy begins, then
  decremented as the copy proceeds.

  Sliding source window (reversed):
    R13:12  data70   newer (lower-address) 8-byte half of the source window
    R11:10  dataF8   older (higher-address) 8-byte half of the source window
    valignb(dataF8, data70, offset) still extracts 8 bytes at byte `offset`.

  Output staging:
    R7:6   ldata0   current 8-byte output chunk
    R19:18 ldata1   alternate 8-byte output chunk (25-31 kernel variant)
    R6     data0    low word of ldata0
    R7     data1    high word of ldata0 — used for sub-word stores in reverse
    R18    d0       scratch for extracting individual bytes/halfwords

  Predicates (shared physical registers):
    p0  ifbyte / ifhword / ifword / noprolog / noepilog
    p1  kernel0 (unused alias, kept for symmetry)
    p2  align

  Scalar temporaries:
    R19  dalign      (dst - src) & 31
    R16  star3       sub-dword byte count in prolog (prolog & 7)
    R8   rest / mask / shift   (reused across phases)
    R7   back        scratch
    R3   epilog      bytes after last full 32-byte chunk
    R15:14 inc       packed decrement: R15=-32 (prefetch), R14=-1 (kernel)
    R4   kernel      number of 32-byte chunks
    R5   ptr_in_p_128 / shift2   prefetch pointer (decrements), epilog shift
    R5:4 ptr_in_p_128kernel      paired prefetch ptr + kernel count for vaddw
    R15  prolog / epilogdws      dword count (setup), epilog dword count (epilog)
    R14  shiftb / align888       sub-word shift, alignment probe
    R9   offset / len8           src byte offset, dword count (fast path)
    R20  (unused in this file, no over variable needed — endstop not used)
---------------------------------------------------------------------------*/
        .section .text
        .global rmemcpy_asm
        .type rmemcpy_asm, @function
rmemcpy_asm:

/*---------------------------------------------------------------------------
  Register aliases
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
#define d0                 R18    // scratch for byte/halfword extraction in sub-word stores
#define ifbyte             p0
#define ifhword            p0
#define ifword             p0
#define noprolog           p0
#define noepilog           p0
#define align              p2
#define kernel0            p1
#define dalign             R19
#define rest               R8
#define star3              R16
#define back               R7
#define epilog             R3
#define inc                R15:14
#define ptr_in_p_128       R5     // prefetch pointer (decrements, not increments)
#define kernel             R4
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

/*===========================================================================
  ENTRY GUARDS AND FAST-PATH DISPATCH
  ====================================
  Same structure as fmemcpy_asm, but the trivial-return condition uses
  p1 = (len==0) OR (src==dst) rather than AND-NOT, because the two
  conditions are combined with or() in the second packet.

  Key difference from forward copy: ptr_in is advanced to src+len and
  ptr_out is advanced to dst+len before the main copy begins, so all
  subsequent pointer arithmetic uses decrement.
===========================================================================*/

/* On arch > v82, insert a return-buffer NOP for the branch predictor. */
#if __HEXAGON_ARCH__ > 82
	nop_rb;
#endif

/* Packet: compute trivial-return flags; begin alignment probe; test len > 23.
   p2 = (len==0), p1 = (src==dst) — combined with or() in next packet. */
{
        p2 = cmp.eq(len, #0)               // p2 = (len == 0)
        align888 = or(ptr_in, ptr_out)     // alignment probe: OR src and dst
        p0 = cmp.gt(len, #23)              // p0 = (len > 23)
        p1 = cmp.eq(ptr_in, ptr_out)       // p1 = (src == dst)
}
/* Packet: combine trivial-return flags; finish alignment probe; compute dword count.
   p1 = (len==0) OR (src==dst): either condition means nothing to do. */
{
        p1 = or(p2, p1)                    // p1 = trivial return condition
        p3 = cmp.gt(len, #95)              // p3 = (len > 95): too big for dword shortcut
        align888 = or(align888, len)       // OR in len to complete alignment probe
        len8 = lsr(len, #3)                // len8 = len / 8 (dword count)
}
/* Packet: advance ptr_in to src+len (end of source); compute epilog base;
   test 8-byte alignment; return immediately if trivial. */
{
        if(!p1) ptr_in = add(ptr_in, len)  // ptr_in = src + len (one past end)
        epilog = sub(#32, ptr_out)         // epilog = 32 - dst (bytes to 32B boundary from HIGH end)
        p2 = bitsclr(align888, #7)         // p2 = all of src, dst, len are 8-aligned
        if(p1) jumpr r31                   // trivial: return immediately
}
/* Packet: advance ptr_out to dst+len; take dword fast path if all-aligned AND len<=95;
   set up prefetch pointer (starts 32 bytes BELOW ptr_in). */
{
        ptr_in_p_128 = add(ptr_in, #-32)   // prefetch pointer: 32 bytes below src end
        ptr_out = add(ptr_out, len)        // ptr_out = dst + len (one past end)
        p2 = and(p2,!p3)                   // p2 = all-aligned AND len <= 95
        if(p2.new) jump:NT .Ldwordaligned  // take reverse dword fast path
}
/* Packet: prefetch first source line (below ptr_in); take byte fast path if len<=23. */
{
        dcfetch(ptr_in_p_128)              // non-faulting prefetch 32 bytes below src end
        if(!p0) jump  .Lshort_bytecpy      // len <= 23: use reverse byte loop
        mask.l = #LO(0x7fffffff)           // lower 16 bits of mask constant
}

/*===========================================================================
  STACK FRAME SETUP
  =================
  Save callee-saved registers r16-r19 (16 bytes).
  Note: only r16-r19 are saved here (not r20-r21 as in fmemcpy_asm),
  because rmemcpy_asm does not use r20/r21.
===========================================================================*/

/* Packet: allocate frame; finish mask; advance prefetch pointer further down; count leading zeros. */
{
#if __HEXAGON_ARCH__ > 82
        allocframe_rb(#16)                 // push LR; allocate 16 bytes (for r16-r19)
#else
        allocframe(#16)
#endif
        mask.h = #HI(0x7fffffff)           // upper 16 bits: mask = 0x7FFFFFFF
        ptr_in_p_128 = add(ptr_in_p_128, #-32) // advance prefetch to 64 bytes below
        back = cl0(len)                    // count leading zeros of len (caps prolog)
}
/* Packet: save r16-r17; compute epilog size (capped at len); extract source byte offset.
   offset = ptr_in & 7: the byte offset of the source END within its dword.
   This is used as the valignb shift for the reverse sliding window. */
{
        memd(sp+#0) = R17:16               // save r16, r17
        epilog &= lsr(mask, back)          // epilog = (32-dst) & (0x7FFFFFFF >> clz(len))
                                           //   → bytes from HIGH end to next 32B boundary, capped at len
        offset = and(ptr_in, #7)           // offset = (src+len) & 7: end-of-source alignment
}
/* Packet: compute kernel byte count; save r18-r19; compute relative alignment. */
{
        kernel = sub(len, epilog)          // kernel = len - epilog (bytes before the prolog)
        memd(sp+#8) = R19:18               // save r18, r19
        dalign = sub(ptr_out, ptr_in)      // dalign = (dst+len) - (src+len) = dst - src
}

/*===========================================================================
  PROLOG SIZE CALCULATION (reversed)
  ====================================
  In the reverse copy, "prolog" refers to the bytes at the HIGH end of the
  destination — the bytes written first to bring ptr_out down to a 32-byte
  boundary.  The calculation mirrors fmemcpy_asm but uses the epilog
  variable (which was computed as 32-dst) as the starting point.

  prolog = kernel & 31: sub-32-byte tail at the LOW end of the kernel range
  star3  = prolog & 7:  sub-dword bytes in the prolog
  shift  = star3 * 8:   bit shift for extracting sub-word data
  ptr_in is aligned DOWN to its 8-byte boundary.
===========================================================================*/

/* Packet: split prolog into sub-dword and dword parts; test if prolog is dword-aligned. */
{
        prolog = and(kernel, #31)          // prolog = bytes at LOW end (0..31)
        noprolog = bitsclr(kernel, #7)     // noprolog = (prolog & 7 == 0): no sub-dword part
        dcfetch(ptr_in_p_128)              // prefetch 64 bytes below src end
        ptr_in_p_128 = add(ptr_in_p_128, #-32) // advance to 96 bytes below
}
/* Packet: align ptr_in down to 8-byte boundary; compute shift and star3. */
{
        ptr_in = and(ptr_in, #-8)          // align src end down to 8-byte boundary
        shift = asl(prolog, #3)            // shift = prolog_bytes * 8
        star3 = and(prolog, #7)            // star3 = sub-dword byte count (0..7)
}
/* Packet: convert prolog to dword count; extract first shiftb; compute back. */
{
        prolog = lsr(prolog, #3)           // prolog = prolog / 8 (dword count)
        shiftb = and(shift, #8)            // shiftb = shift & 8 (byte-store shift)
        back = add(len, offset)            // back = total bytes from aligned src base
}

/*===========================================================================
  PREFETCH BURST + KERNEL SIZE
  ============================
  Issue 5 dcfetch instructions (working downward from src end).
  Compute final kernel count and adjust prolog for sp1loop0.
  inc = combine(#-32, #-1): prefetch DECREMENTS by 32, kernel decrements by 1.
===========================================================================*/

/* Packet: check if back > 8; compute kernel chunks; prefetch 96B below. */
{
        p3 = cmp.gt(back, #8)              // p3 = need to load dataF8 at entry
        kernel = lsr(kernel, #5)           // kernel = (len - epilog) / 32
        dcfetch(ptr_in_p_128)              // prefetch 96B below src end
        ptr_in_p_128 = add(ptr_in_p_128, #-32) // advance to 128B below
}
/* Packet: adjust prolog for sp1loop0 off-by-one; prefetch 128B below. */
{
        p1 = cmp.eq(prolog, #0)
        if(!p1.new) prolog = add(prolog, #1)  // sp1loop0 suppresses first write, so +1
        dcfetch(ptr_in_p_128)              // prefetch 128B below
        ptr_in_p_128 = add(ptr_in_p_128, #-32) // advance to 160B below
}
/* Packet: final two prefetches; check if kernel == 1. */
{
        dcfetch(ptr_in_p_128)              // prefetch 160B below
        ptr_in_p_128 = add(ptr_in_p_128, #-32) // advance to 192B below
        p2 = cmp.eq(kernel, #1)            // p2 = only one 32B chunk
}

/*===========================================================================
  INITIAL SOURCE LOADS (reversed)
  =================================
  Load the first two 8-byte source dwords from the HIGH end of the source.
  ptr_in is already aligned to an 8-byte boundary (pointing at the last
  aligned dword of the source).

  If offset == 0 (source end is exactly dword-aligned), we must NOT load
  dataF8 — doing so would read 8 bytes past the end of the source buffer.
  The conditional load is guarded by p2 = (offset != 0).

  The first load uses ptr_in++#-8 (post-decrement by 8), so after the
  packet ptr_in points to the dword below.
===========================================================================*/

.Lskip64:
/* Packet: finalise dalign; guard the dataF8 load; step ptr_in back by 8. */
{
        dalign = and(dalign, #31)          // dalign = (dst - src) & 31
        p2 = cmp.eq(offset, #0)            // p2 = source end is exactly dword-aligned
        ptr_in = add(ptr_in, #-8)          // step ptr_in back to last full dword
        if(!p2.new) dataF8 = memd(ptr_in)  // if not aligned: load upper window half
                                           //   (reads the partial dword at the end)
}
/* Packet: load lower window half; skip sub-dword prolog if prolog & 7 == 0. */
{
        data70 = memd(ptr_in++#-8)         // load 8 bytes; ptr_in -= 8
        if(noprolog) jump .Lnoprolog32     // skip sub-dword prolog
        align = offset                     // set valignb shift = source end alignment
}

/*===========================================================================
  SUB-DWORD PROLOG (reversed, 0–7 bytes)
  ========================================
  Write 0–7 bytes at the HIGH end of the destination to bring ptr_out
  down to a dword boundary.  Works from high to low: ptr_out is
  pre-decremented before each store (no post-increment).

  The bytes are extracted from the HIGH end of ldata0 using left-shift
  (lsl) rather than right-shift (lsr used in the forward copy).
  d0 holds the extracted byte/halfword value for the store.

  Bits 3, 4, 5 of shift (= prolog_bytes*8) encode which stores are needed,
  same as the forward copy.  offset is decremented (not incremented) as
  the source window is consumed.
===========================================================================*/

/* Packet: extract 8 bytes from the HIGH end of the source window; test for byte store;
   update running source offset (subtract star3). */
{
        ldata0 = valignb(dataF8, data70, align) // extract 8 bytes at byte `offset`
        ifbyte = tstbit(shift,#3)          // bit 3 set → need a 1-byte store
        offset = sub(offset, star3)        // offset -= sub-dword byte count
}
/* Packet: extract the byte value from the HIGH end of data1 (= high word of ldata0);
   left-shift ldata0 to consume the byte; pre-decrement ptr_out if storing. */
{
        d0 = lsr(data1, #24)               // d0 = top byte of ldata0 (byte to store)
        ldata0 = lsl(ldata0, shiftb)       // consume byte: shift ldata0 left by 8
        shiftb = and(shift, #16)           // shiftb = shift & 16 (halfword shift)
        if(ifbyte) ptr_out = add(ptr_out, #-1) // pre-decrement ptr_out for byte store
}
/* Packet: conditionally store byte; test for halfword; extract halfword value. */
{
	if(ifbyte) memb(ptr_out) = d0      // store byte at ptr_out (no post-increment)
        ifhword = tstbit(shift,#4)         // bit 4 set → need a 2-byte store
        d0 = lsr(data1, #16)               // d0 = top halfword of ldata0
        if(ifhword.new) ptr_out = add(ptr_out, #-2) // pre-decrement for halfword store
}
/* Packet: conditionally store halfword; left-shift ldata0 to consume it; test for word. */
{
	if(ifhword) memh(ptr_out) = d0     // store halfword at ptr_out
        ldata0 = lsl(ldata0, shiftb)       // consume halfword: shift ldata0 left by 16
        ifword = tstbit(shift,#5)          // bit 5 set → need a 4-byte store
}
/* Packet: pre-decrement ptr_out for word store; check if offset went negative
   (meaning we crossed a dword boundary and need to slide the window). */
{
        if(ifword) ptr_out = add(ptr_out, #-4) // pre-decrement for word store
        p2 = cmp.ge(offset, #0)            // p2 = offset >= 0 (no window slide needed)
}
/* Packet: conditionally store word (data1 = high word of ldata0);
   if offset went negative, slide the window downward. */
{
	if(ifword) memw(ptr_out) = data1   // store word at ptr_out
        if(!p2) dataF8 = data70            // slide: older becomes newer
        if(!p2) data70 = memd(ptr_in++#-8) // load next 8 source bytes (downward)
        align = offset                     // update valignb shift
}

/*===========================================================================
  DWORD PROLOG LOOP (reversed, 0–3 × 8-byte stores)
  ====================================================
  Write full 8-byte dwords downward until ptr_out reaches a 32-byte
  boundary.  ptr_out is pre-decremented by 8 before the loop, then
  post-decremented by 8 on each store (ptr_out++#-8 = post-decrement).
  After the loop, ptr_out is corrected back up by 8.
===========================================================================*/

.Lnoprolog32:
/* Packet: set up hardware loop; compute rest for the load guard; pre-decrement ptr_out. */
{
        p3 = sp1loop0(.Ldword_loop_prolog, prolog) // zero-overhead loop
        rest = sub(len, star3)             // rest = bytes remaining after sub-dword prolog
        ptr_out = add(ptr_out, #-8)        // pre-decrement: first store goes here
}
/* Loop body: store previous ldata0 downward; compute next ldata0; conditionally load. */
.Ldword_loop_prolog:
{
	if(p3) memd(ptr_out++#-8) = ldata0 // store 8 bytes; ptr_out -= 8 (suppressed first iter)
        ldata0 = valignb(dataF8, data70, align) // extract next 8 bytes from window
        p0 = cmp.gt(rest, #8)              // p0 = more than 8 bytes left to load
}
{
        dataF8 = data70                    // slide window: older becomes newer
        if(p0) data70 = memd(ptr_in++#-8) // conditionally load next 8 source bytes
        rest = add(rest, #-8)              // rest -= 8
}:endloop0
/* Correct ptr_out back up by 8 (the pre-decrement before the loop was one too many). */
        ptr_out = add(ptr_out, #8)

/*===========================================================================
  KERNEL SETUP (reversed)
  ========================
  Same peeling logic as fmemcpy_asm, but inc = combine(#-32, #-1):
  the prefetch pointer decrements by 32 each iteration (moving downward),
  and kernel decrements by 1.
===========================================================================*/

.Lkernel:
/* Packet: check if kernel > 0; peel last iteration into epilog. */
{
        p3 = cmp.gtu(kernel, #0)              // p3 = at least one 32-byte chunk
        if(p3.new) kernel = add(kernel, #-1)  // peel: last iteration handled in epilog
        if(p3.new) epilog = add(epilog, #32)  // those 32 bytes go into epilog
}
/* Standalone: re-check kernel after peeling. */
        p3 = cmp.eq(kernel, #0)
/* Packet: jump to epilog if nothing left; set up packed decrement; choose loop variant. */
{
        if(p3) jump  .Lepilog              // nothing left: go to epilog
        inc = combine(#-32, #-1)           // packed decrement: prefetch-=32, kernel-=1
        p1 = cmp.gt(dalign, #24)           // p1 = dalign > 24: use two-register pipeline
}
/* Packet: branch to odd-alignment loop if dalign > 24. */
{
        if(p1) jump  .Lodd_alignment       // dalign 25-31: ldata0/ldata1 pipeline
        rest = add(kernel, #0)             // rest = kernel (first-iter sentinel)
}
/* Packet: set up hardware loop; pre-decrement ptr_out. */
{
        loop0(.Loword_loop_25to31, kernel) // zero-overhead loop
        ptr_out = add(ptr_out, #-8)        // pre-decrement: first store goes here
}

/*===========================================================================
  KERNEL LOOP — EVEN ALIGNMENT (dalign 0–24), reversed
  ======================================================
  32 bytes per iteration (4 × 8-byte stores), working downward.
  Two-register pipeline (ldata0/ldata1).
  All stores use post-decrement (#-8): ptr_out moves downward.
  vaddw simultaneously decrements kernel and decrements the prefetch pointer.
===========================================================================*/

	.falign
.Loword_loop_25to31:
/* Packet: prefetch 128B below current src read pointer; advance prefetch and decrement kernel. */
{
        dcfetch(ptr_in_p_128)              // non-faulting prefetch 128B below
        ptr_in_p_128kernel = vaddw(ptr_in_p_128kernel, inc) // prefetch-=32, kernel-=1
        p3 = cmp.eq(kernel, rest)          // p3 = first iteration (suppress ldata1 store)
}
/* Packet: suppress ldata1 store on first iteration; compute ldata1; load next src dword. */
{
	if(!p3) memd(ptr_out++#-8) = ldata1 // store ldata1 downward (suppressed first iter)
        ldata1 = valignb(dataF8, data70, align) // compute ldata1 from window
        dataF8 = memd(ptr_in++#-8)         // load next 8 source bytes (downward)
}
/* Packet: store ldata0 downward; compute next ldata0; load next src dword. */
{
	memd(ptr_out++#-8) = ldata0        // store 8 bytes downward
        ldata0 = valignb(data70, dataF8, align) // compute ldata0 (args swapped)
        data70 = memd(ptr_in++#-8)         // load next 8 source bytes
}
/* Packet: store ldata1 downward; compute next ldata1; load next src dword. */
{
	memd(ptr_out++#-8) = ldata1        // store 8 bytes downward
        ldata1 = valignb(dataF8, data70, align) // compute ldata1
        dataF8 = memd(ptr_in++#-8)         // load next 8 source bytes
}
/* Packet: store ldata0 downward; compute next ldata0; load next src dword. */
{
	memd(ptr_out++#-8) = ldata0        // store 8 bytes downward (32 bytes this iter)
        ldata0 = valignb(data70, dataF8, align) // compute ldata0 for next iteration
        data70 = memd(ptr_in++#-8)         // load next 8 source bytes
}:endloop0
/* Post-loop: store the final ldata1 (peeled last iteration's first 8 bytes). */
{
	memd(ptr_out) = ldata1             // store last ldata1 (no decrement: last store)
        jump .Lepilog
}

/*===========================================================================
  KERNEL LOOP — ODD ALIGNMENT (dalign 25–31), reversed
  ======================================================
  Single-register pipeline (ldata0 only), working downward.
===========================================================================*/

.Lodd_alignment:
/* Packet: set up hardware loop; pre-decrement ptr_out. */
{
        loop0(.Loword_loop_00to24, kernel) // zero-overhead loop
        ptr_out = add(ptr_out, #-8)        // pre-decrement
}
	.falign
.Loword_loop_00to24:
/* Packet: prefetch 128B below; advance prefetch and decrement kernel. */
{
        dcfetch(ptr_in_p_128)              // non-faulting prefetch 128B below
        ptr_in_p_128kernel = vaddw(ptr_in_p_128kernel, inc) // prefetch-=32, kernel-=1
}
/* Four store-compute-load packets writing 32 bytes total, all downward. */
{
	memd(ptr_out++#-8) = ldata0        // store 8 bytes downward
        ldata0 = valignb(dataF8, data70, align) // compute next 8 bytes
        dataF8 = memd(ptr_in++#-8)         // load next 8 source bytes
}
{
	memd(ptr_out++#-8) = ldata0        // store 8 bytes downward
        ldata0 = valignb(data70, dataF8, align) // compute next 8 bytes (args swapped)
        data70 = memd(ptr_in++#-8)         // load next 8 source bytes
}
{
	memd(ptr_out++#-8) = ldata0        // store 8 bytes downward
        ldata0 = valignb(dataF8, data70, align) // compute next 8 bytes
        dataF8 = memd(ptr_in++#-8)         // load next 8 source bytes
}
{
	memd(ptr_out++#-8) = ldata0        // store 8 bytes downward (32 bytes this iter)
        ldata0 = valignb(data70, dataF8, align) // compute ldata0 for next iteration
        data70 = memd(ptr_in++#-8)         // load next 8 source bytes
}:endloop0
/* Correct ptr_out back up by 8 after the loop. */
        ptr_out = add(ptr_out, #8)

/*===========================================================================
  EPILOG SETUP (reversed)
  ========================
  The epilog writes the LOW end of the destination (the first bytes).
  Split remaining bytes into full dwords (epilogdws) and sub-dword tail.
===========================================================================*/

.Lepilog:
/* Packet: check for empty epilog; split into dword count and sub-dword remainder. */
{
        noepilog = cmp.eq(epilog,#0)       // noepilog = nothing left to write
        epilogdws = lsr(epilog, #3)        // epilogdws = epilog / 8 (full dwords)
}
/* Packet: return early if no epilog; compute bit-shift for sub-word extraction. */
{
        if(noepilog) jump .Lmemcpy_return  // nothing left: restore regs and return
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
  DWORD EPILOG LOOP (reversed, 0–3 × 8-byte stores)
  ====================================================
  Write full 8-byte dwords downward.  The store uses ptr_out+#-8 (store
  at ptr_out-8 without changing ptr_out) followed by ptr_out -= 8 in the
  same packet, which is equivalent to a pre-decrement store.
  The load guard uses epilog > 16 (not a stop address) because in the
  reverse direction we know we are reading within the original source range.
===========================================================================*/

.Ldword_loop_epilog:
/* Packet: store 8 bytes at ptr_out-8; compute next 8 bytes; decrement ptr_out. */
{
	memd(ptr_out+#-8) = ldata0         // store 8 bytes at ptr_out-8
        ldata0 = valignb(dataF8, data70, align) // extract next 8 bytes from window
        ptr_out = add(ptr_out, #-8)        // ptr_out -= 8
        p3 = cmp.gt(epilog, #16)           // p3 = more than 16 bytes left in epilog
}
/* Packet: slide window; conditionally load next source dword. */
{
        dataF8 = data70                    // slide window: older becomes newer
        if(p3) data70 = memd(ptr_in++#-8) // load next 8 source bytes if more remain
        epilog = add(epilog, #-8)          // epilog -= 8
}:endloop0

/*===========================================================================
  SUB-DWORD EPILOG (reversed, 0–7 bytes)
  ========================================
  Write the final 0–7 bytes at the LOW end of the destination.
  Same left-shift extraction as the sub-dword prolog: bytes are taken
  from the HIGH end of ldata0/data1, and ptr_out is pre-decremented
  before each store.
===========================================================================*/

.Lepilog60:
/* Packet: pre-decrement ptr_out for word store if needed. */
{
	if(ifword) ptr_out = add(ptr_out, #-4) // pre-decrement for word store
}
/* Packet: conditionally store word (data1 = high word of ldata0);
   left-shift ldata0 to consume it; test for halfword. */
{
	if(ifword) memw(ptr_out) = data1   // store word at ptr_out
        ifhword = tstbit(epilog,#1)        // bit 1 set → need a 2-byte store
        ldata0 = lsl(ldata0, shiftb)       // consume word: shift ldata0 left by 32
        shiftb = and(shift2, #16)          // shiftb = shift2 & 16 (halfword shift)
}
/* Packet: extract halfword value; pre-decrement ptr_out for halfword store;
   left-shift ldata0 to consume it. */
{
        d0 = lsr(data1, #16)               // d0 = top halfword of ldata0
	if(ifhword) ptr_out = add(ptr_out, #-2) // pre-decrement for halfword store
        ldata0 = lsl(ldata0, shiftb)       // consume halfword: shift ldata0 left by 16
}
/* Packet: conditionally store halfword; test for byte; extract byte value;
   pre-decrement ptr_out for byte store. */
{
	if(ifhword) memh(ptr_out) = d0     // store halfword at ptr_out
        ifbyte = tstbit(epilog,#0)         // bit 0 set → need a 1-byte store
        d0 = lsr(data1, #24)               // d0 = top byte of ldata0
	if(ifbyte.new) ptr_out = add(ptr_out, #-1) // pre-decrement for byte store
}
/* Packet: conditionally store byte; fall through to return. */
{
	if(ifbyte) memb(ptr_out) = d0      // store byte at ptr_out
        jump .Lmemcpy_return
}

/*===========================================================================
  FAST PATH: REVERSE BYTE LOOP  (len <= 23)
  ===========================================
  Copy bytes one at a time from high to low.
  ptr_out and ptr_in are pre-decremented by 1 before the loop so that
  the first store goes to dst+len-1 and the first load comes from src+len-1.
  The loop uses post-decrement (#-1) on both pointers.
===========================================================================*/

.Lshort_bytecpy:
/* Packet: pre-decrement both pointers; set up hardware loop. */
{
	ptr_out = add(ptr_out, #-1)        // ptr_out = dst + len - 1 (last byte of dst)
        ptr_in = add(ptr_in, #-1)          // ptr_in  = src + len - 1 (last byte of src)
        p3 = sp1loop0(.Lbyte_copy, len)    // zero-overhead loop: len iterations
}
	.falign
/* Loop body: store previous byte downward; load next byte downward. */
.Lbyte_copy:
{
        if(p3) memb(ptr_out++#-1) = data0  // store previous byte; ptr_out -= 1
        data0 = memb(ptr_in++#-1)          // load next byte; ptr_in -= 1
}:endloop0
/* Post-loop: write the last byte; return (no frame on this path). */
{
        memb(ptr_out) = data0              // write the final byte
        jumpr   r31                        // return (no frame allocated on this path)
}

/*===========================================================================
  FAST PATH: REVERSE DWORD LOOP  (src, dst, len all 8-aligned, len <= 95)
  =========================================================================
  All three of src, dst, and len are multiples of 8.  No valignb needed.
  ptr_out and ptr_in are pre-decremented by 8 before the loop.
===========================================================================*/

.Ldwordaligned:
/* Packet: set up hardware loop; pre-decrement both pointers. */
{
        p3 = sp1loop0(.Ldword_copy, len8)  // zero-overhead loop: len/8 iterations
	ptr_out = add(ptr_out, #-8)        // ptr_out = dst + len - 8 (last dword of dst)
        ptr_in = add(ptr_in, #-8)          // ptr_in  = src + len - 8 (last dword of src)
}
	.falign
/* Loop body: store previous dword downward; load next dword downward. */
.Ldword_copy:
{
        if(p3) memd(ptr_out++#-8) = ldata0 // store 8 bytes downward (suppressed first iter)
        ldata0 = memd(ptr_in++#-8)         // load next 8 source bytes downward
}:endloop0
/* Post-loop: write the last dword; return (no frame on this path). */
{
        memd(ptr_out) = ldata0             // write the final dword
        jumpr r31                          // return (no frame allocated on this path)
}

/*===========================================================================
  FUNCTION RETURN  (general path)
  =================================
  Restore callee-saved registers r16-r19, then deallocate frame and return.
  Note: only r16-r19 are restored (not r20-r21, which were not saved).
===========================================================================*/

.Lmemcpy_return:
/* Packet: restore r18-r19 and r16-r17. */
{
        r19:18 = memd(sp+#8)               // restore r18, r19
        r17:16 = memd(sp+#0)               // restore r16, r17
}
/* Deallocate frame (restores sp and r31) then return. */
        deallocframe                       // restore sp + r31
        jumpr r31                          // return (r0 = original dst, unchanged)
        .size rmemcpy_asm, . - rmemcpy_asm
