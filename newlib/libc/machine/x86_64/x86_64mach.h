/*
 ** This file is distributed WITHOUT ANY WARRANTY; without even the implied
 ** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __USER_LABEL_PREFIX__
#define __USER_LABEL_PREFIX__ _
#endif

#define __REG_PREFIX__ %

/* ANSI concatenation macros.  */

#define CONCAT1(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a##b

/* Use the right prefix for global labels.  */

#define SYM(x) CONCAT1(__USER_LABEL_PREFIX__, x)

/* Use the right prefix for registers.  */

#define REG(x) CONCAT1(__REG_PREFIX__, x)

#define rax REG(rax)
#define rbx REG(rbx)
#define rcx REG(rcx)
#define rdx REG(rdx)
#define rsi REG(rsi)
#define rdi REG(rdi)
#define rbp REG(rbp)
#define rsp REG(rsp)

#define r8  REG(r8)
#define r9  REG(r9)
#define r10 REG(r10)
#define r11 REG(r11)
#define r12 REG(r12)
#define r13 REG(r13)
#define r14 REG(r14)
#define r15 REG(r15)

#define eax REG(eax)
#define ebx REG(ebx)
#define ecx REG(ecx)
#define edx REG(edx)
#define esi REG(esi)
#define edi REG(edi)
#define ebp REG(ebp)
#define esp REG(esp)

#define st0 REG(st)
#define st1 REG(st(1))
#define st2 REG(st(2))
#define st3 REG(st(3))
#define st4 REG(st(4))
#define st5 REG(st(5))
#define st6 REG(st(6))
#define st7 REG(st(7))

#define ax REG(ax)
#define bx REG(bx)
#define cx REG(cx)
#define dx REG(dx)

#define ah REG(ah)
#define bh REG(bh)
#define ch REG(ch)
#define dh REG(dh)

#define al REG(al)
#define bl REG(bl)
#define cl REG(cl)
#define dl REG(dl)

#define sil REG(sil)

#define mm1 REG(mm1)
#define mm2 REG(mm2)
#define mm3 REG(mm3)
#define mm4 REG(mm4)
#define mm5 REG(mm5)
#define mm6 REG(mm6)
#define mm7 REG(mm7)

#define xmm0 REG(xmm0)
#define xmm1 REG(xmm1)
#define xmm2 REG(xmm2)
#define xmm3 REG(xmm3)
#define xmm4 REG(xmm4)
#define xmm5 REG(xmm5)
#define xmm6 REG(xmm6)
#define xmm7 REG(xmm7)

#define cr0 REG(cr0)
#define cr1 REG(cr1)
#define cr2 REG(cr2)
#define cr3 REG(cr3)
#define cr4 REG(cr4)

#ifdef _I386MACH_NEED_SOTYPE_FUNCTION
#define SOTYPE_FUNCTION(sym) .type SYM(sym),@function
#else
#define SOTYPE_FUNCTION(sym)
#endif

#ifndef _I386MACH_DISABLE_HW_INTERRUPTS
#define        __CLI
#define        __STI
#else
#define __CLI  cli
#define __STI  sti
#endif
