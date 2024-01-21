multilibtool_DATA += \
	%D%/crti.o \
	%D%/crtn.o \
	%D%/crt1.o
libobjs_a_SOURCES += \
	%D%/crti.S \
	%D%/crtn.S \
	%D%/crt1.S

multilibtool_LIBRARIES += %D%/libh.a
%C%_libh_a_SOURCES = \
	%D%/intable.c \
	%D%/dvz_hndl.c \
	%D%/flg_hndl.c \
	%D%/iad_hndl.c \
	%D%/svc_hndl.c \
	%D%/und_hndl.c

multilibtool_DATA += \
	%D%/sim.ld

multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	%D%/_exit.c \
	%D%/close.c \
	%D%/_getenv.c \
	%D%/lseek.c \
	%D%/open.c \
	%D%/read.c \
	%D%/_rename.c \
	%D%/sbrk.c \
	%D%/time.c \
	%D%/unlink.c \
	%D%/write.c \
	%D%/stat.c \
	%D%/putnum.c \
	%D%/kill.c \
	%D%/getpid.c \
	%D%/isatty.c \
	%D%/fstat.c
