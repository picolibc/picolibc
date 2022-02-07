## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	asctime.c	\
	asctime_r.c	\
	clock.c		\
	ctime.c		\
	ctime_r.c	\
	difftime.c	\
	gettzinfo.c	\
	gmtime.c	\
	gmtime_r.c	\
	lcltime.c	\
	lcltime_r.c	\
	mktime.c	\
	month_lengths.c \
	strftime.c  	\
	strptime.c	\
	time.c		\
	tzcalc_limits.c \
	tzlock.c	\
	tzset.c		\
	tzset_r.c	\
	tzvars.c	\
	wcsftime.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)

$(lpfx)wcsftime.o: strftime.c
