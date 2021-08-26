# Makefile for Cygwin server

# This file is part of Cygwin.

# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

include $(top_srcdir)/Makefile.am.common

cygserver_flags=$(cxxflags_common) -Wimplicit-fallthrough=5 -Werror -DSYSCONFDIR="\"$(sysconfdir)\""
AM_CXXFLAGS = $(CFLAGS)

noinst_LIBRARIES = libcygserver.a
sbin_PROGRAMS = cygserver
bin_SCRIPTS = cygserver-config

cygserver_SOURCES = \
	bsd_helper.cc \
	bsd_log.cc \
	bsd_mutex.cc \
	client.cc \
	cygserver.cc \
	msg.cc \
	process.cc \
	pwdgrp.cc \
	sem.cc \
	setpwd.cc \
	shm.cc \
	sysv_msg.cc \
	sysv_sem.cc \
	sysv_shm.cc \
	threaded_queue.cc \
	transport.cc \
	transport_pipes.cc

cygserver_CXXFLAGS = $(cygserver_flags) -D__OUTSIDE_CYGWIN__
cygserver_LDADD = -lntdll
cygserver_LDFLAGS = -static -static-libgcc

# Note: the objects in libcygserver are built without -D__OUTSIDE_CYGWIN__,
# unlike cygserver.exe

libcygserver_a_SOURCES = \
	$(cygserver_SOURCES)

libcygserver_a_CXXFLAGS = $(cygserver_flags)

cygdocdir = $(datarootdir)/doc/Cygwin

install-data-local:
	@$(MKDIR_P) $(DESTDIR)$(cygdocdir)
	$(INSTALL_DATA) $(srcdir)/README $(DESTDIR)$(cygdocdir)/cygserver.README
	@$(MKDIR_P) $(DESTDIR)$(sysconfdir)/defaults/etc
	$(INSTALL_DATA) $(srcdir)/cygserver.conf $(DESTDIR)$(sysconfdir)/defaults/etc/cygserver.conf

uninstall-local:
	rm -f $(DESTDIR)$(cygdocdir)/cygserver.README
	rm -f $(DESTDIR)$(sysconfdir)/defaults/etc/cygserver.conf
