## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I$(srcdir)/../include -I$(srcdir)/.. $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = addr2ascii.c ascii2addr.c alias-lookup.c \
	base64.c check_pf.c digits_dots.c \
	ether_aton.c ether_aton_r.c ether_hton.c \
	ether_line.c ether_ntoa.c ether_ntoa_r.c ether_ntoh.c \
	ethers-lookup.c getaddrinfo.c getaliasent.c \
	getaliasent_r.c getaliasname.c getaliasname_r.c gethstbyad.c \
	gethstbyad_r.c gethstbynm2.c gethstbynm2_r.c gethstbynm.c \
	gethstbynm_r.c gethstent.c gethstent_r.c getnameinfo.c getnetbyad.c \
	getnetbyad_r.c getnetbynm.c getnetbynm_r.c getnetent.c getnetent_r.c \
	getnetgrent.c getnetgrent_r.c getnssent.c getnssent_r.c getproto.c \
	getproto_r.c getprtent.c \
	getprtent_r.c getprtname.c getprtname_r.c getrpcbyname.c \
	getrpcbyname_r.c getrpcbynumber.c getrpcbynumber_r.c getrpcent.c \
	getrpcent_r.c getservent.c getservent_r.c getsrvbynm.c getsrvbynm_r.c \
	getsrvbypt.c getsrvbypt_r.c grp-lookup.c \
	herrno.c hosts-lookup.c ifreq.c \
	in6_addr.c inet6_option.c inet_addr.c \
	inet_lnaof.c inet_mkadr.c inet_net.c inet_neta.c inet_netof.c \
	inet_net_ntop.c inet_net_pton.c inet_ntoa.c inet_ntop.c \
	inet_pton.c issetugid-stub.c key-lookup.c netgrp-lookup.c \
	network-lookup.c nsswitch.c \
	ns_name.c ns_netint.c ns_parse.c ns_print.c ns_samedomain.c \
	ns_ttl.c nsap_addr.c proto-lookup.c opensock.c pwd-lookup.c recv.c \
	res_comp.c res_data.c res_debug.c res_hconf.c res_init.c \
	res_libc.c res_mkquery.c \
	res_query.c res_send.c \
	rexec.c rpc-lookup.c ruserpass.c send.c service-lookup.c spwd-lookup.c 

ELIX_4_SOURCES = \
	ifname.c \
	rcmd.$(oext) \
	rcmdsh.$(oext)

if ELIX_LEVEL_1
ELIX_SOURCES =
else
if ELIX_LEVEL_2
ELIX_SOURCES =
else
if ELIX_LEVEL_3
ELIX_SOURCES =
else
ELIX_SOURCES = $(ELIX_4_SOURCES)
endif
endif
endif

libnet_la_LDFLAGS = -Xcompiler -nostdlib
libnet_la_CFLAGS = -DINET6

if USE_LIBTOOL
noinst_LTLIBRARIES = libnet.la
libnet_la_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../../../Makefile.shared

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/arpa; \
	for i in $(srcdir)/../include/arpa/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/arpa/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/net; \
	for i in $(srcdir)/../include/net/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/net/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netinet; \
	for i in $(srcdir)/../include/netinet/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netinet/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netinet6; \
	for i in $(srcdir)/../include/netinet6/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netinet6/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netns; \
	for i in $(srcdir)/../include/netns/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netns/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/rpc; \
	for i in $(srcdir)/../include/rpc/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/rpc/`basename $$i`; \
	done;
