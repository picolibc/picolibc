#ifndef __OR1K_INTERNAL_H__
#define __OR1K_INTERNAL_H__

#include <stdint.h>
#include <string.h>

#include "include/or1k-support.h"

extern uint8_t* _or1k_stack_top;
extern size_t _or1k_stack_size;
extern uint8_t* _or1k_stack_bottom;

extern uint8_t* _or1k_exception_stack_top;
extern size_t _or1k_exception_stack_size;
extern uint8_t* _or1k_exception_stack_bottom;

#ifdef __OR1K_MULTICORE__
extern uint8_t* *_or1k_stack_core;
extern uint8_t* *_or1k_exception_stack_core;
#endif


// The first two vectors are not used (address 0 and reset)
#define OR1K_NUM_EXCEPTIONS 30

typedef or1k_exception_handler_fptr or1k_exception_handler_table_t[OR1K_NUM_EXCEPTIONS];

#ifdef __OR1K_MULTICORE__
extern or1k_exception_handler_table_t *_or1k_exception_handler_table;
#else
extern or1k_exception_handler_table_t _or1k_exception_handler_table;
#endif

typedef or1k_interrupt_handler_fptr or1k_interrupt_handler_table_t[32];
typedef void* or1k_interrupt_handler_data_ptr_table_t[32];

#ifdef __OR1K_MULTICORE__
extern or1k_interrupt_handler_table_t *_or1k_interrupt_handler_table;
extern or1k_interrupt_handler_data_ptr_table_t *_or1k_interrupt_handler_data_ptr_table;
#else
extern or1k_interrupt_handler_table_t _or1k_interrupt_handler_table;
extern or1k_interrupt_handler_data_ptr_table_t _or1k_interrupt_handler_data_ptr_table;
#endif

extern void _or1k_interrupt_handler(void);

struct _or1k_reent {
	/* Tick timer variable */
	volatile uint32_t or1k_timer_ticks;

	/* Tick rate storage */
	uint32_t or1k_timer_period;
	uint32_t or1k_timer_mode;
};


#ifdef __OR1K_MULTICORE__
extern struct _or1k_reent (*_or1k_reent)[];
#define OR1K_REENT (*_or1k_reent)[or1k_coreid()]
#else
extern struct _or1k_reent _or1k_reent;
#define OR1K_REENT _or1k_reent
#endif

extern void _or1k_reent_init();

#endif
