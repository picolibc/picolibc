;int open(const char *pathname, int flags);
;Integer arguments have to be zero extended.
;The second argument is taken from the stack,
;hence it is not zero extended here.

#if defined(__H8300H__)
	.h8300h
#endif
#if defined(__H8300S__)
	.h8300s
#endif

	.section .text
	.align 2
	.global __open
__open:
	jsr @@0xc5
	rts
	.end
