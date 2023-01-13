## csky semihosting support.

multilibtool_DATA += \
	%D%/crt0.o
libobjs_a_SOURCES += \
	%D%/crt0.S

## Here's all the hosted stuff.

multilibtool_LIBRARIES += %D%/libsemi.a
%C%_libsemi_a_SOURCES = \
	%D%/io-semi.S \
	%D%/sbrk.c \
	%D%/io-gdb.c \
	%D%/io-exit.c \
	%D%/io-close.c \
	%D%/io-fstat.c \
	%D%/io-gettimeofday.c \
	%D%/io-lseek.c \
	%D%/io-read.c \
	%D%/io-system.c \
	%D%/io-unlink.c \
	%D%/io-exit.c \
	%D%/io-gdb.c \
	%D%/io-isatty.c \
	%D%/io-open.c \
	%D%/io-rename.c \
	%D%/io-stat.c \
	%D%/io-time.c \
	%D%/io-write.c \
	%D%/kill.c \
	%D%/getpid.c
%C%_libsemi_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-nostdlib \
	-DHOSTED=1
