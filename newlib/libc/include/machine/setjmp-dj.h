/*
** Copyright (C) 1991 DJ Delorie, 24 Kirsten Ave, Rochester NH 03867-2954
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* Modified to use SETJMP_DJ_H rather than SETJMP_H to avoid
   conflicting with setjmp.h.  Ian Taylor, Cygnus support, April,
   1993.  */

#ifndef _SETJMP_DJ_H_
#define _SETJMP_DJ_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned long eax;
  unsigned long ebx;
  unsigned long ecx;
  unsigned long edx;
  unsigned long esi;
  unsigned long edi;
  unsigned long ebp;
  unsigned long esp;
  unsigned long eip;
} jmp_buf[1];

extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);

#ifdef __cplusplus
}
#endif

#endif
