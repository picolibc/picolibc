## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I$(srcdir)/include $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

SUBDIRS = machine \
	cmath \
	.
SUBLIBS = \
	cmath/libcmath.la \
	$(LINUX_MACH_LIB)

LIB_SOURCES = \
	aio.c \
	aio64.c \
	brk.c \
	calloc.c \
	callocr.c \
	cfreer.c \
	cfspeed.c \
	clock_getres.c \
	clock_gettime.c \
	clock_settime.c \
	confstr.c \
	ctermid.c \
	fclean.c \
	flockfile.c \
	fpathconf.c \
	free.c \
	freer.c \
	fstab.c \
	fstatvfs.c \
	fstatvfs64.c \
	ftok.c \
	ftw.c \
	ftw64.c \
	funlockfile.c \
	getdate.c \
	getdate_err.c \
	gethostname.c \
	getoptlong.c \
	getreent.c \
	getrlimit64.c \
	getwd.c \
	ids.c \
	inode.c \
	io.c \
	io64.c \
	ipc.c \
	linux.c \
	malign.c \
	malignr.c \
	mallinfor.c \
	mallocr.c \
	malloptr.c \
	mallstatsr.c \
	mmap.c \
	mntent.c \
	mntent_r.c \
	mq_close.c \
	mq_getattr.c \
	mq_notify.c \
	mq_open.c \
	mq_receive.c \
	mq_send.c \
	mq_setattr.c \
	mq_unlink.c \
	msize.c \
	msizer.c \
	mstats.c \
	mtrim.c \
	mtrimr.c \
	ntp_gettime.c \
	pathconf.c \
	pread.c \
	pread64.c \
	process.c \
	psignal.c \
	pvallocr.c \
	pwrite.c \
	pwrite64.c \
	raise.c \
	readdir64.c \
	realloc.c \
	reallocr.c \
	realpath.c \
	rename.c \
	resource.c \
	scandir64.c \
	sched.c \
	select.c \
	seteuid.c \
	sethostname.c \
	setrlimit64.c \
	shm_open.c \
	shm_unlink.c \
	sig.c \
	sigaction.c \
	sigqueue.c \
	signal.c \
	siglongjmp.c \
	sigset.c \
	sigwait.c \
	socket.c \
	sleep.c \
	stack.c \
	statvfs.c \
	statvfs64.c \
	strsignal.c \
	strverscmp.c \
	sysconf.c \
	sysctl.c \
	systat.c \
	system.c \
	tcdrain.c \
	tcsendbrk.c \
	termios.c \
	time.c \
	ttyname_r.c \
	usleep.c \
	utimes.c \
	vallocr.c \
	versionsort.c \
	versionsort64.c \
	wait.c

# This will handle both /usr/src/linux-2.4/include/asm/signal.h (in Red Hat Linux 7.1)
# and also /usr/src/linux/include/asm/signal.h in older versions of Red Hat Linux
SIGNAL_H = /usr/src/$(shell ls /usr/src/ | grep ^linux | head -n 1)/include/asm/signal.h

liblinux_la_LDFLAGS = -Xcompiler -nostdlib

ADD_OBJS = \
	malloc.$(oext) \
	mcheck.$(oext) \
	$(LINUX_MACH_LIB)

if USE_LIBTOOL
noinst_LTLIBRARIES = liblinux.la
liblinux_la_SOURCES = $(LIB_SOURCES)
liblinux_la_LIBADD = $(ADD_OBJS)
liblinux_la_DEPENDENCIES = $(ADD_OBJS)
LIBC_COMPILE = $(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_LIBADD = $(ADD_OBJS)
lib_a_DEPENDENCIES = $(ADD_OBJS)
LIBC_COMPILE = $(COMPILE)
noinst_DATA =

lib.a:  $(ADD_OBJS) $(lib_a_OBJECTS)
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

malloc.$(oext): $(srcdir)/malloc.c
	$(LIBC_COMPILE) -D_LIBC=1 -c $< -o $@

mcheck.$(oext): $(srcdir)/mcheck.c
	$(LIBC_COMPILE) -D_LIBC=1 -c $< -o $@

sig.$(oext):	siglist.inc

siglist.inc:
		$(AWK) '/#define.SIG.*[1-9][0-9]*/ { n[$$3] = $$2 } \
		  /#define.NSIG.*[1-9][0-9]*/ { max = $$3 } \
		  /#define.SIGUNUSED/ \
		    { for (i = 0; i <= max; i++) print "\"" n[i] "\"," }' \
		    < $(SIGNAL_H) > siglist.inc || \
		    { rm -f siglist.inc; exit 1; }

AM_CFLAGS = -I $(srcdir)/../../stdio
ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
