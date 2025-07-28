/* Hexagon implementation of setjmp */

	.text
	.p2align 5
	.globl setjmp
	.type  setjmp, @function

setjmp:
 /* Save all the callee-preserved registers */
 /* r0 has jmp_buf */
    memw(r0++#4)=r16
    memw(r0++#4)=r17
    memw(r0++#4)=r18
    memw(r0++#4)=r19
    memw(r0++#4)=r20
    memw(r0++#4)=r21
    memw(r0++#4)=r22
    memw(r0++#4)=r23
    memw(r0++#4)=r24
    memw(r0++#4)=r25
    memw(r0++#4)=r26
    memw(r0++#4)=r27
    memw(r0++#4)=r29
    memw(r0++#4)=r30
    memw(r0++#4)=r31
 
    r0 = #0
    jumpr r31
        .size setjmp, . - setjmp 
