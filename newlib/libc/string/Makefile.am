## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	bcopy.c \
	bzero.c \
	explicit_bzero.c \
	ffsl.c \
	ffsll.c \
	fls.c \
	flsl.c \
	flsll.c \
	index.c \
	memchr.c \
	memcmp.c \
	memcpy.c \
	memmove.c \
	memset.c \
	rindex.c \
	strcasecmp.c \
	strcat.c \
	strchr.c \
	strcmp.c \
	strcoll.c \
	strcpy.c \
	strcspn.c \
	strdup.c \
	strdup_r.c \
	strerror.c \
	strerror_r.c \
	strlcat.c \
	strlcpy.c \
	strlen.c  \
	strlwr.c \
	strncasecmp.c \
	strncat.c \
	strncmp.c \
	strncpy.c \
	strnlen.c \
	strnstr.c \
	strpbrk.c \
	strrchr.c \
	strsep.c \
	strsignal.c \
	strspn.c \
	strtok.c \
	strtok_r.c \
	strupr.c \
	strxfrm.c  \
	strstr.c \
	swab.c \
	timingsafe_bcmp.c \
	timingsafe_memcmp.c \
	u_strerr.c \
	wcscat.c \
	wcschr.c \
	wcscmp.c \
	wcscoll.c \
	wcscpy.c \
	wcscspn.c \
	wcslcat.c \
	wcslcpy.c \
	wcslen.c \
	wcsncat.c \
	wcsncmp.c \
	wcsncpy.c \
	wcsnlen.c \
	wcspbrk.c \
	wcsrchr.c \
	wcsspn.c \
	wcsstr.c \
	wcstok.c \
	wcswidth.c \
	wcsxfrm.c \
	wcwidth.c \
	wmemchr.c \
	wmemcmp.c \
	wmemcpy.c \
	wmemmove.c \
	wmemset.c \
	xpg_strerror_r.c

if ELIX_LEVEL_1
ELIX_2_SOURCES =
else
ELIX_2_SOURCES = \
	bcmp.c \
	memccpy.c \
	mempcpy.c \
	stpcpy.c \
	stpncpy.c \
	strndup.c \
	strcasestr.c \
	strchrnul.c \
	strndup_r.c \
	wcpcpy.c \
	wcpncpy.c \
	wcsdup.c
endif !ELIX_LEVEL_1

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
	gnu_basename.c \
	memmem.c \
	memrchr.c \
	rawmemchr.c \
	strcasecmp_l.c \
	strcoll_l.c \
	strncasecmp_l.c \
	strverscmp.c \
	strxfrm_l.c \
	wcscasecmp.c \
	wcscasecmp_l.c \
	wcscoll_l.c \
	wcsncasecmp.c \
	wcsncasecmp_l.c \
	wcsxfrm_l.c
endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

libstring_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libstring.la
libstring_la_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES=\
bcmp.def	memcpy.def	strcmp.def	strncat.def strstr.def \
bcopy.def	memmove.def	strcoll.def	strncmp.def strtok.def \
bzero.def	memset.def	strcpy.def	strncpy.def strxfrm.def \
index.def	rindex.def	strcspn.def	strpbrk.def swab.def \
memchr.def	strcat.def	strerror.def	strerror_r.def strrchr.def \
memcmp.def	strchr.def	strlen.def	strnlen.def strspn.def \
strcasecmp.def	strncasecmp.def strcasestr.def	strlwr.def  strupr.def \
memccpy.def 	mempcpy.def	stpcpy.def	stpncpy.def strsignal.def \
wcscasecmp.def	wcscat.def	wcschr.def	wcscmp.def wcscoll.def \
wcscpy.def	wcscspn.def	wcpcpy.def	wcpncpy.def wcsdup.def \
wcslcat.def	wcslcpy.def	wcslen.def	wcsncasecmp.def wcsncat.def \
wcsncmp.def	wcsncpy.def	wcsnlen.def	wcspbrk.def \
wcsrchr.def	wcsspn.def	wcsstr.def 	wcstok.def  \
wcswidth.def	wcsxfrm.def	wcwidth.def	wmemchr.def \
wmemcmp.def	wmemcpy.def	wmemmove.def	wmemset.def \
memmem.def	memrchr.def	rawmemchr.def	strchrnul.def \
strcasecmp_l.def strcoll_l.def	strncasecmp_l.def strxfrm_l.def \
wcscasecmp_l.def wcscoll_l.def	wcsncasecmp_l.def wcsxfrm_l.def \
strverscmp.def	strnstr.def

CHAPTERS = strings.tex wcstrings.tex
