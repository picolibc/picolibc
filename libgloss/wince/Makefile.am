## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = foreign

gdbdir = ${dir ${patsubst %/,%,${dir @srcdir@}}}gdb
VPATH = $(gdbdir)
bin_PROGRAMS = stub.exe
stub_exe_SOURCES = wince-stub.c

tooldir = $(exec_prefix)/$(host_alias)
AM_CPPFLAGS = -I$(gdbdir)
LDADD = @LDADD@
