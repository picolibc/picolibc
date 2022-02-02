multilibtool_DATA += %D%/crt0.o %D%/sim.ld
multilibtool_LIBRARIES += %D%/libeval.a
libobjs_a_SOURCES += %D%/crt0.S %D%/test.o

%C%_libeval_a_SOURCES = \
	%D%/_exit.c %D%/access.c %D%/chmod.c %D%/close.c %D%/crt1.c \
	%D%/fstat.c %D%/getpid.c %D%/gettime.c %D%/isatty.c \
	%D%/kill.c %D%/lseek.c %D%/open.c %D%/read.c \
	%D%/sbrk.c %D%/stat.c %D%/time.c %D%/trap.c %D%/unlink.c %D%/utime.c %D%/write.c

check_PROGRAMS += %D%/test
