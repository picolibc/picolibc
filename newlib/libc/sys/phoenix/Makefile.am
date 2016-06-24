## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I$(srcdir)/include $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS) $(INCLTDL)

SUBDIRS = machine

SUBLIBS = \
	$(PHOENIX_MACH_LIB)

SOURCES = \
	net/domainname.c \
	net/freeaddrinfo.c \
	net/getaddrinfo.c \
	net/gethostbyaddr.c \
	net/gethostbyname.c \
	net/getnameinfo.c \
	net/getservbyname.c \
	net/getservbyport.c \
	net/herror.c \
	net/hostname.c \
	net/inet_addr.c \
	net/inet_aton.c \
	net/inet_lnaof.c \
	net/inet_makeaddr.c \
	net/inet_net_ntop.c \
	net/inet_net_pton.c \
	net/inet_netof.c \
	net/inet_network.c \
	net/inet_ntoa.c \
	net/inet_ntop.c \
	net/inet_pton.c \
	net/nametoindex.c \
	net/network.c \
	alarm.c \
	chmod.c \
	chown.c \
	clocks.c \
	dup.c \
	endmntent.c \
	exec.c \
	fcntl.c \
	fork.c \
	fs.c \
	getentropy.c \
	getmntent.c \
	getpagesize.c \
	groups.c \
	ids.c \
	io.c \
	ioctl.c \
	mmap.c \
	mount.c \
	nanosleep.c \
	phoenix.c \
	pid.c \
	pipe.c \
	poll.c \
	realpath.c \
	reboot.c \
	resource.c \
	sbrk.c \
	sched.c \
	select.c \
	semaphore.c \
	setmntent.c \
	shm.c \
	signal.c \
	socket.c \
	sockopt.c \
	stat.c \
	statfs.c \
	symlink.c \
	sync.c \
	sysconf.c \
	syslog.c \
	time.c \
	truncate.c \
	tty.c \
	uname.c \
	utime.c \
	wait.c

ADD_OBJS =

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(SOURCES)
lib_a_LIBADD = $(ADD_OBJS)
lib_a_DEPENDENCIES = $(ADD_OBJS)
lib_a_CFLAGS = $(AM_CFLAGS)
LIBC_COMPILE = $(COMPILE)
noinst_DATA =

lib.a:  $(lib_a_OBJECTS)
	rm -f $@
	rm -rf tmp
	mkdir tmp
	cd tmp; \
	for i in $(SUBLIBS); do \
	  $(AR) x ../$$i; \
	done;
	$(AR) $(AR_FLAGS) $@ tmp/*.o  $^
	$(RANLIB) $@
	rm -rf tmp

all-local: crt0.o

crt0.o: machine/crt0.o
	rm -f $@
	ln machine/crt0.o $@ >/dev/null 2>/dev/null || cp machine/crt0.o $@

machine/crt0.o: ; @true

AM_CFLAGS = -I $(srcdir)/../../stdio
ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/arpa; \
	for i in $(srcdir)/include/arpa/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/arpa/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/net; \
	for i in $(srcdir)/include/net/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/net/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netinet; \
	for i in $(srcdir)/include/netinet/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netinet/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netinet6; \
	for i in $(srcdir)/include/netinet6/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netinet6/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netpacket; \
	for i in $(srcdir)/include/netpacket/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netpacket/`basename $$i`; \
	done;
