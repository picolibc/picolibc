!
! Fast SH memset
!
! by Toshiyasu Morita (tm@netcom.com)
!
! Entry: r4: destination pointer
!        r5: fill value
!        r6: byte count
!
! Exit:  r0-r3: trashed
!

! This assumes that the first four bytes of the address space (0..3) are
! reserved - usually by the linker script.  Otherwise, we would had to check
! for the case of objects of the size 12..15 at address 0..3 .

#include "asm.h"

ENTRY(memset)
	mov	#12,r0	! Check for small number of bytes
	cmp/gt	r6,r0
	mov	r4,r0
	SL(bt, L_store_byte_loop_check0, add r4,r6)

	tst	#3,r0	! Align destination
	SL(bt,	L_dup_bytes, extu.b r5,r5)
	.balignw 4,0x0009
L_align_loop:
	mov.b	r5,@r0
	add	#1,r0
	tst	#3,r0
	bf	L_align_loop

L_dup_bytes:	
	swap.b	r5,r2	! Duplicate bytes across longword
	or	r2,r5
	swap.w	r5,r2
	or	r2,r5

	add	#-16,r6

	.balignw 4,0x0009
L_store_long_loop:
	mov.l	r5,@r0	! Store double longs to memory
	cmp/hs	r6,r0
	mov.l	r5,@(4,r0)
	SL(bf, L_store_long_loop, add #8,r0)

	add	#16,r6

L_store_byte_loop_check0:
	cmp/eq	r6,r0
	bt	L_exit
	.balignw 4,0x0009
L_store_byte_loop:
	mov.b	r5,@r0	! Store bytes to memory
	add	#1,r0
	cmp/eq	r6,r0
	bf	L_store_byte_loop

L_exit:
	rts
	mov	r4,r0

