if MACH_ADD_SETJMP
libc_a_SOURCES += %D%/setjmp.S
endif

libc_a_SOURCES += \
	%D%/memchr.S %D%/memcmp.S %D%/memcpy.S %D%/memset.S %D%/strchr.S \
	%D%/memmove.S %D%/strlen.S %D%/i386mach.h
