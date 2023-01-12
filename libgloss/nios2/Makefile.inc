## Linker scripts.

multilibtool_DATA += \
	%D%/qemu-hosted.ld

## Semihosting library.

multilibtool_LIBRARIES += %D%/libnios2.a
%C%_libnios2_a_SOURCES = \
	%D%/crt0.S \
	%D%/io-close.c \
	%D%/io-exit.c \
	%D%/io-fstat.c \
	%D%/io-gdb.c \
	%D%/io-gettimeofday.c \
	%D%/io-isatty.c \
	%D%/io-lseek.c \
	%D%/io-nios2.S \
	%D%/io-open.c \
	%D%/io-read.c \
	%D%/io-rename.c \
	%D%/io-stat.c \
	%D%/io-system.c \
	%D%/io-time.c \
	%D%/io-unlink.c \
	%D%/io-write.c \
	%D%/getpid.c \
	%D%/kill.c \
	%D%/sbrk.c
%C%_libnios2_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DHOSTED=1
