# Makefile for Cygwin utilities

# This file is part of Cygwin.

# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

include $(top_srcdir)/Makefile.am.common

CFLAGS_COMMON=-Wimplicit-fallthrough=4 -Werror
AM_CFLAGS=$(cflags_common) $(CFLAGS_COMMON)
AM_CXXFLAGS=$(cxxflags_common) $(CFLAGS_COMMON)

bin_PROGRAMS = \
	chattr \
	cygpath \
	gencat \
	getconf \
	getfacl \
	kill \
	ldd \
	locale \
	lsattr \
	minidumper \
	mkgroup \
	mkpasswd \
	mount \
	passwd \
	pldd \
	ps \
	regtool \
	setfacl \
	setmetamode \
	ssp \
	tzset \
	umount

# dumper is only built if libbfd.a available
if BUILD_DUMPER
bin_PROGRAMS += dumper
endif

# If prog_SOURCES is not specified, automake defaults to the single file prog.c
cygpath_SOURCES = cygpath.cc
dumper_SOURCES = dumper.cc module_info.cc
kill_SOURCES = kill.cc
ldd_SOURCES = ldd.cc
locale_SOURCES = locale.cc
minidumper_SOURCES = minidumper.cc
mount_SOURCES = mount.cc path.cc
ps_SOURCES = ps.cc
regtool_SOURCES = regtool.cc
umount_SOURCES = umount.cc

# rules to create/update tzmap.h from an online resource
.PHONY: tzmap
tzmap:
	$(srcdir)/tzmap-from-unicode.org > $(srcdir)/$@.h

tzmap.h:
	$(srcdir)/tzmap-from-unicode.org > $(srcdir)/$@

BUILT_SOURCES = tzmap.h

AM_LDFLAGS = -static -Wl,--enable-auto-import
LDADD = -lnetapi32

cygpath_CXXFLAGS = -fno-threadsafe-statics $(AM_CXXFLAGS)
cygpath_LDADD = $(LDADD) -luserenv -lntdll
dumper_CXXFLAGS = -I$(top_srcdir)/../include $(AM_CXXFLAGS)
dumper_LDADD = $(LDADD) -lpsapi -lbfd -lintl -liconv -liberty -lz -lntdll
ldd_LDADD = $(LDADD) -lpsapi -lntdll
mount_CXXFLAGS = -DFSTAB_ONLY $(AM_CXXFLAGS)
minidumper_LDADD = $(LDADD) -ldbghelp
pldd_LDADD = $(LDADD) -lpsapi
ps_LDADD = $(LDADD) -lpsapi -lntdll

if CROSS_BOOTSTRAP
SUBDIRS = mingw
endif
