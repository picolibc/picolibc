;off_t lseek(int fildes, off_t offset, int whence);
;Integer arguments have to be zero extended.

#if defined(__H8300H__)
	.h8300h
#endif
#if defined(__H8300S__)
	.h8300s
#endif

	.section .text
	.align 2
	.global __lseek
__lseek:
#if defined(__H8300H__) || defined(__H8300S__)
#if __INT_MAX__ == 32767
	extu.l er0
	extu.l er2
#endif
#endif
	jsr @@0xc8
	rts
	.end
