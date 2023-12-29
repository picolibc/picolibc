multilibtool_DATA += %D%/crt0.o
libobjs_a_SOURCES += %D%/crt0.s

multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	%D%/syscalls.c \
	putnum.c \
	unlink.c
