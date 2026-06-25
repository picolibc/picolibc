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
  void *memmove(void *dst, const void *src, size_t len)

  Copies len bytes from src to dst, correctly handling overlapping regions.
  dst is returned.

  Overlap handling strategy
  -------------------------
  Three cases are distinguished based on the relationship between src and dst:

  Case 1: src > dst, OR |src - dst| > len  (no harmful overlap)
    → Forward copy is safe.  Delegate to fmemcpy_asm(dst, src, len).
      fmemcpy_asm is identical to memcpy but without dczeroa, which is
      unsafe when regions are adjacent.

  Case 2: src < dst AND len >= 15  (overlapping, large)
    → Reverse copy required.  Split into two rmemcpy_asm calls:
        rmemcpy_asm(dst+15, src+15, len-15)  — bulk of the data
        rmemcpy_asm(dst,    src,    15)       — first 15 bytes
      The split at 15 bytes is required by rmemcpy_asm's contract: it may
      under-read up to 15 bytes below the start of the source buffer (due
      to its aligned-read strategy), so the caller must ensure the first
      15 bytes are copied separately in a second call that is known to be
      within a safe page.

  Case 3: src < dst AND len < 15  (overlapping, small)
    → Reverse copy of the whole region: rmemcpy_asm(dst, src, len).

  Register usage
  --------------
  The function uses a minimal frame (8 bytes) to save r27:26, which are
  used to preserve dst (r0) and src (r1) across the calls to fmemcpy_asm
  and rmemcpy_asm (which may clobber r0 and r1 as return/argument registers).

    r0  dst   (argument, return value)
    r1  src   (argument)
    r2  len   (argument)
    r3  scratch: |src - dst|, used to test for non-overlapping regions
    r26 saved copy of src (r1) across calls
    r27 saved copy of dst (r0) across calls
    p0  src > dst
    p1  len >= 15

  allocframe_rb vs allocframe
  ---------------------------
  On arch > v82, allocframe_rb is used instead of allocframe.  It is
  functionally identical but includes a return-buffer hint that improves
  branch-predictor accuracy on the indirect return.
---------------------------------------------------------------------------*/

	.section .text
        .p2align 4,,15
        .globl memmove
        .type   memmove, @function
memmove:

/*===========================================================================
  ENTRY: allocate frame, classify overlap, save dst/src
===========================================================================*/

/* Packet: allocate 8-byte frame (saves r31); test len >= 15; compute src-dst; test src > dst.
   All four operations execute in parallel in the first cycle. */
        {
#if __HEXAGON_ARCH__ > 82
                allocframe_rb(#8)          // push LR; allocate 8 bytes (for r27:26)
#else
                allocframe(#8)             // push LR; allocate 8 bytes (for r27:26)
#endif
                p1 = cmp.gt(r2,#14)        // p1 = (len >= 15): large enough to split?
                r3 = sub(r1,r0)            // r3 = src - dst (signed)
                p0 = cmp.gtu(r1,r0)        // p0 = (src > dst): unsigned comparison
        }
/* Packet: save dst and src in r27:26 across calls; take absolute value of src-dst;
   jump to forward-copy path if src > dst (no overlap risk). */
        {
                memd(r29) = r27:26         // save r27, r26 on stack
                r27:26 = combine(r0,r1)    // r27 = dst, r26 = src (preserved across calls)
                r3 = abs(r3)               // r3 = |src - dst|
                if (p0) jump .L13          // src > dst: safe to copy forward
        }

/*===========================================================================
  src <= dst: check if regions actually overlap
  If |src - dst| > len the regions are disjoint even though src < dst,
  so a forward copy is still safe.
===========================================================================*/

/* Packet: test if |src - dst| > len (no overlap despite src < dst). */
        {
                p0 = cmp.gt(r3,r2)         // p0 = (|src-dst| > len): regions don't overlap
                if (!p0.new) jump:nt .L12  // they do overlap: must copy in reverse
        }

/*===========================================================================
  .L13: Forward copy path (src > dst, or regions don't overlap)
  Delegate entirely to fmemcpy_asm with the original arguments still in
  r0, r1, r2.  After the call, restore r27:26 and deallocate the frame
  before returning, so the tail-call-like pattern is explicit.
===========================================================================*/

.L13:
                call fmemcpy_asm           // forward copy: dst, src, len still in r0-r2
        {
                r27:26 = memd(r29)         // restore r27, r26
                deallocframe               // restore sp and r31
        }
                jumpr r31                  // return (r0 = dst, set by fmemcpy_asm)

/*===========================================================================
  .L12: Reverse copy path (src < dst AND regions overlap)
  Use rmemcpy_asm, which copies high-to-low to avoid clobbering unread
  source bytes when dst > src.

  If len >= 15: split into two calls to satisfy rmemcpy_asm's contract
  that the caller handles the first 15 bytes separately.
    Call 1: rmemcpy_asm(dst+15, src+15, len-15)  — bulk
    Call 2: rmemcpy_asm(dst,    src,    15)       — first 15 bytes

  If len < 15: single call rmemcpy_asm(dst, src, len).
===========================================================================*/

.L12:
                if (!p1) jump .L15         // len < 15: skip the split, do one call
/* Packet: set up arguments for the bulk reverse copy and call rmemcpy_asm.
   r0 = dst+15, r1 = src+15, r2 = len-15.
   r26 holds the saved src, r27 holds the saved dst. */
        {
                r2 = add(r2,#-15)          // len -= 15 (bulk portion)
                r1 = add(r26,#15)          // src + 15
                r0 = add(r27,#15)          // dst + 15
                call rmemcpy_asm           // reverse copy of bulk: (dst+15, src+15, len-15)
        }
/* Packet: set up arguments for the first-15-bytes reverse copy.
   r1:0 = combine(src, dst) = (r26, r27) restores original dst/src.
   r2 = 15. */
        {
                r1:0 = combine(r26,r27)    // r0 = dst (r27), r1 = src (r26)
                r2 = #15                   // len = 15
        }
/* Fall through to .L15 for the second rmemcpy_asm call. */

.L15:
                call rmemcpy_asm           // reverse copy: (dst, src, len) or (dst, src, 15)
/* Packet: restore r27:26 and deallocate frame; then return. */
        {
                r27:26 = memd(r29)         // restore r27, r26
                deallocframe               // restore sp and r31
        }
                jumpr r31                  // return (r0 = dst, set by rmemcpy_asm)
        .size memmove, . - memmove
