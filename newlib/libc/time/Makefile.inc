libc_a_SOURCES += \
	%D%/asctime.c \
	%D%/asctime_r.c \
	%D%/clock.c \
	%D%/ctime.c \
	%D%/ctime_r.c \
	%D%/difftime.c \
	%D%/gettzinfo.c \
	%D%/gmtime.c \
	%D%/gmtime_r.c \
	%D%/lcltime.c \
	%D%/lcltime_r.c \
	%D%/mktime.c \
	%D%/month_lengths.c \
	%D%/strftime.c   \
	%D%/strptime.c \
	%D%/time.c \
	%D%/tzcalc_limits.c \
	%D%/tzlock.c \
	%D%/tzset.c \
	%D%/tzset_r.c \
	%D%/tzvars.c \
	%D%/wcsftime.c

LIBC_CHEWOUT_FILES += \
	%D%/asctime.def \
	%D%/clock.def \
	%D%/ctime.def \
	%D%/difftime.def \
	%D%/gmtime.def \
	%D%/lcltime.def \
	%D%/mktime.def \
	%D%/strftime.def \
	%D%/time.def \
	%D%/tzlock.def \
	%D%/tzset.def \
	%D%/wcsftime.def

LIBC_CHAPTERS += %D%/time.tex
