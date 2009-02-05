/*
 * ====================================================
 * Copyright (C) 2007 by Ellips BV. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

  #include "x86_64mach.h"

  .global SYM (memcpy)
  SOTYPE_FUNCTION(memcpy)

SYM (memcpy):
  movq    rdi, rax                /* Store destination in return value */
  cmpq    $16, rdx
  jb      byte_copy

  movq    rdi, r8                 /* Align destination on quad word boundary */
  andq    $7, r8
  jz      quadword_aligned
  movq    $8, rcx
  subq    r8, rcx
  subq    rcx, rdx
  rep     movsb

quadword_aligned:
  cmpq    $256, rdx
  jb      quadword_copy

  pushq    rax
  pushq    r12
  pushq    r13
  pushq    r14

  movq    rdx, rcx                /* Copy 128 bytes at a time with minimum cache polution */
  shrq    $7, rcx

  .p2align 4
loop:
  prefetchnta   768 (rsi)
  prefetchnta   832 (rsi)

  movq       (rsi), rax
  movq     8 (rsi), r8
  movq    16 (rsi), r9
  movq    24 (rsi), r10
  movq    32 (rsi), r11
  movq    40 (rsi), r12
  movq    48 (rsi), r13
  movq    56 (rsi), r14

  movntiq rax,    (rdi)
  movntiq r8 ,  8 (rdi)
  movntiq r9 , 16 (rdi)
  movntiq r10, 24 (rdi)
  movntiq r11, 32 (rdi)
  movntiq r12, 40 (rdi)
  movntiq r13, 48 (rdi)
  movntiq r14, 56 (rdi)

  movq     64 (rsi), rax
  movq     72 (rsi), r8
  movq     80 (rsi), r9
  movq     88 (rsi), r10
  movq     96 (rsi), r11
  movq    104 (rsi), r12
  movq    112 (rsi), r13
  movq    120 (rsi), r14

  movntiq rax,  64 (rdi)
  movntiq r8 ,  72 (rdi)
  movntiq r9 ,  80 (rdi)
  movntiq r10,  88 (rdi)
  movntiq r11,  96 (rdi)
  movntiq r12, 104 (rdi)
  movntiq r13, 112 (rdi)
  movntiq r14, 120 (rdi)

  leaq    128 (rsi), rsi
  leaq    128 (rdi), rdi

  dec     rcx
  jnz     loop

  sfence
  movq    rdx, rcx
  andq    $127, rcx
  rep     movsb
  popq    r14
  popq    r13
  popq    r12
  popq    rax
  ret


byte_copy:
  movq    rdx, rcx
  rep     movsb
  ret


quadword_copy:
  movq    rdx, rcx
  shrq    $3, rcx
  .p2align 4
  rep     movsq
  movq    rdx, rcx
  andq    $7, rcx
  rep     movsb                   /* Copy the remaining bytes */
  ret
