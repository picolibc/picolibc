## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	brk.c getoptlong.c ids.c inode.c io.c linux.c process.c realpath.c \
	select.c signal.c systat.c termios.c time.c wait.c

all: crt0.o

signal.o:	siglist.inc

siglist.inc:
		awk '/#define.SIG.*[1-9][0-9]*/ { n[$$3] = $$2 } \
		  /#define.NSIG.*[1-9][0-9]*/ { max = $$3 } \
		  /#define.SIGUNUSED/ \
		    { for (i = 0; i <= max; i++) print "\"" n[i] "\"," }' \
		    </usr/src/linux/include/asm/signal.h >siglist.inc || \
		    { rm -f siglist.inc; exit 1; }

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
