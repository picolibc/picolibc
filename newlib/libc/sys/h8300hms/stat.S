;int stat(const char *file_name, struct stat *buf);

#if defined(__H8300H__)
	.h8300h
#endif
#if defined(__H8300S__)
	.h8300s
#endif

	.section .text
	.align 2
	.global __stat
__stat:
	jsr @@0xca
	rts
	.end
									
