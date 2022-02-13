libc_a_SOURCES += \
	%D%/bsearch.c \
	%D%/db_local.h \
	%D%/extern.h \
	%D%/hash.h \
	%D%/ndbm.c \
	%D%/page.h \
	%D%/qsort.c

## Following are EL/IX level 2 interfaces
if !ELIX_LEVEL_1
libc_a_SOURCES += \
	%D%/hash.c \
	%D%/hash_bigkey.c \
	%D%/hash_buf.c \
	%D%/hash_func.c \
	%D%/hash_log2.c \
	%D%/hash_page.c \
	%D%/hcreate.c \
	%D%/hcreate_r.c \
	%D%/tdelete.c \
	%D%/tdestroy.c \
	%D%/tfind.c \
	%D%/tsearch.c \
	%D%/twalk.c
endif

## Following are EL/IX level 4 interfaces
if ELIX_LEVEL_1
else
if ELIX_LEVEL_2
else
if ELIX_LEVEL_3
else
libc_a_SOURCES += \
	%D%/bsd_qsort_r.c \
	%D%/qsort_r.c
endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

LIBC_CHEWOUT_FILES += \
	%D%/bsearch.def \
	%D%/qsort.def \
	%D%/qsort_r.def
