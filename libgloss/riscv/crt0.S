/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include "newlib.h"

#=========================================================================
# crt0.S : Entry point for RISC-V user programs
#=========================================================================

  .text
  .global _start
  .type   _start, @function
_start:
  # Initialize global pointer
.option push
.option norelax
1:auipc gp, %pcrel_hi(__global_pointer$)
  addi  gp, gp, %pcrel_lo(1b)
.option pop

  /* Initialize jvt CSR (reg addr: 0x0017) */
#ifndef __riscv_cmodel_large
  .weak __jvt_base$
  lla a0, __jvt_base$
#else
  la a0, .Laddr_jvt_base
  ld a0, 0(a0)
#endif
  beqz a0, .Ljvt_init_end
.option push
.option norelax
.option arch, +zicsr
  csrw 0x17, a0
.option pop
.Ljvt_init_end:

  # Clear the bss segment
#ifndef __riscv_cmodel_large
  la      a0, __bss_start
  la      a2, _end
#else
  la      a0, .Laddr_bss_start
  ld      a0, 0(a0)
  la      a2, .Laddr_end
  ld      a2, 0(a2)
#endif
  sub     a2, a2, a0
  li      a1, 0
  call    memset
#ifdef _LITE_EXIT
  # Make reference to atexit weak to avoid unconditionally pulling in
  # support code.  Refer to comments in __atexit.c for more details.
#ifndef __riscv_cmodel_large
  .weak   atexit
  la      a0, atexit
#else
  la      a0, .Laddr_atexit
  ld      a0, 0(a0)
#endif
  beqz    a0, .Lweak_atexit
  .weak   __libc_fini_array
#endif

#ifndef __riscv_cmodel_large
  la      a0, __libc_fini_array   # Register global termination functions
#else
  la      a0, .Laddr_libc_fini_array
  ld      a0, 0(a0)
#endif
  call    atexit                  #  to be called upon exit
#ifdef _LITE_EXIT
.Lweak_atexit:
#endif
  call    __libc_init_array       # Run global initialization functions

  lw      a0, 0(sp)                  # a0 = argc
  addi    a1, sp, __SIZEOF_POINTER__ # a1 = argv
  slli    a2, a0, 1 + __SIZEOF_POINTER__ >> 2
  addi    a2, a2, __SIZEOF_POINTER__
  add     a2, a2, a1                 # a2 = envp
  call    main
  tail    exit
#ifdef __riscv_cmodel_large
.Laddr_gp:
  .dword __global_pointer$
.Laddr_jvt_base:
  .weak __jvt_base$
  .dword __jvt_base$
.Laddr_bss_start:
  .dword __bss_start
.Laddr_end:
  .dword _end
.Laddr_atexit:
  .weak   atexit
  .dword atexit
.Laddr_libc_fini_array:
  .weak   __libc_fini_array
  .dword __libc_fini_array
#endif
  .size  _start, .-_start
