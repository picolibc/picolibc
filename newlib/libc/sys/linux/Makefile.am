## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	brk.c getoptlong.c ids.c inode.c io.c linux.c process.c realpath.c \
	select.c signal.c systat.c termios.c time.c wait.c

# This will handle both /usr/src/linux-2.4/include/asm/signal.h (in Red Hat Linux 7.1)
# and also /usr/src/linux/include/asm/signal.h in older versions of Red Hat Linux
SIGNAL_H = /usr/src/$(shell ls /usr/src/ | grep ^linux | head -n 1)/include/asm/signal.h

liblinux_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = liblinux.la
liblinux_la_SOURCES = $(LIB_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../../Makefile.shared

all: crt0.o

signal.$(oext):	siglist.inc

siglist.inc:
		$(AWK) '/#define.SIG.*[1-9][0-9]*/ { n[$$3] = $$2 } \
		  /#define.NSIG.*[1-9][0-9]*/ { max = $$3 } \
		  /#define.SIGUNUSED/ \
		    { for (i = 0; i <= max; i++) print "\"" n[i] "\"," }' \
		    < $(SIGNAL_H) > siglist.inc || \
		    { rm -f siglist.inc; exit 1; }

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
