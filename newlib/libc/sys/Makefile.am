## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

SUBDIRS = $(sys_dir) .

if HAVE_SYS_DIR
SYSLIB = lib.a
endif

noinst_LIBRARIES = $(SYSLIB)
noinst_DATA = $(CRT0)

lib.a: $(sys_dir)/lib.a
	rm -f $@
	ln $(sys_dir)/lib.a $@ >/dev/null 2>/dev/null \
	 || cp $(sys_dir)/lib.a $@

$(sys_dir)/lib.a: ; @true

crt0.o: $(sys_dir)/crt0.o
	rm -f $@
	ln $(sys_dir)/crt0.o $@ >/dev/null 2>/dev/null \
	 || cp $(sys_dir)/crt0.o $@

$(sys_dir)/crt0.o: ; @true

CLEANFILES = crt0.o

doc:

ACLOCAL_AMFLAGS = -I ../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
