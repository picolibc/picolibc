## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

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
	wcsxfrm_l.c \
	wmempcpy.c
endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
