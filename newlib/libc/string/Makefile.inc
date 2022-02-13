libc_a_SOURCES += \
	%D%/bcopy.c \
	%D%/bzero.c \
	%D%/explicit_bzero.c \
	%D%/ffsl.c \
	%D%/ffsll.c \
	%D%/fls.c \
	%D%/flsl.c \
	%D%/flsll.c \
	%D%/index.c \
	%D%/memchr.c \
	%D%/memcmp.c \
	%D%/memcpy.c \
	%D%/memmove.c \
	%D%/memset.c \
	%D%/rindex.c \
	%D%/strcasecmp.c \
	%D%/strcat.c \
	%D%/strchr.c \
	%D%/strcmp.c \
	%D%/strcoll.c \
	%D%/strcpy.c \
	%D%/strcspn.c \
	%D%/strdup.c \
	%D%/strdup_r.c \
	%D%/strerror.c \
	%D%/strerror_r.c \
	%D%/strlcat.c \
	%D%/strlcpy.c \
	%D%/strlen.c  \
	%D%/strlwr.c \
	%D%/strncasecmp.c \
	%D%/strncat.c \
	%D%/strncmp.c \
	%D%/strncpy.c \
	%D%/strnlen.c \
	%D%/strnstr.c \
	%D%/strpbrk.c \
	%D%/strrchr.c \
	%D%/strsep.c \
	%D%/strsignal.c \
	%D%/strspn.c \
	%D%/strtok.c \
	%D%/strtok_r.c \
	%D%/strupr.c \
	%D%/strxfrm.c  \
	%D%/strstr.c \
	%D%/swab.c \
	%D%/timingsafe_bcmp.c \
	%D%/timingsafe_memcmp.c \
	%D%/u_strerr.c \
	%D%/wcscat.c \
	%D%/wcschr.c \
	%D%/wcscmp.c \
	%D%/wcscoll.c \
	%D%/wcscpy.c \
	%D%/wcscspn.c \
	%D%/wcslcat.c \
	%D%/wcslcpy.c \
	%D%/wcslen.c \
	%D%/wcsncat.c \
	%D%/wcsncmp.c \
	%D%/wcsncpy.c \
	%D%/wcsnlen.c \
	%D%/wcspbrk.c \
	%D%/wcsrchr.c \
	%D%/wcsspn.c \
	%D%/wcsstr.c \
	%D%/wcstok.c \
	%D%/wcswidth.c \
	%D%/wcsxfrm.c \
	%D%/wcwidth.c \
	%D%/wmemchr.c \
	%D%/wmemcmp.c \
	%D%/wmemcpy.c \
	%D%/wmemmove.c \
	%D%/wmemset.c \
	%D%/xpg_strerror_r.c

%C%_ELIX_2_SOURCES =
if ELIX_LEVEL_1
else
%C%_ELIX_2_SOURCES += \
	%D%/bcmp.c \
	%D%/memccpy.c \
	%D%/mempcpy.c \
	%D%/stpcpy.c \
	%D%/stpncpy.c \
	%D%/strndup.c \
	%D%/strcasestr.c \
	%D%/strchrnul.c \
	%D%/strndup_r.c \
	%D%/wcpcpy.c \
	%D%/wcpncpy.c \
	%D%/wcsdup.c
endif !ELIX_LEVEL_1

%C%_ELIX_4_SOURCES =
if ELIX_LEVEL_1
else
if ELIX_LEVEL_2
else
if ELIX_LEVEL_3
else
%C%_ELIX_4_SOURCES += \
	%D%/gnu_basename.c \
	%D%/memmem.c \
	%D%/memrchr.c \
	%D%/rawmemchr.c \
	%D%/strcasecmp_l.c \
	%D%/strcoll_l.c \
	%D%/strncasecmp_l.c \
	%D%/strverscmp.c \
	%D%/strxfrm_l.c \
	%D%/wcscasecmp.c \
	%D%/wcscasecmp_l.c \
	%D%/wcscoll_l.c \
	%D%/wcsncasecmp.c \
	%D%/wcsncasecmp_l.c \
	%D%/wcsxfrm_l.c \
	%D%/wmempcpy.c
endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

libc_a_SOURCES += $(%C%_ELIX_2_SOURCES) $(%C%_ELIX_4_SOURCES)

LIBC_CHEWOUT_FILES += \
	%D%/bcmp.def	%D%/memcpy.def	%D%/strcmp.def	%D%/strncat.def %D%/strstr.def \
	%D%/bcopy.def	%D%/memmove.def	%D%/strcoll.def	%D%/strncmp.def %D%/strtok.def \
	%D%/bzero.def	%D%/memset.def	%D%/strcpy.def	%D%/strncpy.def %D%/strxfrm.def \
	%D%/index.def	%D%/rindex.def	%D%/strcspn.def	%D%/strpbrk.def %D%/swab.def \
	%D%/memchr.def	%D%/strcat.def	%D%/strerror.def	%D%/strerror_r.def %D%/strrchr.def \
	%D%/memcmp.def	%D%/strchr.def	%D%/strlen.def	%D%/strnlen.def %D%/strspn.def \
	%D%/strcasecmp.def	%D%/strncasecmp.def %D%/strcasestr.def	%D%/strlwr.def  %D%/strupr.def \
	%D%/memccpy.def 	%D%/mempcpy.def	%D%/stpcpy.def	%D%/stpncpy.def %D%/strsignal.def \
	%D%/wcscasecmp.def	%D%/wcscat.def	%D%/wcschr.def	%D%/wcscmp.def %D%/wcscoll.def \
	%D%/wcscpy.def	%D%/wcscspn.def	%D%/wcpcpy.def	%D%/wcpncpy.def %D%/wcsdup.def \
	%D%/wcslcat.def	%D%/wcslcpy.def	%D%/wcslen.def	%D%/wcsncasecmp.def %D%/wcsncat.def \
	%D%/wcsncmp.def	%D%/wcsncpy.def	%D%/wcsnlen.def	%D%/wcspbrk.def \
	%D%/wcsrchr.def	%D%/wcsspn.def	%D%/wcsstr.def 	%D%/wcstok.def  \
	%D%/wcswidth.def	%D%/wcsxfrm.def	%D%/wcwidth.def	%D%/wmemchr.def \
	%D%/wmemcmp.def	%D%/wmemcpy.def	%D%/wmemmove.def	%D%/wmemset.def \
	%D%/memmem.def	%D%/memrchr.def	%D%/rawmemchr.def	%D%/strchrnul.def \
	%D%/strcasecmp_l.def %D%/strcoll_l.def	%D%/strncasecmp_l.def %D%/strxfrm_l.def \
	%D%/wcscasecmp_l.def %D%/wcscoll_l.def	%D%/wcsncasecmp_l.def %D%/wcsxfrm_l.def \
	%D%/strverscmp.def	%D%/strnstr.def	%D%/wmempcpy.def

LIBC_CHAPTERS += %D%/strings.tex %D%/wcstrings.tex
