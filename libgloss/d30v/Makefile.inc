multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	isatty.c \
	print.c \
	putnum.c \
	sbrk.c \
	%D%/inbyte.c \
	%D%/outbyte.c \
	%D%/syscalls.c

multilibtool_DATA += \
	%D%/crt0.o
