multilibtool_LIBRARIES += %D%/libnsim.a
%C%_libnsim_a_SOURCES = \
	%D%/libcfunc.c \
	%D%/mcount.c \
	%D%/nsim-syscalls.c \
	%D%/sbrk.c

multilibtool_DATA += \
	%D%/crt0.o \
	%D%/gcrt0.o \
	%D%/nano.specs \
	%D%/nsim.specs
