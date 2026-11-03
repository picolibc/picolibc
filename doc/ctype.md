# Ctype in Picolibc

The Ctype family of functions provide a convenient way to process
character data. The traditional implementation of these is to build a
table of per-character classification data and have the individual
functions index the table and extract the relevant data. This is quite
efficient in code space and time for each indivdual test, but the
table introduces a fairly large fixed cost.

Picolibc provides an alternate implementation, selectable when
compiling files using the API, where each function directly implements
the necessary comparison operations. These are implemented as inline
functions. In many cases, the resulting code is no larger than the
table indexing operations and without the overhead of the table.

For C code, the direct implementation is selected by default when
building with the 'optimize for size' option enabled (-Os). Otherwise,
the table-based implementation is the default. You can select a
particular mode by defining `_PICOLIBC_CTYPE_SMALL` to `1` (direct
implementation) or `0` (table-based implementation), subject to the
exceptions below.

`<ctype.h>` always selects the table-based implementation for C++ code
and when `__MB_EXTENDED_CHARSETS_NON_UNICODE` is defined. In these cases,
it resets `_PICOLIBC_CTYPE_SMALL` to `0`, even if you explicitly defined
it as `1`. The direct implementation does not support those extended
character sets, and libstdc++'s ctype implementation requires the tables.

A project containing both C and C++ files can therefore include both
implementations, depending on the options used to compile the C files.
