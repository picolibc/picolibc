 ##############################################################################
 # setjmp.S -- CR16 setjmp routine                                            #
 #                                                                            #
 # Copyright (c) 2004 National Semiconductor Corporation                      #
 #                                                                            #
 # The authors hereby grant permission to use, copy, modify, distribute,      #
 # and license this software and its documentation for any purpose, provided  #
 # that existing copyright notices are retained in all copies and that this   #
 # notice is included verbatim in any distributions. No written agreement,    #
 # license, or royalty fee is required for any of the authorized uses.        #
 # Modifications to this software may be copyrighted by their authors         #
 # and need not follow the licensing terms described here, provided that      #
 # the new terms are clearly indicated on the first page of each file where   #
 # they apply.                                                                #
 #                                                                            #
 # C library -- setjmp, longjmp                                               #
 # longjmp(a,v)                                                               #
 # will generate a "return(v)"                                                #
 # from the last call to                                                      #
 # setjmp(a)                                                                  #
 # by restoring r7-ra, sp,                                                    #
 # and pc from 'a'                                                            #
 # and doing a return. (Makes sure that longjmp never returns 0).             #
 ##############################################################################

	.text
	.file	"setjmp.s"
        .align 4

        .globl _setjmp
        .align 4
_setjmp:
        #r3, r2: .blkw
	storw r7, 0(r3,r2)
	addd $2, (r3,r2)
	storw r8, 0(r3,r2)
	addd $2, (r3,r2)
	storw r9, 0(r3,r2)
	addd $2, (r3,r2)
	storw r10, 0(r3,r2)
	addd $2, (r3,r2)
	storw r11, 0(r3,r2)
	addd $2, (r3,r2)

	stord (r12), 0(r3,r2)
	addd $4, (r3,r2)
	stord (r13), 0(r3,r2)
	addd $4, (r3,r2)

	stord (ra), 0(r3,r2)
	addd $4, (r3,r2)
	stord (sp), 0(r3,r2)

        movd  $0,(r1,r0)
        jump  (ra)

        .globl _longjmp
_longjmp:
        #r3, r2: .blkw # pointer save area
        #r5, r4: .blkw # ret vlaue

	loadw 0(r3,r2), r7
	addd $2, (r3,r2)
	loadw 0(r3,r2), r8
	addd $2, (r3,r2)
	loadw 0(r3,r2), r9
	addd $2, (r3,r2)
	loadw 0(r3,r2), r10
	addd $2, (r3,r2)
	loadw 0(r3,r2), r11
	addd $2, (r3,r2)

	loadd 0(r3,r2), (r12)
	addd $4, (r3,r2)
	loadd 0(r3,r2), (r13)
	addd $4, (r3,r2)

	loadd 0(r3,r2), (ra)
	addd $4, (r3,r2)
	loadd 0(r3,r2), (sp)

#ifdef __INT32__
        movd (r5,r4), (r1,r0)
        cmpd $0, (r5,r4)
        bne end1
        movd $1, (r1,r0)
#else
        movw r4, r0
        cmpw $0, r4
        bne end1
        movw $1, r0
#endif
end1:
        jump (ra)
        .align 4


