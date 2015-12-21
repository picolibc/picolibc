/*
 * Static instance of _on_exit_args struct.
 *
 * When _REENT_SMALL is used, _atexit struct only contains a pointer to
 * _on_exit_args struct, so this was always allocated with malloc() - even for
 * the first 32 calls of atexit()-like functions, which are guaranteed to
 * succeed, but could fail because of "out of memory" error. This is even worse
 * when _ATEXIT_DYNAMIC_ALLOC is _NOT_ defined, in which case malloc() is not
 * used by internals of atexit()-like functions. In such configuration all calls
 * to the functions that need _on_exit_args struct (on_exit() and
 * __cxa_atexit()) would fail.
 *
 * Thats why a static instance of _on_exit_args struct is provided for
 * _REENT_SMALL configuration. This way the first 32 calls to atexit()-like
 * functions don't need malloc() and will always succeed.
 *
 * Because this struct is not needed for "normal" atexit(), it is used as a weak
 * reference in __register_exitproc(), but any use of on_exit() or
 * __cxa_atexit() will force it to be linked.
 */

#include <reent.h>

#ifdef _REENT_SMALL

static struct _on_exit_args _on_exit_args_instance = {{_NULL}, {_NULL}, 0, 0};

struct _on_exit_args * const __on_exit_args = &_on_exit_args_instance;

#endif	/* def _REENT_SMALL */
