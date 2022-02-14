## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	bsearch.c \
	db_local.h \
	extern.h \
	hash.h \
	ndbm.c \
	page.h \
	qsort.c

## Following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
ELIX_2_SOURCES =
else
ELIX_2_SOURCES = \
	hash.c \
	hash_bigkey.c \
	hash_buf.c \
	hash_func.c \
	hash_log2.c \
	hash_page.c \
	hcreate.c \
	hcreate_r.c \
	tdelete.c \
	tdestroy.c \
	tfind.c \
	tsearch.c \
	twalk.c
endif

## Following are EL/IX level 4 interfaces
if ELIX_LEVEL_1
ELIX_4_SOURCES =
else
if ELIX_LEVEL_2
ELIX_4_SOURCES =
else
if ELIX_LEVEL_3
ELIX_4_SOURCES =
else
ELIX_4_SOURCES = \
	bsd_qsort_r.c \
	qsort_r.c
endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
