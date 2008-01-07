#include "m68kasm.h"

.global SYM (setjmp)
.global SYM (longjmp)

SYM (setjmp):
	moveal sp@(4),a0
	movel sp@(0),a0@(12)
	movel sp,a0@(8)
	moveml d2-d7/a2-a6,a0@(20)
	clrl d0
	rts

SYM (longjmp):
	moveal sp@(4),a0
	movel sp@(8),d0
	bne 1f
	movel &1,d0
1:
	moveml a0@(20),d2-d7/a2-a6
	moveal a0@(8),sp
	movel a0@(12),sp@
	rts

#ifdef M68881
.global SYM (setjmp_68881)
.global SYM (longjmp_68881)

SYM (setjmp_68881):
	moveal sp@(4),a0
	movel sp@(0),a0@(12)
	movel sp,a0@(8)
	moveml d2-d7/a2-a6,a0@(20)
	fmovemx fp2-fp7,a0@(64)
	clrl d0
	rts

SYM (longjmp_68881):
	moveal sp@(4),a0
	fmovemx a0@(64),fp2-fp7
	movel sp@(8),d0
	bne 1f
	movel &1,d0
1:
	moveml a0@(20),d2-d7/a2-a6
	moveal a0@(8),sp
	movel a0@(12),sp@
	rts
#endif
