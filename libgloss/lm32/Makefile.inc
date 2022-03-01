multilibtool_DATA += %D%/crt0.o %D%/sim.ld
multilibtool_LIBRARIES += %D%/libgloss.a
libobjs_a_SOURCES += %D%/crt0.S

%C%_libgloss_a_SOURCES = \
	%D%/isatty.c \
	%D%/scall.S
