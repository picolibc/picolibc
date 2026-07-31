# Thread Safety in Picolibc
Copyright © 2026 Keith Packard

For much of the standard C library, the implementation is expected to
operate correctly in the presence of multiple threads. Picolibc has
variety of techniques for ensuring this: atomics, thread local
variables and mutexes.

In other areas, the promises made by the standard are less
robust. Some APIs have both thread-unsafe and thread-safe variants
while others are draped with `[[deprecated]]` annotations and dire
warnings in the documentation.

This document describes which portions of Picolibc are thread safe and
which are not.

## Data which are supposed to be thread-safe

The C library is required to make some data local to each thread and
not be shared.

For `errno`, many C libraries have complicated mechanisms to obscure
where the value is stored. Picolibc takes a simple route and makes
`errno` a public thread local variable.

POSIX offers locale APIs like `uselocale` that set thread-local state.
Picolibc uses thread local variables for that.

## Functions using global data with atomics

Because locking is expensive and often difficult to implement in
embedded environments, there is one place in Picolibc where thread
safety is achieved by careful use of atomic operations.

 * getc/ungetc. Stdio is only required to buffer a single character
   via ungetc, and that's all Picolibc offers. Because of this
   restriction, it can store that as an atomic variable and provide
   thread safety without any locks.

## Functions using global data with locking

For functions which hold internal state that is expected to be shared
across threads, we need to manage that using locks instead of
per-thread variables.

 * arc4random, arc4random_buf. Holds the PRNG state which is seeded
   using getentropy.

 * on_exit, atexit, __cxa_atexit. Picolibc only calls exit handlers
   when exit itself is invoked and not at thread
   termination. Therefore, the global exit handlers are all shared.

 * aligned_alloc, calloc, free, malloc, memalign, posix_memalign,
   pvalloc, realloc, valloc. There is only one memory heap in
   picolibc, so all of its state is protected by locking for use by
   multiple threads.

 * setlocale. In picolibc, a locale is represented by a single
   integer. setlocale is the only function which writes to this, so
   instead of using atomics, we simply assume that the value can be
   safely read and written using normal memory accesses.

 * tzset. This function parses the value of the TZ environment
   variable into a bunch of global variables, including `extern long
   timezone;` and `extern int daylight;`. These globals are then
   accessed by a range of time-zone sensitive functions: asctime,
   asctime_r, ctime, ctime_r, localtime, localtime_r and
   mktime. Locking is used to avoid tearing between tzset and the
   other functions.

 * Buffered I/O in stdio. Each FILE using the buffered I/O stdio
   module, which includes files opened using fopen and fdopen, has a
   lock protecting its internal state. Files that only provide get,
   put and flush interfaces are intrinsically thread safe as they have
   no mutable state other than ungetc.

## Functions using thread-local data

In many cases, the lack of thread safety is simply a matter of using
static variables for persistent state and not an intrinsic requirement
of the API. Where practical, picolibc makes these thread safe by using
thread local static storage rather than global static storage.

Here are places where picolibc uses thread-local storage for static
data so that multiple threads can safely use these APIs. This is
permitted, but not required, by the C standard.

 * asctime. See also asctime_r.
 * localtime. See also localtime_r.
 * strsignal. This uses a thread-local string for unknown signals.
 * ecvt. Use snprintf instead.
 * fcvt. Use snprintf instead.
 * rand48. Provides per-thread random sequences.
 * random. Also provides per-thread random sequences.
 * strtok. See also strtok_r and strsep.
 * gmtime. See also gmtime_r.

## Functions using global data without locking

Many of these return pointers to persistent data which makes locking
unhelpful. Others are constrained by standards which declare the
shared state as explicitly extern. In most cases, there are
straightforward thread-safe alternatives.

 * hcreate, hsearch, hdestroy. There are _r versions of all of these
   which applications should use instead.

 * tmpnam. This function shouldn't be used. Not only does it have data
   races between threads, it also introduces data races with other
   applications.

 * setenv. The environ variable is defined as `extern char **environ;`
   by POSIX, which limits the ability to provide any sort of locking
   around access.

 * endgrent, fgetgrent, getgrent, getgrent_r, getgrgid,
   getgrnam. These all use shared data in ways that make locking
   unhelpful. Applications should use the re-entrant functions
   (fgetgrent_r, getgrgid_r and getgruid_r) instead.

 * endpwent, fgetpwent, getpwent, getpwent_r, getpwnam,
   getpwuid. These all use shared data in ways that make locking
   unhelpful. Applications should use the re-entrant functions
   (fgetpwent_r, getpwnam_r and getpwuid_r) instead.

 * getopt, getopt_long, getopt_long_only. Picolibc offers _r variants
   of these functions that hold state in a buffer provided by the
   application, however that's not standard.

 * getsubopt. There isn't any re-entrant version of this function.

 * set_constraint_handler_s. Annex K of the C standard adds quite a
   few functions that improve thread safety, but this is not one of
   them. You get only a single global handler that is called when a
   runtime constraint violation is detected by one of the Annex K
   functions.

 * mblen, mbtowc. These two functions hold internal state between
   translations. Use mbrlen and mbrtowc instead.

 * mbrlen, mbrtowc, wcrtomb, c8rtomb, c16rtomb, c32rtomb, mbrtoc8,
   mbrtoc16, mbrtoc32. These functions all take an optional state
   buffer, if you don't provide one, a static one will be used
   instead. Don't do that.

 * getdate. This returns a pointer to static storage as well as
   storing an error code in a variable declared as `extern int
   getdate_err;`, which cannot be made thread-local. Use getdate_r
   instead.

 * lgamma. This returns the sign of the true gamma value in the global
   signgam. Use lgamma_r instead.
   
