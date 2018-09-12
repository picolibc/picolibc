nano_malloc_srcs_stdlib = [
  'nano-mallocr.c',
]

std_malloc_srcs_stdlib = [
  'mallocr.c',
]

std_strtod_srcs_stdlib = [
  'strtod.c',
]

srcs_stdlib = [
    'a64l.c',
    'abort.c',
    'abs.c',
    '__adjust.c',
    'aligned_alloc.c',
    'arc4random.c',
    'arc4random_uniform.c',
    'assert.c',
    '__atexit.c',
    'atexit.c',
    'atof.c',
    'atoff.c',
    'atoi.c',
    'atol.c',
    'atoll.c',
    'btowc.c',
    '__call_atexit.c',
    'calloc.c',
    'cxa_atexit.c',
    'cxa_finalize.c',
    'div.c',
    'drand48.c',
    'dtoa.c',
    'dtoastub.c',
    'ecvtbuf.c',
    'efgcvt.c',
    'environ.c',
    'envlock.c',
    'eprintf.c',
    'erand48.c',
    'exit.c',
    '_Exit.c',
    '__exp10.c',
    'gdtoa-gethex.c',
    'gdtoa-hexnan.c',
    'getenv.c',
    'getenv_r.c',
    'getopt.c',
    'getsubopt.c',
    'imaxabs.c',
    'imaxdiv.c',
    'itoa.c',
    'jrand48.c',
    'l64a.c',
    'labs.c',
    'lcong48.c',
    'ldiv.c',
    'llabs.c',
    'lldiv.c',
    'lrand48.c',
    'malign.c',
    'malloc.c',
    'mblen.c',
    'mblen_r.c',
    'mbrlen.c',
    'mbrtowc.c',
    'mbsinit.c',
    'mbsnrtowcs.c',
    'mbsrtowcs.c',
    'mbstowcs.c',
    'mbstowcs_r.c',
    'mbtowc.c',
    'mbtowc_r.c',
    'mlock.c',
    'mprec.c',
    'mrand48.c',
    'msize.c',
    'mstats.c',
    'mtrim.c',
    'nrand48.c',
    'on_exit_args.c',
    'on_exit.c',
    'putenv.c',
    'putenv_r.c',
    'quick_exit.c',
    'rand48.c',
    'rand.c',
    'random.c',
    'rand_r.c',
    'reallocarray.c',
    'realloc.c',
    'reallocf.c',
    'rpmatch.c',
    'sb_charsets.c',
    'seed48.c',
    'setenv.c',
    'setenv_r.c',
    'srand48.c',
    'strtodg.c',
    'strtoimax.c',
    'strtol.c',
    'strtoll.c',
    'strtoll_r.c',
    'strtorx.c',
    'strtoul.c',
    'strtoull.c',
    'strtoull_r.c',
    'strtoumax.c',
    'system.c',
    '__ten_mu.c',
    'utoa.c',
    'valloc.c',
    'wcrtomb.c',
    'wcsnrtombs.c',
    'wcsrtombs.c',
    'wcstod.c',
    'wcstoimax.c',
    'wcstol.c',
    'wcstoll.c',
    'wcstoll_r.c',
    'wcstombs.c',
    'wcstombs_r.c',
    'wcstoul.c',
    'wcstoull.c',
    'wcstoull_r.c',
    'wcstoumax.c',
    'wctob.c',
    'wctomb.c',
    'wctomb_r.c',
]

lsrcs_stdlib = [
  'ldtoa.c',
  'strtold.c',
  'wcstold.c'
]

if newlib_io_long_double
  srcs_stdlib += lsrcs_stdlib
endif

if newlib_nano_malloc
  srcs_stdlib += nano_malloc_srcs_stdlib
else
  srcs_stdlib += std_malloc_srcs_stdlib
endif

if not newlib_tinystdio
  srcs_stdlib += std_strtod_srcs_stdlib
endif

hdrs_stdlib = [
    'arc4random.h',
    'atexit.h',
    'chacha_private.h',
    'gd_qnan.h',
    'gdtoa.h',
    'local.h',
    'mbctype.h',
    'mprec.h',
    'on_exit_args.h',
    'rand48.h',
    'std.h',
]

malloc_defs = [
  '-DDEFINE_MALLOC',
  '-DDEFINE_FREE',
  '-DDEFINE_REALLOC',
  '-DDEFINE_CALLOC',
  '-DDEFINE_CFREE',
  '-DDEFINE_MEMALIGN',
  '-DDEFINE_VALLOC',
  '-DDEFINE_PVALLOC',
  '-DDEFINE_MALLINFO',
  '-DDEFINE_MALLOC_STATS',
]

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_stdlib' + target,
		static_library('stdlib' + target,
			srcs_stdlib,
			pic: false,
			include_directories: inc,
			c_args: value[1] + ['-DINTERNAL_NEWLIB'] + malloc_defs))
endforeach
