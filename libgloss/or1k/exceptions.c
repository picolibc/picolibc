#include "include/or1k-support.h"

#include "or1k-internals.h"

#ifdef __OR1K_MULTICORE__
or1k_exception_handler_table_t *_or1k_exception_handler_table;
#else
or1k_exception_handler_table_t _or1k_exception_handler_table;
#endif

void or1k_exception_handler_add(int id, or1k_exception_handler_fptr handler)
{
	// Subtract 2 as we do not have a vector at 0 and reset is static
	id = id - 2;
#ifdef __OR1K_MULTICORE__
	_or1k_exception_handler_table[or1k_coreid()][id] = handler;

#else
	_or1k_exception_handler_table[id] = handler;
#endif
}
