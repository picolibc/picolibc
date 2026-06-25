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
  void *memcpy(void *dst, const void *src, size_t len)

  Copies len bytes from src to dst.  dst is returned.  Regions must not
  overlap.

  Algorithm overview
  ------------------
  The copy is split into three phases relative to the destination pointer:

    prolog  : 0–31 bytes written to bring dst up to a 32-byte boundary.
              Sub-divided into a sub-dword part (0–7 bytes, written as
              byte/halfword/word stores) and a dword part (0–3 × 8-byte
              stores).

    kernel  : One or more 32-byte chunks written in a tight VLIW loop.
              Two loop variants are selected based on the relative
              alignment of src and dst (dalign = dst - src):
                dalign 25–31 : two-register software pipeline (ldata0/ldata1)
                dalign  0–24 : single-register pipeline

    epilog  : Remaining 0–31 bytes, mirroring the prolog structure.

  Fast paths (no frame allocation):
    len == 0 or src == dst  → immediate return
    len <= 23               → byte-by-byte loop
    src, dst, len all 8-aligned AND len <= 95 → dword loop

  Misalignment handling
  ---------------------
  The source is always read from an 8-byte-aligned address.  Two
  consecutive 8-byte registers form a sliding window:

      [ data70 (older) | dataF8 (newer) ]

  valignb(dataF8, data70, offset) extracts 8 bytes starting at byte
  `offset` within that window, producing correctly aligned output data
  regardless of the source's byte alignment.  This avoids any branch on
  alignment in the hot path.

  Cache optimisations
  -------------------
  dcfetch : non-faulting prefetch issued 128 bytes ahead of the source
            read pointer, keeping the source data in L1 cache.
  dczeroa : "allocate without fetch" issued ahead of the destination
            write pointer.  Tells the cache to allocate a line without
            reading it from memory first (safe because we will overwrite
            the entire line), eliminating the read-for-ownership miss.

  Over-read guard
  ---------------
  The sliding-window technique reads source data in 8-byte chunks from
  an aligned base, which can extend slightly past the end of the source
  buffer.  The `stop` register marks the last address at which a memd
  is safe; loads beyond it are suppressed with a conditional.
---------------------------------------------------------------------------*/

	.text
	.global memcpy
	.type memcpy, @function
memcpy:

/*---------------------------------------------------------------------------
  Register aliases
  ----------------
  C ABI arguments / return value:
    R0  ptr_out   destination pointer (returned unchanged)
    R1  ptr_in    source pointer
    R2  len       byte count

  Sliding source window (two consecutive 8-byte registers):
    R13:12  data70   "lower" (older) 8 bytes of the source window
    R11:10  dataF8   "upper" (newer) 8 bytes of the source window
  valignb(dataF8, data70, offset) extracts 8 bytes at byte `offset`.

  Output staging registers:
    R7:6   ldata0   even 8-byte output chunk (also used for sub-word stores)
    R19:18 ldata1   odd  8-byte output chunk (only used in the 25-31 kernel)
    R6     data0    low  word of ldata0 — used for byte/halfword/word stores
    R7     data1    high word of ldata0 (unused directly; part of ldata0 pair)

  Predicate registers (many names share the same physical register because
  they are never live at the same time):
    p0  ifbyte / ifhword / ifword   sub-word store flags (prolog and epilog)
    p0  noprolog                    prolog is already dword-aligned
    p0  noepilog                    no epilog bytes remain
    p0  kernel1                     kernel iteration count > 1
    p1  nokernel                    no 32-byte kernel chunks
    p2  align                       source is not dword-aligned (offset > 0)

  Scalar temporaries (several names share the same physical register;
  the active meaning is noted per phase):
    R19  dalign      (dst - src) & 31 — relative alignment, selects kernel variant
    R16  star3       sub-dword byte count in prolog (prolog & 7)
    R8   rest        scratch / loop counter
    R7   back        scratch (reuses data1 slot before ldata1 is live)
    R3   epilog      bytes remaining after the last full 32-byte chunk
    R15:14 inc       packed increment used in kernel: R15=+32 (prefetch), R14=-1 (kernel)
    R4   kernel      number of 32-byte chunks in the main loop
    R5   ptr_in_p_128       prefetch pointer (src + 128 ahead)
    R5:4 ptr_in_p_128kernel paired view: prefetch ptr (R5) and kernel count (R4)
                             vaddw on this pair simultaneously advances the prefetch
                             pointer and decrements the kernel counter each iteration
    R8   mask        0x7FFFFFFF bitmask used to cap prolog to len  [setup phase]
    R8   shift       prolog_bytes * 8, used to extract sub-word data [prolog phase]
    R5   shift2      epilog_bytes * 8, used to extract sub-word data [epilog phase]
    R15  prolog      bytes before the first 32-byte-aligned dst address [setup]
    R15  epilogdws   number of full dwords in the epilog              [epilog]
    R14  shiftb      intermediate shift amount for sub-word extraction
    R9   offset      ptr_in & 7 — source byte offset within its dword
    R17  ptr_out_p_32  rolling dczeroa target pointer
    R14  align888    OR of src, dst, len — used to test 8-byte alignment
    R9   len8        len / 8 — dword count for the aligned fast path
    R20  over        (ptr_in + len) & 7 — bytes src end extends past a dword boundary
    R21  stop        last address at which a memd on src is safe (over-read guard)
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
#define ptr_out_p_32       R17
#define align888           R14
#define len8               R9
#define over               R20
#define stop               R21

/*===========================================================================
  ENTRY GUARDS AND FAST-PATH DISPATCH
  ====================================
  Determine which of four paths to take:
    1. Trivial return  (len==0 or src==dst)
    2. Dword fast path (src, dst, len all 8-aligned AND len <= 95)
    3. Byte fast path  (len <= 23)
    4. General path    (everything else)
===========================================================================*/

