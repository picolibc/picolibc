## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = -I$(srcdir)/../common -I$(srcdir)/ $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

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

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(src) $(fsrc) $(lsrc)
lib_a_CFLAGS = $(AM_CFLAGS)

# A partial dependency list.

$(lib_a_OBJECTS): $(srcdir)/../../libc/include/complex.h $(srcdir)/cephes_subr.h $(srcdir)/cephes_subrf.h  $(srcdir)/cephes_subrl.h
