## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

SUBDIRS = machine \
	.
SUBLIBS = \
	$(LINUX_MACH_LIB)

LIB_SOURCES = \
	brk.c cfspeed.c flockfile.c funlockfile.c gethostname.c getoptlong.c \
	getreent.c ids.c inode.c io.c io64.c linux.c mmap.c \
	pread.c pread64.c process.c pwrite.c pwrite64.c raise.c realpath.c \
	rename.c resource.c sched.c select.c seteuid.c shm_open.c shm_unlink.c \
	sig.c sigaction.c sigqueue.c signal.c siglongjmp.c sigset.c sigwait.c \
	socket.c sleep.c stack.c sysconf.c sysctl.c systat.c system.c \
	tcdrain.c tcsendbrk.c termios.c time.c usleep.c wait.c

# This will handle both /usr/src/linux-2.4/include/asm/signal.h (in Red Hat Linux 7.1)
# and also /usr/src/linux/include/asm/signal.h in older versions of Red Hat Linux
SIGNAL_H = /usr/src/$(shell ls /usr/src/ | grep ^linux | head -n 1)/include/asm/signal.h

liblinux_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = liblinux.la
liblinux_la_SOURCES = $(LIB_SOURCES)
liblinux_la_LIBADD = $(LINUX_MACH_LIB)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_LIBADD = $(LINUX_MACH_LIB)
lib_a_DEPENDENCIES = $(LINUX_MACH_LIB)
noinst_DATA =

lib.a:  $(LINUX_MACH_LIB) $(lib_a_OBJECTS)
	rm -f $@
	rm -rf tmp
	mkdir tmp
	cd tmp; \
	for i in $(SUBLIBS); do \
	  $(AR) x ../$$i; \
	done; \
	$(AR) $(AR_FLAGS) ../$@ *.o
	$(RANLIB) $@
	rm -rf tmp

endif # USE_LIBTOOL


include $(srcdir)/../../../Makefile.shared

all: crt0.o crt1.o

crt0.o: machine/crt0.o
	rm -f $@
	ln machine/crt0.o $@ >/dev/null 2>/dev/null || cp machine/crt0.o $@

machine/crt0.o: ; @true


sig.$(oext):	siglist.inc

siglist.inc:
		$(AWK) '/#define.SIG.*[1-9][0-9]*/ { n[$$3] = $$2 } \
		  /#define.NSIG.*[1-9][0-9]*/ { max = $$3 } \
		  /#define.SIGUNUSED/ \
		    { for (i = 0; i <= max; i++) print "\"" n[i] "\"," }' \
		    < $(SIGNAL_H) > siglist.inc || \
		    { rm -f siglist.inc; exit 1; }

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
