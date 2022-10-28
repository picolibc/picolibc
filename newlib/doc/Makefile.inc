MKDOC = doc/makedoc$(EXEEXT_FOR_BUILD)

# We can't use noinst_PROGRAMS, because automake will add $(EXEEXT).
noinst_DATA += $(MKDOC)

# We don't use CFLAGS with CC_FOR_BUILD because here CFLAGS will
# actually be CFLAGS_FOR_TARGET, and in some cases that will include
# -Os, which CC_FOR_BUILD may not recognize.

$(MKDOC): doc/makedoc.o
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(LDFLAGS_FOR_BUILD) -o $@ $<

doc/makedoc.o: doc/makedoc.c
	$(MKDIR_P) doc
	$(CC_FOR_BUILD) -g $(CFLAGS_FOR_BUILD) -o $@ -c $<

#
# Subdir documentation rules.
#

SUFFIXES += .def

CHEW = $(MKDOC) -f $(srcdir)/%D%/doc.str

.c.def:
	$(AM_V_at)$(MKDIR_P) $(@D)
	$(AM_V_GEN)$(CHEW) < $< > $*.def || ( rm $*.def && false )

SUFFIXES += .xml

DOCBOOK_CHEW = ${top_srcdir}/%D%/makedocbook.py

.c.xml:
	$(AM_V_at)$(MKDIR_P) $(@D)
	$(AM_V_GEN)$(DOCBOOK_CHEW) < $< > $*.xml || ( rm $*.xml && false )

# We can't use .tex.xml rule here as it'll conflict with .c.xml when the chapter
# name (e.g. "stdio.xml") matches a source file name (e.g. "stdio.c").  We've
# been flattening chapters into the main library dir (e.g. libc/) to avoid that.
TEXI2DOCBOOK = $(top_srcdir)/%D%/chapter-texi2docbook.py
