## Process this file with automake to generate Makefile.in
AUTOMAKE_OPTIONS = cygnus

SUBDIRS = ces ccs lib .

cctdir=$(datadir)/iconv_data
cct_DATA=encoding.aliases

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

SUBDEFS = lib/stmp-def

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES = iconv.def
CHAPTERS = iconv.tex

iconv.def: lib/iconv.def
	cp lib/iconv.def iconv.def

iconv.xml: lib/iconv.xml
	cp lib/iconv.xml iconv.xml

stmp-def: force
	(cd lib && $(MAKE) doc)
	touch $@

.PHONY: force
force:

$(SUBDEFS): stmp-def

lib/iconv.def: $(SUBDEFS); @true

lib/stmp-def: ; @true
