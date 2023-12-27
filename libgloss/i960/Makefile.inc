multilibtool_DATA += \
	%D%/crt0.o
libobjs_a_SOURCES += \
	%D%/crt0.c

multilibtool_DATA += \
	%D%/mon960.ld

multilibtool_LIBRARIES += %D%/libmon960.a
%C%_libmon960_a_SOURCES = \
	putnum.c \
	unlink.c \
	%D%/mon-read.c \
	%D%/mon-write.c \
	%D%/mon-syscalls.S \
	%D%/mon960.c
