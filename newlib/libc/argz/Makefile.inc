## All functions in this directory are EL/IX level 2 and above
if !ELIX_LEVEL_1
libc_a_SOURCES += \
	%D%/argz_add.c \
	%D%/argz_add_sep.c \
	%D%/argz_append.c \
	%D%/argz_count.c \
	%D%/argz_create.c \
	%D%/argz_create_sep.c \
	%D%/argz_delete.c \
	%D%/argz_extract.c \
	%D%/argz_insert.c \
	%D%/argz_next.c \
	%D%/argz_replace.c \
	%D%/argz_stringify.c \
	%D%/buf_findstr.c \
	%D%/envz_entry.c \
	%D%/envz_get.c \
	%D%/envz_add.c \
	%D%/envz_remove.c \
	%D%/envz_merge.c \
	%D%/envz_strip.c
endif
