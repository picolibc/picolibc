libc_a_CCASFLAGS_%C% = -Wa,--gdwarf-2

libc_a_SOURCES += \
	%D%/setjmp.S %D%/memchr.S %D%/memcmp.S %D%/memcpy.S %D%/memset.S %D%/strchr.S \
	%D%/strcmp.S %D%/strcpy.S %D%/strlen.S
