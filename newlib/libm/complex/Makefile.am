## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I$(srcdir)/../common -I$(srcdir)/ $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

src = cabs.c cacos.c cacosh.c carg.c casin.c casinh.c \
      catan.c catanh.c ccos.c ccosh.c cephes_subr.c \
      cexp.c cimag.c  clog.c conj.c  \
      cpow.c cproj.c  creal.c  \
      csin.c csinh.c csqrt.c ctan.c ctanh.c
	

fsrc =	cabsf.c casinf.c ccosf.c cimagf.c cprojf.c  \
        csqrtf.c cacosf.c casinhf.c ccoshf.c clogf.c \
        crealf.c ctanf.c cacoshf.c catanf.c   \
        cephes_subrf.c conjf.c csinf.c ctanhf.c \
        cargf.c catanhf.c cexpf.c cpowf.c csinhf.c

libcomplex_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libcomplex.la
libcomplex_la_SOURCES = $(src) $(fsrc)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(src) $(fsrc)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

chobj =	cabs.def cacos.def cacosh.def carg.def \
        casin.def casinh.def catan.def catanh.def \
        ccos.def ccosh.def cexp.def cimag.def clog.def \
        conj.def cpow.def cproj.def creal.def \
        csin.def csinh.def csqrt.def ctan.def ctanh.def


SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(chobj)
	cat $(srcdir)/complex.tex >> $(TARGETDOC)

CLEANFILES = $(chobj) *.ref

# A partial dependency list.

$(lib_a_OBJECTS): $(srcdir)/../../libc/include/complex.h $(srcdir)/cephes_subr.h $(srcdir)/cephes_subrf.h
