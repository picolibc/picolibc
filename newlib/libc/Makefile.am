## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

if HAVE_POSIX_DIR
POSIX_SUBDIR = posix
endif

if HAVE_SIGNAL_DIR
SIGNAL_SUBDIR = signal
endif

if HAVE_SYSCALL_DIR
SYSCALLS_SUBDIR = syscalls
endif

if HAVE_UNIX_DIR
UNIX_SUBDIR = unix
endif

# The order of SUBDIRS is important for the integrated documentation.
# Do not change the order without considering the doc impact.
SUBDIRS = stdlib ctype stdio string $(SIGNAL_SUBDIR) time locale sys reent \
	errno misc machine $(UNIX_SUBDIR) $(POSIX_SUBDIR) $(SYSCALLS_SUBDIR) .

noinst_DATA = $(CRT0)

if USE_LIBTOOL
noinst_LTLIBRARIES = libc.la
SUBLIBS = \
	stdlib/libstdlib.$(aext) \
	ctype/libctype.$(aext) \
	stdio/libstdio.$(aext) \
	string/libstring.$(aext) \
	$(LIBC_SIGNAL_LIB) \
	time/libtime.$(aext) \
	locale/liblocale.$(aext) \
	$(LIBC_SYS_LIB) \
	reent/libreent.$(aext) \
	errno/liberrno.$(aext) \
	misc/libmisc.$(aext) \
	$(LIBC_MACHINE_LIB) \
	$(LIBC_UNIX_LIB) \
	$(LIBC_POSIX_LIB) \
	$(LIBC_SYSCALL_LIB)
else
noinst_LIBRARIES = libc.a
SUBLIBS = \
	stdlib/lib.$(aext) \
	ctype/lib.$(aext) \
	stdio/lib.$(aext) \
	string/lib.$(aext) \
	$(LIBC_SIGNAL_LIB) \
	time/lib.$(aext) \
	locale/lib.$(aext) \
	$(LIBC_SYS_LIB) \
	reent/lib.$(aext) \
	errno/lib.$(aext) \
	misc/lib.$(aext) \
	$(LIBC_MACHINE_LIB) \
	$(LIBC_UNIX_LIB) \
	$(LIBC_POSIX_LIB) \
	$(LIBC_SYSCALL_LIB)
endif

libc_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
libc_la_SOURCES =
libc_la_LIBADD = $(SUBLIBS)
else
libc.a: $(SUBLIBS)
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

$(SUBLIBS): ; @true

crt0.o: sys/crt0.o
	rm -f $@
	ln sys/crt0.o $@ >/dev/null 2>/dev/null || cp sys/crt0.o $@

sys/crt0.o: ; @true

info_TEXINFOS = libc.texinfo

# This is a list of the stmp-def files in each subdirectory which
# builds .def files.  We don't list subdirectories which don't build
# .def files; if the list of subdirectories changes, we must change
# this as well.
SUBDEFS = \
	stdlib/stmp-def \
	ctype/stmp-def \
	stdio/stmp-def \
	string/stmp-def \
	$(LIBC_SIGNAL_DEF) \
	time/stmp-def \
	locale/stmp-def \
	reent/stmp-def \
	misc/stmp-def

libc.info: sigset.texi targetdep.tex $(SUBDEFS)

stmp-sigset: config.status
	if test -n "$(LIBC_SIGNAL_LIB)"; then \
	  echo "@set SIGNALS" >tmp.texi; \
	else \
	  echo "@clear SIGNALS" >tmp.texi; \
	fi
	$(SHELL) $(newlib_basedir)/../move-if-change tmp.texi sigset.texi
	touch $@

sigset.texi: stmp-sigset ; @true

stmp-targetdep: force
	rm -f tmp.texi
	targetdoc=`pwd`/tmp.texi; \
	for d in $(SUBDIRS); do \
	  if test "$$d" != "."; then \
	    (cd $$d && $(MAKE) TARGETDOC=$${targetdoc} doc) || exit 1; \
	  fi; \
	done
	cat $(srcdir)/sys.tex >>tmp.texi
	$(SHELL) $(newlib_basedir)/../move-if-change tmp.texi targetdep.tex
	touch $@

targetdep.tex: stmp-targetdep ; @true

$(SUBDEFS): stmp-targetdep ; @true

.PHONY: force
force:

CLEANFILES = $(CRT0) \
	sigset.texi stmp-sigset tmp.texi targetdep.tex stmp-targetdep

ACLOCAL_AMFLAGS = -I ..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
