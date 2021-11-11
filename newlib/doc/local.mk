# We can't use noinst_PROGRAMS, because automake will add $(EXEEXT).
noinst_DATA += doc/makedoc

MKDOC = doc/makedoc$(EXEEXT_FOR_BUILD)

# We don't use CFLAGS with CC_FOR_BUILD because here CFLAGS will
# actually be CFLAGS_FOR_TARGET, and in some cases that will include
# -Os, which CC_FOR_BUILD may not recognize.

$(MKDOC): doc/makedoc.o
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(LDFLAGS_FOR_BUILD) -o $@ $<

doc/makedoc.o: doc/makedoc.c
	$(MKDIR_P) doc
	$(CC_FOR_BUILD) -g $(CFLAGS_FOR_BUILD) -o $@ -c $<

man-cache:
	${srcdir}/doc/makedocbook.py --cache

PHONY += man-cache
