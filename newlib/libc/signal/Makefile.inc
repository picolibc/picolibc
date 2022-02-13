if HAVE_SIGNAL_DIR

libc_a_SOURCES += \
	%D%/psignal.c %D%/raise.c %D%/signal.c %D%/sig2str.c

endif

LIBC_CHEWOUT_FILES += \
	%D%/psignal.def \
	%D%/raise.def \
	%D%/signal.def \
	%D%/sig2str.def

LIBC_CHAPTERS += %D%/signal.tex
