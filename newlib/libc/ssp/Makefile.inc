## General sources.
%C%_ELIX_SOURCES = \
  %D%/chk_fail.c \
  %D%/stack_protector.c

## String sources.
%C%_ELIX_SOURCES += \
  %D%/memcpy_chk.c \
  %D%/memmove_chk.c \
  %D%/mempcpy_chk.c \
  %D%/memset_chk.c \
  %D%/stpcpy_chk.c \
  %D%/stpncpy_chk.c \
  %D%/strcat_chk.c \
  %D%/strcpy_chk.c \
  %D%/strncat_chk.c \
  %D%/strncpy_chk.c

## Stdio sources.
if HAVE_STDIO_DIR
%C%_ELIX_SOURCES += \
  %D%/gets_chk.c \
  %D%/snprintf_chk.c \
  %D%/sprintf_chk.c \
  %D%/vsnprintf_chk.c \
  %D%/vsprintf_chk.c
endif

## None of these functions are specified by EL/IX
if ELIX_LEVEL_1
else
if ELIX_LEVEL_2
else
if ELIX_LEVEL_3
else
if ELIX_LEVEL_4
else
libc_a_SOURCES += $(%C%_ELIX_SOURCES)
endif
endif
endif
endif

LIBC_CHEWOUT_FILES +=

LIBC_CHAPTERS += %D%/ssp.tex
