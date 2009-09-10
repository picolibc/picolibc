/* Copyright (C) 2006 Free Software Foundation, Inc.
   This file is free software; you can redistribute it and/or modify
   it under the same terms as newlib/libc/string/memcpy.c .  */

#include "asm.h"

#ifdef __A4__
ENTRY(memcpy)
	
;;;  if count of copies is zero
	mov.f  lp_count,r2
	jz.d   [blink]
	or	r4,r0,r1

	and.f   0,r4,3 		;  check if 4 byte aligned
#ifndef __A4__ 
	bnz.d	@copy_bytewise
	lsr	r4,r2,3		;  calculate length in words
#else
	bnz.d	@copy_bytewise
	lsr	r4,r2
	lsr	r4,r4
	lsr	r4,r4
#endif	

	;;  Now both pointers are
	;;  4 bytes aligned.
	;;
	lsr.f	lp_count,r4
	
	sub	r1,r1,4
	sub	r3,r0,4
	lpnz	@copy16bytes
	ld.a	r4,[r1,4]
	ld.a	r5,[r1,4]
	ld.a	r6,[r1,4]
	ld.a	r7,[r1,4]
	st.a	r4,[r3,4]
	st.a	r5,[r3,4]
	st.a	r6,[r3,4]
	st.a	r7,[r3,4]
copy16bytes:		
	
	bcs.d	@remainder8bytes
	and.f	0,r2,4
#ifdef __A4__
	nop
#endif
	bnz.d	@copy4bytes
	
memcpytill1:
	and.f	lp_count, r2, 3
memcpy1:		
	add	r1,r1,3
	jz	[blink]
	add	r3,r3,3
	lp	@end_loop
	ldb.a	r4,[r1,1]
	stb.a	r4,[r3,1]
end_loop:	
	j	[blink]	
			
remainder8bytes:
	ld.a	r4,[r1,4]
	ld.a	r5,[r1,4]
	st.a	r4,[r3,4]
	bz.d	@memcpytill1
	st.a	r5,[r3,4]
	and.f	lp_count,r2,3
	
copy4bytes:	
	ld.a	r4,[r1,4]
	b.d	@memcpy1
	st.a	r4,[r3,4]

copy_bytewise:
	sub	r1,r1,1
	sub	r3,r0,1
	lp	@.end_bytes
	ldb.a	r4,[r1,1]
	stb.a	r4,[r3,1]
.end_bytes:
	j	[blink]
ENDFUNC(memcpy)
#endif /* __A4__ */
