libc_a_SOURCES += \
	%D%/net/domainname.c \
	%D%/net/freeaddrinfo.c \
	%D%/net/getaddrinfo.c \
	%D%/net/gethostbyaddr.c \
	%D%/net/gethostbyname.c \
	%D%/net/getnameinfo.c \
	%D%/net/getservbyname.c \
	%D%/net/getservbyport.c \
	%D%/net/herror.c \
	%D%/net/hostname.c \
	%D%/net/inet_addr.c \
	%D%/net/inet_aton.c \
	%D%/net/inet_lnaof.c \
	%D%/net/inet_makeaddr.c \
	%D%/net/inet_net_ntop.c \
	%D%/net/inet_net_pton.c \
	%D%/net/inet_netof.c \
	%D%/net/inet_network.c \
	%D%/net/inet_ntoa.c \
	%D%/net/inet_ntop.c \
	%D%/net/inet_pton.c \
	%D%/net/nametoindex.c \
	%D%/net/network.c \
	%D%/alarm.c \
	%D%/chmod.c \
	%D%/chown.c \
	%D%/clocks.c \
	%D%/dup.c \
	%D%/endmntent.c \
	%D%/exec.c \
	%D%/fcntl.c \
	%D%/fork.c \
	%D%/fs.c \
	%D%/getentropy.c \
	%D%/getmntent.c \
	%D%/getpagesize.c \
	%D%/groups.c \
	%D%/ids.c \
	%D%/io.c \
	%D%/ioctl.c \
	%D%/mmap.c \
	%D%/mount.c \
	%D%/nanosleep.c \
	%D%/phoenix.c \
	%D%/pid.c \
	%D%/pipe.c \
	%D%/poll.c \
	%D%/realpath.c \
	%D%/reboot.c \
	%D%/resource.c \
	%D%/sbrk.c \
	%D%/sched.c \
	%D%/select.c \
	%D%/semaphore.c \
	%D%/setmntent.c \
	%D%/shm.c \
	%D%/signal.c \
	%D%/socket.c \
	%D%/sockopt.c \
	%D%/stat.c \
	%D%/statfs.c \
	%D%/symlink.c \
	%D%/sync.c \
	%D%/sysconf.c \
	%D%/syslog.c \
	%D%/time.c \
	%D%/truncate.c \
	%D%/tty.c \
	%D%/uname.c \
	%D%/utime.c \
	%D%/wait.c

libc_a_CPPFLAGS_%C%_net = -I$(srcdir)/%D%

%D%/crt0.o: %D%/$(PHOENIX_MACHINE_DIR)/crt0.o
	$(AM_V_at)rm -f $@
	$(AM_V_GEN)ln $< $@ >/dev/null 2>/dev/null || cp $< $@

INSTALL_DATA_LOCAL += install-data-local-%C%
install-data-local-%C%:
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/arpa; \
	for i in $(srcdir)/%D%/include/arpa/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/arpa/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/net; \
	for i in $(srcdir)/%D%/include/net/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/net/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netinet; \
	for i in $(srcdir)/%D%/include/netinet/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netinet/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netinet6; \
	for i in $(srcdir)/%D%/include/netinet6/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netinet6/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netpacket; \
	for i in $(srcdir)/%D%/include/netpacket/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netpacket/`basename $$i`; \
	done;

if HAVE_LIBC_SYS_PHOENIX_ARM_DIR
include %D%/machine/arm/Makefile.inc
endif
