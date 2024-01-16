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

When building with the 'optimize for size' option enabled (-Os), the
new direct implementation is selected by default. You can also force a
particular mode by defining `_PICOLIBC_CTYPE_SMALL` to `1` (select
direct implementation) or `0` (select array-based implementation).
