/* Hexagon implementation of setjmp */

	.text
	.p2align 5
	.globl longjmp
	.type  longjmp, @function
longjmp:
    r16=memw(r0++#4)
    r17=memw(r0++#4)
    r18=memw(r0++#4)
    r19=memw(r0++#4)
    r20=memw(r0++#4)
    r21=memw(r0++#4)
    r22=memw(r0++#4)
    r23=memw(r0++#4)
    r24=memw(r0++#4)
    r25=memw(r0++#4)
    r26=memw(r0++#4)
    r27=memw(r0++#4)
    r29=memw(r0++#4)
    r30=memw(r0++#4)
    r31=memw(r0++#4)

    r0 = r1
    r1 = #0
    p0 = cmp.eq(r0,r1)
    if (!p0) jumpr r31 /* not 0, return */
    r0 = #1 /* oops, it was 0 */

    jumpr r31
        .size longjmp, . - longjmp 
