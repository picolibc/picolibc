## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	_exit.s		\
	_main.c		\
	abort.c		\
	access.c	\
	bdos.c		\
	bdosptr.c	\
	brk.s		\
	chdir.s		\
	chmod.c		\
	chown.c		\
	clock.S		\
	close.s		\
	creat.s		\
	crlf2nl.c	\
	dir.c		\
	dosmem.S 	\
	dpmi.c          \
	dup.s		\
	dup2.s		\
	exec.c          \
	fchmod.c	\
	findfirs.s	\
	findnext.s	\
	fixpath.c	\
	fstat.s		\
	fsync.s		\
	ftruncat.c	\
	gerrno.s	\
	getcwd.c	\
	getdate.c 	\
	getdtabl.c	\
	getgid.c	\
	getkey.s	\
	getpages.c	\
	getpid.c	\
	getrusag.c	\
	gettime.c	\
	gettimeo.c	\
	getuid.c	\
	getwd.s		\
	go32func.c      \
	infoblk.c	\
	inportb.s	\
	inportl.s	\
	inportsb.s	\
	inportsl.s	\
	inportsw.s	\
	inportw.s	\
	int86x.s	\
	intdos.c	\
	intdosx.c	\
	isatty.s	\
	kbhit.s		\
	kill.c		\
	link.s		\
	longjmp.S	\
	lseek.s		\
	lstat.s		\
	mkdir.s		\
	open.s		\
	outportb.s	\
	outportl.s	\
	outportw.s	\
	outprtsb.s	\
	outprtsl.s	\
	outprtsw.s	\
	read.s		\
	readv.c		\
	rename.s	\
	rmdir.s		\
	sbrk.s		\
	screen.S	\
	setjmp.S	\
	setmode.s	\
	setstack.S 	\
	settimeo.s	\
	sleep.c		\
	stat.c		\
	stat_ast.s  	\
	syserr.c	\
	system.s	\
	tell.s		\
	time.c		\
	truncate.c	\
	turbo.s		\
	umask.c		\
	unlink.s	\
	utime.c		\
	utimes.c	\
	write.s		\
	writev.c

all: crt0.o

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
