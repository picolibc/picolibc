## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

SUBDIRS = $(libm_machine_dir) .

if USE_LIBTOOL
else
noinst_LIBRARIES = lib.a

lib.a: $(libm_machine_dir)/lib.a
	rm -f $@
	ln $(libm_machine_dir)/lib.a $@ >/dev/null 2>/dev/null || \
	 cp $(libm_machine_dir)/lib.a $@
endif # USE_LIBTOOL

ACLOCAL_AMFLAGS = -I ../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

$(libm_machine_dir)/lib.a:

doc:  
