## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	_alloc.s	\
	_close.s	\
	_cycles.s	\
	_exit.s		\
	_getpsiz.s	\
	_gettz.s	\
	_ioctl.s	\
	_iostat.s	\
	_iowait.s	\
	_lseek.s	\
	_open.s		\
	_query.s	\
	_read.s		\
	_setim.s	\
	_settrap.s	\
	_setvec.s	\
	_tmpnam.s	\
	_write.s	\
	alloc.s		\
	clock.s		\
	getargs.s	\
	getenv.s	\
	read.s		\
	remove.s	\
	rename.s	\
	signal.s	\
	_sbrk.s 	\
	_fstat.s 	\
	_isatty.s 	\
	systime.s	\
	vec.s		\
	kill.c		\
	getpid.c	\
	stubs.s

all: crt0.o

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
