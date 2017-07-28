## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I$(srcdir)/../common -I$(srcdir)/ $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

src = cabs.c cacos.c cacosh.c carg.c casin.c casinh.c \
      catan.c catanh.c ccos.c ccosh.c cephes_subr.c \
      cexp.c cimag.c  clog.c clog10.c conj.c  \
      cpow.c cproj.c  creal.c  \
      csin.c csinh.c csqrt.c ctan.c ctanh.c

lsrc = cabsl.c creall.c cimagl.c ccoshl.c cacoshl.c \
       clogl.c csqrtl.c cargl.c cprojl.c cexpl.c \
       cephes_subrl.c cacosl.c ccosl.c casinl.c \
       catanhl.c conjl.c cpowl.c ctanhl.c ctanl.c \
       casinhl.c csinhl.c csinl.c catanl.c

fsrc =	cabsf.c casinf.c ccosf.c cimagf.c cprojf.c  \
        csqrtf.c cacosf.c casinhf.c ccoshf.c clogf.c clog10f.c \
        crealf.c ctanf.c cacoshf.c catanf.c   \
        cephes_subrf.c conjf.c csinf.c ctanhf.c \
        cargf.c catanhf.c cexpf.c cpowf.c csinhf.c

libcomplex_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libcomplex.la
libcomplex_la_SOURCES = $(src) $(fsrc) $(lsrc)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(src) $(fsrc) $(lsrc)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES =	cabs.def cacos.def cacosh.def carg.def \
        casin.def casinh.def catan.def catanh.def \
        ccos.def ccosh.def cexp.def cimag.def clog.def \
        clog10.def conj.def cpow.def cproj.def creal.def \
        csin.def csinh.def csqrt.def ctan.def ctanh.def

CHAPTERS = complex.tex

# A partial dependency list.

$(lib_a_OBJECTS): $(srcdir)/../../libc/include/complex.h $(srcdir)/cephes_subr.h $(srcdir)/cephes_subrf.h  $(srcdir)/cephes_subrl.h
