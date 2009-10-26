	.file	"memchr.S"

	.section .text
	
	.global  _memchr
	.type	 _memchr,@function
_memchr:
	cmp	#0, r3		; If r3 is 0 suntil.b will do nothing and not set any flags...
	stz     #1, r1		; ...so store 1 into r1.  It will be decremented by the SUB later.
	suntil.b    		; Search until *r1 == r2 or r3 bytes have been examined.
	stnz	#1, r1		; If no match was found return NULL.
	sub	#1, r1		; suntil.b leaves r1 pointing at the address *after* the match.
	rts
