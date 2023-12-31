multilibtool_DATA += %D%/crt0.o
libobjs_a_SOURCES += %D%/crt0.S

multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	%D%/sbrk.c \
	putnum.c \
	%D%/getpid.c \
	%D%/kill.c \
	%D%/syscalls.c
