/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

 
/* The traptable has to be the first code in a boot PROM. */

#include <asm-leon/head.h>
                
#define TRAP(H)  mov %psr, %l0; sethi %hi(H), %l4; jmp %l4+%lo(H); nop;
#define TRAPL(H)  mov %g0, %l0; sethi %hi(H), %l4; jmp %l4+%lo(H); nop;
#define TRAP_ENTRY(H) rd %psr, %l0; b H; rd %wim, %l3; nop;

#define WIM_INIT 2
#ifdef _SOFT_FLOAT
#define PSR_INIT 0x0e0
#else
#define PSR_INIT 0x10e0
#endif

#define NUMREGWINDOWS 8

/* Unexcpected trap will halt the processor by forcing it to error state */
#define BAD_TRAP ta 0; nop; nop; nop;

/* Software trap. Treat as BAD_TRAP */
#define SOFT_TRAP BAD_TRAP

#define TT_MASK         0xff0   // trap type mask from tbr
#define TT_SHL          4       // shift to get a tbr value

  .seg    "text"

   /* ------- */
   .weak  _start_svt_weak
   .set	  _start_svt_weak,_start_svt_disp
   /* ------- */
	
  .global _trap_table, cpuinit, _hardreset, _hardreset_svt
  .global _fpdis_enable_svt,_fpdis_svt,_window_overflow_svt,_window_underflow_svt
  .global _leonbare_irq_entry_svt,_irqcall_svt,_flush_windows_svt,_srmmu_fault_svt,_irqcall_disableirq_svt
  .global start, _start_svt_real

_start_svt_real:	
	nop
	
_start_svt_disp:	
	rd	%tbr, %l3
	rd	%psr, %l0
	! here,locals have been set up as follows:
	! %l0 = psr
	! %l1 = pc	
	! %l2 = npc
	! %l3 = tbr
	and     %l3,TT_MASK,%l3
	srl     %l3,TT_SHL,%l3
	
/*
struct get {
  int start,end,func;
};

struct get table[3] = {
  {0,1,..},
  {0,0,0},
};

int gettrap(int nr){
  struct get *p = table;
  while((p->start) || (p->end) || (p->func)) {
    if (p->start <= nr && p->end >= nr) {
      return p->func;
    }
    p++;
  }
  return 0;
}
	
$sparc-elf-gcc -S gettrap.c -o test.S -O2
*/
	
#define loc_o0 l3	
#define loc_o1 l4	
#define loc_o2 l5	
#define loc_o3 l6	
	
        sethi   %hi(trap_table), %loc_o2
        or      %loc_o2, %lo(trap_table), %loc_o2
        mov     %loc_o0, %loc_o3
        ld      [%loc_o2], %loc_o1
.LL13:
        cmp     %loc_o1, %loc_o3
.LL12:
        bg,a    .LL11
        add     %loc_o2, 12, %loc_o2
        ld      [%loc_o2+4], %loc_o0
        cmp     %loc_o0, %loc_o3
        bge,a   .LL1
        ld      [%loc_o2+8], %loc_o0
        add     %loc_o2, 12, %loc_o2
.LL11:
        ld      [%loc_o2], %loc_o0
        orcc    %loc_o0, 0, %loc_o1
        bne     .LL12
        cmp     %loc_o1, %loc_o3
        ld      [%loc_o2+4], %loc_o0
        cmp     %loc_o0, 0
        bne     .LL12
        cmp     %loc_o1, %loc_o3
        ld      [%loc_o2+8], %loc_o0
        cmp     %loc_o0, 0
        bne     .LL12
        cmp     %loc_o1, %loc_o3
	
	!not in table
	BAD_TRAP 
	
.LL1:
	jmp %loc_o0
	 nop	

	
        .global trap_table,svt_trap_table_ext,svt_trap_table_ext_end
        .section        ".rodata"
        .align 4
trap_table:
	.long 0,0,       _hardreset_svt
	.long 1,1,       _srmmu_fault_svt		! 01 instruction_access_exception (in mmu_asm.S)
	.long 4,4
#ifndef _SOFT_FLOAT
	.long _fpdis_enable_svt				! 04 fp_disabled
#else
	.long _fpdis_svt				! 04 fp_disabled
#endif
#ifndef _FLAT
	.long 5,   5,    _window_overflow_svt		! 05 window_overflow
	.long 6,   6,    _window_underflow_svt		! 06 window_underflow
#endif
	.long 9,9,       _srmmu_fault_svt		! 09 data_access_exception (in mmu_asm.S)
	.long 0x11,0x1f, _leonbare_irq_entry_svt	! 11-1f interrupt level
	.long 0x82,0x82, _irqcall_svt			! 82
#ifndef _FLAT
	.long 0x83,0x83, _flush_windows_svt		! 83
#endif
	.long 0x85,0x85, _irqcall_disableirq_svt	! 85
svt_trap_table_ext:	
	.long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
        .long   0,0,0
svt_trap_table_ext_end:		
        .long   0,0,0

