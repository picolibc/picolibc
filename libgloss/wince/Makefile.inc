## Process this file with automake to produce Makefile.in.

gdbdir = ${dir ${patsubst %/,%,${dir @srcdir@}}}gdb
VPATH += $(gdbdir)
bin_PROGRAMS += %D%/stub.exe
%C%_stub_exe_SOURCES = wince-stub.c
%C%_stub_exe_CPPFLAGS = $(AM_CPPFLAGS) -I$(gdbdir)
%C%_stub_exe_LDADD = -lwinsock $(WINCE_STUB_LIBS)