/* On arch > v82, insert a return-buffer NOP to avoid a branch-predictor
   penalty on the indirect return at the end of the function. */
#if __HEXAGON_ARCH__ > 82
	nop_rb
#endif

/* Packet: compute early-exit predicate p1, alignment probe, and small-len flag.
   p1 = (len != 0) AND (src != dst) — both conditions must hold to proceed.
   The two "p1 =" assignments in the same packet implement a logical AND:
   the second write to p1 ANDs with the result of the first. */
{
        p1 = !cmp.eq(len, #0)              // p1 = (len != 0)
        p1 = !cmp.eq(ptr_in, ptr_out)      // p1 &= (src != dst)
        align888 = or(ptr_in, ptr_out)     // begin alignment probe: OR src and dst
        p0 = cmp.gtu(len, #23)             // p0 = (len > 23) — worth the general path?
}
/* Packet: early return if trivial; finish alignment probe; compute dword count.
   align888 will have bits 0-2 clear only if src, dst, AND len are all 8-aligned. */
{
        if(!p1) jumpr r31                  // return immediately if len==0 or src==dst
        p3 = cmp.gtu(len, #95)             // p3 = (len > 95) — too big for dword shortcut
        align888 = or(align888, len)       // OR in len to complete the alignment probe
        len8 = lsr(len, #3)                // len8 = len / 8 (dword count for aligned path)
}
/* Packet: prefetch first source cache line; test 8-byte alignment.
   bitsclr(x, mask) is true when all bits of mask are zero in x. */
{
	dcfetch(ptr_in)                    // non-faulting prefetch of first source line
	p2 = bitsclr(align888, #7)         // p2 = ((src|dst|len) & 7 == 0): all 8-aligned?
}
/* Packet: take dword fast path if all-aligned AND len <= 95.
   Subtract 8 from len because the dword loop reads one dword before the
   loop counter check, so it would overshoot by one iteration otherwise. */
{
	p2 = and(p2,!p3)                   // p2 = all-aligned AND len <= 95
	if(p2.new) len = add(len, #-8)     // adjust len for dword loop entry
	if(p2.new) jump:NT .Ldwordaligned  // take dword fast path (not-taken hint)
}
/* Packet: take byte fast path if len <= 23; start loading the prolog mask.
   prolog = -ptr_out: the low 5 bits give bytes until the next 32-byte boundary. */
{
	if(!p0) jump   .Lbytes23orless     // len <= 23: use byte-by-byte loop
	mask.l = #LO(0x7fffffff)           // load lower 16 bits of mask constant
	prolog = sub(#0, ptr_out)          // prolog = -dst (low bits = bytes to 32B boundary)
}

/*===========================================================================
  STACK FRAME SETUP
  =================
  Save callee-saved registers r16-r21 (6 registers = 24 bytes).
  Also finish loading the mask constant and set up the prefetch pointer.
===========================================================================*/

/* Packet: allocate frame, finish mask, set up prefetch pointer, count leading zeros.
   cl0(len) gives the number of leading zeros in len, used below to cap prolog. */
{
        allocframe(#24)                    // push LR; allocate 24 bytes (for r16-r21)
	mask.h = #HI(0x7fffffff)           // load upper 16 bits: mask = 0x7FFFFFFF
	ptr_in_p_128 = add(ptr_in, #32)    // prefetch pointer starts 32 bytes ahead of src
	back = cl0(len)                    // count leading zeros of len (used to cap prolog)
}
/* Packet: save r16-r17; compute prolog size; extract source byte offset. */
{
	memd(sp+#0) = R17:16               // save r16, r17
	prolog &= lsr(mask, back)          // prolog = (-dst) & (0x7FFFFFFF >> clz(len))
	                                   //   → bytes to next 32B boundary, capped at len
	offset = and(ptr_in, #7)           // offset = src & 7: byte misalignment within dword
}
/* Packet: save r18-r19; compute relative alignment; note if src is misaligned. */
{
	memd(sp+#8) = R19:18               // save r18, r19
	dalign = sub(ptr_out, ptr_in)      // dalign = dst - src (will be masked to &31 later)
	p3 = cmp.gt(offset, #0)            // p3 = src is not dword-aligned
}
/* Packet: save r20-r21; begin computing the over-read guard (stop).
   over = src + len = one-past-end of the source buffer.
   back = offset + len = total bytes spanned from the aligned src base. */
{
	over = add(len, ptr_in)            // over = src + len (one-past-end of source)
	back = add(len, offset)            // back = bytes from aligned src base to src end
	memd(sp+#16) = R21:20              // save r20, r21
	stop = len                         // stop will become the over-read guard address
}

/*===========================================================================
  PROLOG SIZE CALCULATION
  =======================
  Split `prolog` (bytes before the first 32-byte-aligned dst address) into:
    star3  = prolog & 7          sub-dword bytes (written as byte/hword/word)
    prolog = prolog >> 3         full dwords     (written in the dword prolog loop)
    shift  = star3 * 8           bit shift for extracting sub-word data from ldata0
  Also align ptr_in down to its 8-byte boundary so all source reads are aligned.
===========================================================================*/

/* Packet: test if prolog is already dword-aligned; mask prolog to 5 bits; prefetch. */
{
	noprolog = bitsclr(prolog, #7)     // noprolog = (prolog & 7 == 0): no sub-dword part
	prolog = and(prolog, #31)          // keep only 5 bits (prolog is 0..31)
	dcfetch(ptr_in_p_128)              // prefetch 32 bytes ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance prefetch pointer to 64B ahead
}
/* Packet: compute kernel byte count; bit-shift for sub-word extraction; align src ptr. */
{
	kernel = sub(len, prolog)          // kernel = len - prolog (bytes after prolog)
	shift = asl(prolog, #3)            // shift = prolog_bytes * 8 (for sub-word extraction)
	star3 = and(prolog, #7)            // star3 = sub-dword byte count in prolog (0..7)
	ptr_in = and(ptr_in, #-8)          // align src down to 8-byte boundary
}
/* Packet: convert prolog to dword count; compute epilog; set dczeroa target; finish over. */
{
	prolog = lsr(prolog, #3)           // prolog = prolog / 8 (number of dwords to write)
	epilog = and(kernel, #31)          // epilog = kernel & 31 (bytes after last 32B chunk)
	ptr_out_p_32 = add(ptr_out, prolog)// dczeroa target: first 32B-aligned dst cache line
	over = and(over, #7)               // over = (src+len) & 7: bytes past last dword boundary
}

/*===========================================================================
  OVER-READ GUARD (stop)
  ======================
  The sliding-window reads src in 8-byte aligned chunks, which may extend
  slightly past the end of the source buffer.  `stop` is the last address
  at which a memd is safe; loads at or beyond stop are suppressed.

  stop = ptr_in_aligned + len - over
       = last aligned address that is fully within the source buffer
  If over > 0 (src end is not dword-aligned), allow one more read (+8).
===========================================================================*/

/* Packet: check if src end is dword-aligned; compute stop base. */
{
	p3 = cmp.gt(over, #0)              // p3 = src end is not dword-aligned
	stop+= sub(ptr_in, over)           // stop = ptr_in_aligned + len - over
}
/* Packet: if src end is not dword-aligned, allow one more 8-byte read. */
{
	if(p3) stop = add(stop, #8)        // extend stop by 8 if src end is mid-dword
}

/*===========================================================================
  PREFETCH BURST + KERNEL SIZE
  ============================
  Issue 5 dcfetch instructions to fill the prefetch pipeline (5 × 32 = 160
  bytes ahead of the current src read pointer).  Simultaneously compute the
  final kernel count (number of 32-byte chunks) and adjust the prolog loop
  count for the sp1loop0 off-by-one convention.
===========================================================================*/

/* Packet: check if back > 8 (need second initial dword load); compute kernel chunks; prefetch. */
{
	p3 = cmp.gtu(back, #8)             // p3 = need to load dataF8 at entry (back > 8)
	kernel = lsr(kernel, #5)           // kernel = (len - prolog) / 32: number of 32B chunks
	dcfetch(ptr_in_p_128)              // prefetch 64B ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance to 96B ahead
}
/* Packet: adjust prolog count for sp1loop0 (which suppresses the first write);
   prefetch 96B ahead. */
{
	p1 = cmp.eq(prolog, #0)
	if(!p1.new) prolog = add(prolog, #1)  // sp1loop0 suppresses iteration 0, so add 1
	dcfetch(ptr_in_p_128)              // prefetch 96B ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance to 128B ahead
}
/* Packet: set nokernel flag; prefetch 128B ahead; extract first shiftb value. */
{
	nokernel = cmp.eq(kernel,#0)       // nokernel = no 32-byte chunks to copy
	dcfetch(ptr_in_p_128)              // prefetch 128B ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance to 160B ahead
	shiftb = and(shift, #8)            // shiftb = shift & 8 (byte-store shift amount)
}
/* Packet: final prefetch; skip dczeroa if no kernel; check if kernel == 1. */
{
	dcfetch(ptr_in_p_128)              // prefetch 160B ahead
	ptr_in_p_128 = add(ptr_in_p_128, #32) // advance to 192B ahead
	if(nokernel)  jump .Lskip64        // no kernel: skip dczeroa pre-allocation
	p2 = cmp.eq(kernel, #1)            // p2 = only one 32B chunk (don't advance dczeroa ptr)
}

/*===========================================================================
  DCZEROA PRE-ALLOCATION
  ======================
  Pre-allocate two destination cache lines with dczeroa ("allocate without
  fetch").  This tells the cache to create a clean line without reading the
  old contents from memory, eliminating the read-for-ownership miss since
  we will overwrite the entire line.  Only done when kernel > 0.
===========================================================================*/

/* Packet: zero-allocate first output cache line; conditionally advance ptr for second. */
{
	dczeroa(ptr_out_p_32)              // allocate dst cache line (no memory read)
	if(!p2) ptr_out_p_32 = add(ptr_out_p_32, #32) // advance only if kernel > 1
}
/* Packet: zero-allocate second output cache line; finalise dalign. */
{
	dalign = and(dalign, #31)          // dalign = (dst - src) & 31: relative alignment
	dczeroa(ptr_out_p_32)              // allocate second dst cache line
}

/*===========================================================================
  INITIAL SOURCE LOADS
  ====================
  Load the first two 8-byte source dwords into the sliding window
  (data70, dataF8).  ptr_in is already aligned to an 8-byte boundary.

  The load uses post-increment #16 (not #8) because the conditional load
  of dataF8 in the same packet uses ptr_in+#8 without incrementing, so
  the net effect is ptr_in advances by 16 (past both dwords).
===========================================================================*/

.Lskip64:
/* Packet: load first (and optionally second) source dword; skip sub-dword prolog if
   prolog is already dword-aligned. */
{
	data70 = memd(ptr_in++#16)         // load 8 bytes from aligned src; ptr_in += 16
	if(p3) dataF8 = memd(ptr_in+#8)   // if back>8: also load next 8 bytes (no increment)
	if(noprolog) jump .Lnoprolog32     // skip sub-dword prolog if prolog & 7 == 0
	align = offset                     // set valignb shift = src byte misalignment
}

/*===========================================================================
  SUB-DWORD PROLOG  (0–7 bytes)
  ==============================
  Write 0–7 bytes to bring ptr_out to a dword (8-byte) boundary.
  The bytes to write are encoded in bits 3, 4, 5 of `shift` (= prolog_bytes*8):
    bit 3 set → write 1 byte
    bit 4 set → write 2 bytes (halfword)
    bit 5 set → write 4 bytes (word)
  After each store, ldata0 is right-shifted to consume those bytes.
===========================================================================*/

/* Packet: extract first 8 aligned bytes from source window via valignb;
   test whether a byte store is needed; update running source offset. */
{
	ldata0 = valignb(dataF8, data70, align) // extract 8 bytes at byte `offset` of window
	ifbyte = tstbit(shift,#3)          // bit 3 of shift set → need a 1-byte store
	offset = add(offset, star3)        // offset += sub-dword byte count
}
/* Packet: conditionally store 1 byte; shift ldata0 to consume it; test for halfword. */
{
	if(ifbyte) memb(ptr_out++#1) = data0  // store byte if needed; advance dst
	ldata0 = lsr(ldata0, shiftb)       // consume the byte from ldata0 (shift right 8)
	shiftb = and(shift, #16)           // shiftb = shift & 16 (halfword-store shift amount)
	ifhword = tstbit(shift,#4)         // bit 4 of shift set → need a 2-byte store
}
/* Packet: conditionally store halfword; shift ldata0; test for word; check offset overflow. */
{
	if(ifhword) memh(ptr_out++#2) = data0 // store halfword if needed; advance dst
	ldata0 = lsr(ldata0, shiftb)       // consume the halfword from ldata0 (shift right 16)
	ifword = tstbit(shift,#5)          // bit 5 of shift set → need a 4-byte store
	p2 = cmp.gtu(offset, #7)           // p2 = running offset crossed a dword boundary
}
/* Packet: conditionally store word; if offset crossed dword boundary, slide the window. */
{
	if(ifword) memw(ptr_out++#4) = data0  // store word if needed; advance dst
	if(p2) data70 = dataF8             // slide window: old upper becomes new lower
	if(p2) dataF8 = memd(ptr_in++#8)  // load next 8 source bytes into upper window
	align = offset                     // update valignb shift for dword prolog loop
}

.Lnoprolog32:
/*===========================================================================
  DWORD PROLOG LOOP  (0–3 × 8-byte stores)
  ==========================================
  Write full 8-byte dwords until ptr_out reaches a 32-byte boundary.
  Uses sp1loop0 (zero-overhead hardware loop).  The loop body is software-
  pipelined: the store of the previously computed ldata0 is paired with the
  computation of the next ldata0 via valignb, and a conditional source load.

  sp1loop0 suppresses the store on the first iteration (p3 is false on
  entry), which is why prolog was incremented by 1 above.
===========================================================================*/

/* Packet: set up hardware loop for `prolog` iterations. */
{
	p3 = sp1loop0(.Ldword_loop_prolog, prolog) // zero-overhead loop: prolog iterations
}
/* Loop body: store previous ldata0; compute next ldata0; conditionally load next source dword. */
.Ldword_loop_prolog:
{
	if(p3) memd(ptr_out++#8) = ldata0  // store 8 bytes (suppressed on first iteration)
	ldata0 = valignb(dataF8, data70, align) // extract next 8 bytes from sliding window
	p0 = cmp.ltu(ptr_in, stop)         // p0 = safe to load more source data
}
{
	data70 = dataF8                    // slide window: upper becomes lower
	if(p0) dataF8 = memd(ptr_in++#8)  // conditionally load next 8 source bytes
}:endloop0

/*===========================================================================
  KERNEL SETUP
  ============
  Peel the last kernel iteration: move its 32 bytes into the epilog path.
  This avoids an edge-case where the last kernel iteration would need to
  suppress some stores.  Then choose between two loop variants based on
  dalign (the relative alignment of dst and src, mod 32).
===========================================================================*/

.Lkernel:
/* Packet: check if kernel > 0; peel last iteration into epilog. */
{
	p3 = cmp.gtu(kernel, #0)              // p3 = at least one 32-byte chunk to copy
	if(p3.new) kernel = add(kernel, #-1)  // peel: last iteration handled in epilog
	if(p3.new) epilog = add(epilog, #32)  // those 32 bytes go into the epilog path
}
/* Packet: re-check kernel after peeling; set up packed increment; choose loop variant.
   inc = combine(#32, #-1): R15=+32 (prefetch advance), R14=-1 (kernel decrement).
   vaddw on ptr_in_p_128kernel (R5:4) simultaneously applies both in one SIMD add. */
{
	nokernel = cmp.eq(kernel, #0)         // re-check: did peeling empty the kernel?
	if(nokernel.new) jump:NT  .Lepilog    // if so, skip to epilog (not-taken hint)
	inc = combine(#32, #-1)               // packed increment: prefetch +32, kernel -1
	p3 = cmp.gtu(dalign, #24)             // p3 = dalign > 24: use two-register pipeline
}
/* Packet: branch to odd-alignment loop if dalign > 24. */
{
	if(p3) jump  .Lodd_alignment          // dalign 25-31: use ldata0/ldata1 pipeline
}
/* Packet: set up hardware loop for even-alignment variant; save kernel for first-iter check. */
{
	loop0(.Loword_loop_25to31, kernel)    // zero-overhead loop: kernel iterations
	kernel1 = cmp.gtu(kernel, #1)         // kernel1 = more than one iteration remains
	rest = kernel                          // rest = initial kernel count (first-iter sentinel)
}

/*===========================================================================
  KERNEL LOOP — EVEN ALIGNMENT (dalign 0–24)
  ============================================
  Writes 32 bytes per iteration (4 × 8-byte stores).
  Uses a two-register software pipeline (ldata0 / ldata1) to hide the
  valignb latency.  The first store is suppressed on the first iteration
  (p3 = cmp.eq(kernel, rest) is true on entry, so !p3 is false).

  Each iteration:
    - dcfetch 128 bytes ahead of src read pointer
    - dczeroa the next 32-byte dst cache line (if more iterations remain)
    - vaddw simultaneously decrements kernel and advances prefetch pointer
    - 4 × (store 8 bytes, compute next 8 bytes via valignb, load next src dword)
===========================================================================*/

     .falign                               // align loop top to fetch-packet boundary
.Loword_loop_25to31:
/* Packet: prefetch 128B ahead; advance dczeroa pointer if more iterations remain. */
{
	dcfetch(ptr_in_p_128)              // non-faulting prefetch 128B ahead of src
	if(kernel1) ptr_out_p_32 = add(ptr_out_p_32, #32) // advance dczeroa target
}
/* Packet: allocate next dst cache line; test if this is the first iteration. */
{
	dczeroa(ptr_out_p_32)              // allocate dst cache line without memory read
	p3 = cmp.eq(kernel, rest)          // p3 = first iteration (suppress ldata1 store)
}
/* Packet: advance prefetch ptr and decrement kernel atomically via vaddw;
   suppress ldata1 store on first iteration; compute ldata1; load next src dword. */
{
	ptr_in_p_128kernel = vaddw(ptr_in_p_128kernel, inc) // prefetch+=32, kernel-=1
	if(!p3) memd(ptr_out++#8) = ldata1 // store ldata1 (suppressed on first iteration)
	ldata1 = valignb(dataF8, data70, align) // compute ldata1 from sliding window
	data70 = memd(ptr_in++#8)          // load next 8 source bytes into lower window
}
/* Packet: store ldata0; compute next ldata0; load next src dword. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(data70, dataF8, align) // compute ldata0 (note: args swapped)
	dataF8 = memd(ptr_in++#8)          // load next 8 source bytes into upper window
}
/* Packet: store ldata1; compute next ldata1; load next src dword. */
{
	memd(ptr_out++#8) = ldata1         // store 8 bytes
	ldata1 = valignb(dataF8, data70, align) // compute ldata1
	data70 = memd(ptr_in++#8)          // load next 8 source bytes
}
/* Packet: store ldata0; compute next ldata0; load next src dword; update kernel1 flag. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes (32 bytes written this iteration)
	ldata0 = valignb(data70, dataF8, align) // compute ldata0 for next iteration
	dataF8 = memd(ptr_in++#8)          // load next 8 source bytes
	kernel1 = cmp.gtu(kernel, #1)      // update: more than one iteration still remaining?
}:endloop0
/* Post-loop: store the final ldata1 (the peeled last iteration's first 8 bytes). */
{
	memd(ptr_out++#8) = ldata1         // store last ldata1
	jump .Lepilog                      // proceed to epilog
}

/*===========================================================================
  KERNEL LOOP — ODD ALIGNMENT (dalign 25–31)
  ============================================
  Same structure as the even-alignment loop but uses only ldata0 (no
  ldata1 alternation).  This variant is used when dalign > 24 because the
  valignb latency is hidden differently at those alignments.
===========================================================================*/

.Lodd_alignment:
/* Packet: set up hardware loop; initialise kernel1 and rest for first-iter suppression. */
{
	loop0(.Loword_loop_00to24, kernel)    // zero-overhead loop: kernel iterations
	kernel1 = cmp.gtu(kernel, #1)         // kernel1 = more than one iteration remains
	rest = add(kernel, #-1)               // rest = kernel - 1 (first-iter sentinel)
}
     .falign                               // align loop top to fetch-packet boundary
.Loword_loop_00to24:
/* Packet: prefetch 128B ahead; advance prefetch ptr and decrement kernel via vaddw;
   advance dczeroa pointer if more iterations remain. */
{
	dcfetch(ptr_in_p_128)              // non-faulting prefetch 128B ahead of src
	ptr_in_p_128kernel = vaddw(ptr_in_p_128kernel, inc) // prefetch+=32, kernel-=1
	if(kernel1) ptr_out_p_32 = add(ptr_out_p_32, #32) // advance dczeroa target
}
/* Packet: allocate next dst cache line. */
{
	dczeroa(ptr_out_p_32)              // allocate dst cache line without memory read
}
/* Packet: store ldata0; compute next ldata0; load next src dword. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(dataF8, data70, align) // compute next 8 bytes from sliding window
	data70 = memd(ptr_in++#8)          // load next 8 source bytes into lower window
}
/* Packet: store ldata0; compute next ldata0; load next src dword. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(data70, dataF8, align) // compute next 8 bytes (args swapped)
	dataF8 = memd(ptr_in++#8)          // load next 8 source bytes into upper window
}
/* Packet: store ldata0; compute next ldata0; load next src dword. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(dataF8, data70, align) // compute next 8 bytes
	data70 = memd(ptr_in++#8)          // load next 8 source bytes
}
/* Packet: store ldata0; compute next ldata0; load next src dword; update kernel1 flag. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes (32 bytes written this iteration)
	ldata0 = valignb(data70, dataF8, align) // compute ldata0 for next iteration
	dataF8 = memd(ptr_in++#8)          // load next 8 source bytes
	kernel1 = cmp.gtu(kernel, #1)      // update: more than one iteration still remaining?
}:endloop0

/*===========================================================================
  EPILOG SETUP
  ============
  Compute how many full dwords (epilogdws) and sub-dword bytes (kernel & 7)
  remain.  If nothing remains, jump straight to the return path.
===========================================================================*/

.Lepilog:
/* Packet: check for empty epilog; split epilog into dword count and sub-dword remainder. */
{
	noepilog = cmp.eq(epilog,#0)       // noepilog = no bytes left to write
	epilogdws = lsr(epilog, #3)        // epilogdws = epilog / 8 (full dwords)
	kernel = and(epilog, #7)           // kernel (reused) = epilog & 7 (sub-dword bytes)
}
/* Packet: return early if no epilog; compute bit-shift for sub-word extraction. */
{
	if(noepilog) jump .Lmemcpy_return  // nothing left: restore regs and return
	if(noepilog) ptr_out = sub(ptr_out, len) // (dead path) restore ptr_out
	p3 = cmp.eq(epilogdws, #0)         // p3 = no full dwords in epilog
	shift2 = asl(epilog, #3)           // shift2 = epilog_bytes * 8 (for sub-word extraction)
}
/* Packet: extract word-store shift; test for word store; skip dword loop if none. */
{
	shiftb = and(shift2, #32)          // shiftb = shift2 & 32 (word-store shift amount)
	ifword = tstbit(epilog,#2)         // bit 2 of epilog set → need a 4-byte store
	if(p3) jump .Lepilog60             // no full dwords: go straight to sub-dword epilog
}
/* Packet: set up hardware loop for epilogdws iterations. */
{
	loop0(.Ldword_loop_epilog, epilogdws) // zero-overhead loop: epilogdws iterations
}

/*===========================================================================
  DWORD EPILOG LOOP  (0–3 × 8-byte stores)
  ==========================================
  Mirror of the dword prolog loop.  Writes full 8-byte dwords from the
  sliding window, guarded by the over-read stop address.
===========================================================================*/

.Ldword_loop_epilog:
/* Packet: store 8 bytes; compute next 8 bytes; check if safe to load more source. */
{
	memd(ptr_out++#8) = ldata0         // store 8 bytes
	ldata0 = valignb(dataF8, data70, align) // extract next 8 bytes from sliding window
	p3 = cmp.ltu(ptr_in, stop)         // p3 = ptr_in is still within safe read range
}
/* Packet: slide window; conditionally load next source dword (guarded by stop). */
{
	data70 = dataF8                    // slide window: upper becomes lower
	if(p3) dataF8 = memd(ptr_in++#8)  // load next 8 source bytes only if safe
}:endloop0

/*===========================================================================
  SUB-DWORD EPILOG  (0–7 bytes)
  ==============================
  Mirror of the sub-dword prolog.  Write the final 0–7 bytes as word,
  halfword, and/or byte stores based on bits 2, 1, 0 of `epilog`.
  After all stores, restore ptr_out to its original value (= ptr_out - len)
  to satisfy the C ABI requirement that memcpy returns the original dst.
===========================================================================*/

.Lepilog60:
/* Packet: conditionally store word; shift ldata0 to consume it; test for halfword. */
{
	if(ifword) memw(ptr_out++#4) = data0  // store 4 bytes if needed; advance dst
	ldata0 = lsr(ldata0, shiftb)       // consume the word from ldata0 (shift right 32)
	ifhword = tstbit(epilog,#1)        // bit 1 of epilog set → need a 2-byte store
	shiftb = and(shift2, #16)          // shiftb = shift2 & 16 (halfword-store shift amount)
}
/* Packet: conditionally store halfword; shift ldata0; test for byte; adjust len for return. */
{
	if(ifhword) memh(ptr_out++#2) = data0 // store 2 bytes if needed; advance dst
	ldata0 = lsr(ldata0, shiftb)       // consume the halfword from ldata0 (shift right 16)
	ifbyte = tstbit(epilog,#0)         // bit 0 of epilog set → need a 1-byte store
	if(ifbyte.new) len = add(len, #-1) // adjust len so ptr_out - len = original dst
}
/* Packet: conditionally store byte; restore ptr_out to original dst value; fall through. */
{
	if(ifbyte) memb(ptr_out) = data0   // store 1 byte if needed (no advance: last store)
	ptr_out = sub(ptr_out, len)        // restore ptr_out = original dst (return value)
	jump .Lmemcpy_return               // restore callee-saved regs and return
}

/*===========================================================================
  FAST PATH: BYTE LOOP  (len <= 23)
  ===================================
  Simple byte-by-byte copy using a zero-overhead hardware loop.
  sp1loop0 suppresses the store on the first iteration (p3 is false),
  so the loop body is software-pipelined: load byte N while storing byte N-1.
  After the loop, write the last byte and restore ptr_out.
===========================================================================*/

.Lbytes23orless:
/* Packet: set up hardware loop for len iterations; adjust len for ptr_out restoration. */
{
	p3 = sp1loop0(.Lbyte_copy, len)    // zero-overhead loop: len iterations
	len = add(len, #-1)                // pre-decrement len for ptr_out = ptr_out - len
}
/* Loop body: load byte from src; store previous byte to dst (suppressed on first iter). */
.Lbyte_copy:
{
	data0 = memb(ptr_in++#1)           // load 1 byte from src; advance src
	if(p3) memb(ptr_out++#1) = data0   // store previous byte to dst (skip first iter)
}:endloop0
/* Post-loop: write the last byte; restore ptr_out; return (no frame to deallocate). */
{
	memb(ptr_out) = data0              // write the final byte
	ptr_out = sub(ptr_out, len)        // restore ptr_out = original dst (return value)
	jumpr   r31                        // return (no frame allocated on this path)
}

/*===========================================================================
  FAST PATH: DWORD LOOP  (src, dst, len all 8-aligned, len <= 95)
  =================================================================
  All three of src, dst, and len are multiples of 8, so no valignb is
  needed.  Simple 8-byte store loop using a zero-overhead hardware loop.
  len was pre-decremented by 8 at entry to account for the one dword read
  before the loop counter check.
===========================================================================*/

.Ldwordaligned:
/* Packet: set up hardware loop for len8 = len/8 iterations. */
{
	p3 = sp1loop0(.Ldword_copy, len8)  // zero-overhead loop: len/8 iterations
}
/* Loop body: store previous dword; load next dword (store suppressed on first iter). */
.Ldword_copy:
{
	if(p3) memd(ptr_out++#8) = ldata0  // store 8 bytes (suppressed on first iteration)
	ldata0 = memd(ptr_in++#8)          // load next 8 source bytes
}:endloop0
/* Post-loop: write the last dword; restore ptr_out; return (no frame allocated). */
{
	memd(ptr_out) = ldata0             // write the final dword
	ptr_out = sub(ptr_out, len)        // restore ptr_out = original dst (return value)
	jumpr r31                          // return (no frame allocated on this path)
}

/*===========================================================================
  FUNCTION RETURN  (general path)
  =================================
  Restore callee-saved registers r16-r21 from the stack frame, then use
  dealloc_return to deallocate the frame (restore sp and r31) and jump to
  the return address in a single instruction.
===========================================================================*/

.Lmemcpy_return:
/* Packet: restore r20-r21 and r18-r19 from stack. */
{
	r21:20 = memd(sp+#16)              // restore r20, r21
	r19:18 = memd(sp+#8)               // restore r18, r19
}
/* Packet: restore r16-r17; deallocate frame and return in one instruction.
   dealloc_return restores sp (undoes allocframe) and jumps to the saved r31. */
{
	r17:16 = memd(sp+#0)               // restore r16, r17
	dealloc_return                     // restore sp + r31, then jump to r31
}

	.size memcpy, . - memcpy
