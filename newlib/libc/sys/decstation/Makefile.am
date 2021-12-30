## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

COPYOFILES = \
	read.o \
	open.o \
	_exit.o \
	write.o \
	lseek.o \
	close.o \
	sbrk.o \
	cerror.o \
	fork.o \
	isatty.o \
	ioctl.o \
	getpid.o \
	time.o \
	kill.o \
	wait.o \
	execve.o \
	gettimeofday.o \
	link.o \
	unlink.o \
	fstat.o \
	times.o  \
	getrusage.o  

lib_a_SOURCES = _main.c
lib_a_LIBADD = $(COPYOFILES)

$(COPYOFILES):
	ar x /lib/libc.a $(COPYOFILES) 

all-local: crt0.o

crt0.o:
	cp /lib/crt0.o crt0.o

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../.. -I ../../../../config
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
