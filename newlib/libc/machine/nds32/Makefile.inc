libc_a_SOURCES += \
	%D%/abort.c \
	%D%/setjmp.S \
	%D%/strcmp.S \
	%D%/strcpy.S
if !IS_NDS32_ISA_V3M
libc_a_SOURCES += %D%/memcpy.S %D%/memset.S
endif
