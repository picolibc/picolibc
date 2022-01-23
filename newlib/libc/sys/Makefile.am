## Process this file with automake to generate Makefile.in

noinst_DATA = $(CRT0)

$(CRT0): $(sys_dir)/$(CRT0)
	rm -f $@
	ln $(sys_dir)/$(CRT0) $@ >/dev/null 2>/dev/null \
	 || cp $(sys_dir)/$(CRT0) $@

$(sys_dir)/$(CRT0): ; @true

CLEANFILES = $(CRT0)

doc:

docbook:
