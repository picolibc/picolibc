## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

src = 	s_finite.c s_copysign.c s_modf.c s_scalbn.c \
	s_cbrt.c s_expm1.c s_ilogb.c \
	s_infinity.c s_log1p.c s_nan.c s_nextafter.c \
	s_rint.c s_logb.c s_matherr.c s_lib_ver.c

fsrc =	sf_finite.c sf_copysign.c sf_modf.c sf_scalbn.c \
	sf_cbrt.c sf_expm1.c sf_ilogb.c \
	sf_infinity.c sf_log1p.c sf_nan.c sf_nextafter.c \
	sf_rint.c sf_logb.c

lib_a_SOURCES = $(src) $(fsrc)

chobj =	scbrt.def scopysign.def sexpm1.def silogb.def \
	sinfinity.def slog1p.def smatherr.def smodf.def \
	snan.def snextafter.def sscalbn.def

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(chobj)
	cat $(srcdir)/common.tex >> $(TARGETDOC)

CLEANFILES = $(chobj) *.ref

# Texinfo does not appear to support underscores in file names, so we
# name the .def files without underscores.

smodf.def: s_modf.c
	$(CHEW) < $(srcdir)/s_modf.c >$@ 2>/dev/null
	touch stmp-def

scopysign.def: s_copysign.c
	$(CHEW) < $(srcdir)/s_copysign.c >$@ 2>/dev/null
	touch stmp-def

sscalbn.def: s_scalbn.c
	$(CHEW) < $(srcdir)/s_scalbn.c >$@ 2>/dev/null
	touch stmp-def

scbrt.def: s_cbrt.c
	$(CHEW) < $(srcdir)/s_cbrt.c >$@ 2>/dev/null
	touch stmp-def

serf.def: s_erf.c
	$(CHEW) < $(srcdir)/s_serf.c >$@ 2>/dev/null
	touch stmp-def

sexpn.def: s_expm.c
	$(CHEW) < $(srcdir)/s_expn.c >$@ 2>/dev/null
	touch stmp-def

sexpm1.def: s_expm1.c
	$(CHEW) < $(srcdir)/s_expm1.c >$@ 2>/dev/null
	touch stmp-def

silogb.def: s_ilogb.c
	$(CHEW) < $(srcdir)/s_ilogb.c >$@ 2>/dev/null
	touch stmp-def

sinfinity.def: s_infinity.c
	$(CHEW) < $(srcdir)/s_infinity.c >$@ 2>/dev/null
	touch stmp-def

slog1p.def: s_log1p.c
	$(CHEW) < $(srcdir)/s_log1p.c >$@ 2>/dev/null
	touch stmp-def

smatherr.def: s_matherr.c
	$(CHEW) < $(srcdir)/s_matherr.c >$@ 2>/dev/null
	touch stmp-def

snan.def: s_nan.c
	$(CHEW) < $(srcdir)/s_nan.c >$@ 2>/dev/null
	touch stmp-def

snextafter.def: s_nextafter.c
	$(CHEW) < $(srcdir)/s_nextafter.c >$@ 2>/dev/null
	touch stmp-def

# A partial dependency list.

$(lib_a_OBJECTS): $(srcdir)/../../libc/include/math.h fdlibm.h
