## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
  chk_fail.c \
  stack_protector.c

STRING_SOURCES = \
  memcpy_chk.c \
  memmove_chk.c \
  mempcpy_chk.c \
  memset_chk.c \
  stpcpy_chk.c \
  stpncpy_chk.c \
  strcat_chk.c \
  strcpy_chk.c \
  strncat_chk.c \
  strncpy_chk.c

STDIO_SOURCES = \
  gets_chk.c \
  snprintf_chk.c \
  sprintf_chk.c \
  vsnprintf_chk.c \
  vsprintf_chk.c

## None of these functions are specified by EL/IX
if ELIX_LEVEL_1
ELIX_SOURCES =
else
if ELIX_LEVEL_2
ELIX_SOURCES =
else
if ELIX_LEVEL_3
ELIX_SOURCES =
else
if ELIX_LEVEL_4
ELIX_SOURCES =
else
if HAVE_STDIO_DIR
ELIX_SOURCES = $(GENERAL_SOURCES) $(STRING_SOURCES) $(STDIO_SOURCES)
else
ELIX_SOURCES = $(GENERAL_SOURCES) $(STRING_SOURCES)
endif
endif
endif
endif
endif

libssp_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libssp.la
libssp_la_SOURCES = $(ELIX_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

CHEWOUT_FILES =
CHAPTERS = ssp.tex

include $(srcdir)/../../Makefile.shared
