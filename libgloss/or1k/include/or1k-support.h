/* Copyright (c) 2014 Authors
 *
 * Contributor Julius Baxter <julius.baxter@orsoc.se>
 * Contributor Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/* -------------------------------------------------------------------------- */
/* This program is commented throughout in a fashion suitable for processing
with Doxygen. */
/* -------------------------------------------------------------------------- */

#include <stdint.h>

#ifndef __OR1K_SUPPORT_H__
#define __OR1K_SUPPORT_H__

/*!
* \defgroup or1k_macros OR1K macros
* @{
*/

/*!
* Access byte-sized memory mapped register
*
* Used to access a byte-sized memory mapped register. It avoids usage errors
* when not defining register addresses volatile and handles casting correctly.
*
* Example for both read and write:
*
* \code
* uint8_t status = REG8(IPBLOCK_STATUS_REG_ADDR);
* REG8(IPBLOCK_ENABLE) = 1;
* \endcode
*
* \param add Register address
*/
#define REG8(add) *((volatile unsigned char *) (add))

/*!
* Access halfword-sized memory mapped register
*
* Used to access a 16 byte-sized memory mapped register. It avoids usage errors
* when not defining register addresses volatile and handles casting correctly.
*
* See REG8() for an example.
*
* \param add Register address
*/
#define REG16(add) *((volatile unsigned short *) (add))

/*!
* Access word-sized memory mapped register
*
* Used to access a word-sized memory mapped register. It avoids usage errors
* when not defining register addresses volatile and handles casting correctly.
*
* See REG8() for an example.
*
* \param add Register address
*/
#define REG32(add) *((volatile unsigned long *) (add))
/*!
* @}
*/

/*!
* \defgroup or1k_interrupts OR1K interrupt control
*
* Interrupt control function prototypes
*
* @{
*/

/*! Function pointer to interrupt handler functions */
typedef void (*or1k_interrupt_handler_fptr)(void* data);

/*!
* Add interrupt handler for interrupt line
*
* Registers a callback function for a certain interrupt line.
*
* \param line Interrupt line/id to register a handler for
* \param handler Handler to register
* \param data Data value passed to the handler
*/
void or1k_interrupt_handler_add(uint32_t line,
		or1k_interrupt_handler_fptr handler,
		void* data);

/*!
* Enable interrupts from a given line
*
* Unmask the given interrupt line. It is also important to enable interrupts
* in general, e.g., using or1k_interrupts_enable().
*
* \param line Interrupt line to enable
*/
void or1k_interrupt_enable(int line);

/*!
* Disable interrupts from a given line
*
* Mask given interrupt line. It can be unmasked using or1k_interrupt_enable().
*
* \param line Interrupt line to disable
*/
void or1k_interrupt_disable(int line);

/*!
* Disable interrupts
*
* This disables the interrupt exception. This is sufficient to disable all
* interrupts. It does not change the mask register (which is modified using
* or1k_interrupt_enable() and or1k_interrupt_disable()).
*
* The interrupt exception can be enabled using or1k_interrupts_enable().
*
* Finally, the status of the interrupt exception enable flag is returned by
* this function. That allows to call this function even if interrupts are
* already disabled. To restore the value of the interrupt exception enable
* flag, use the or1k_interrupts_restore() function. That way you avoid to
* accidentally enable interrupts. Example:
*
* \code
* void f() {
*   uint32_t interrupt_status = or1k_interrupts_disable();
*   // do something
*   or1k_interrupts_restore(status);
* }
* \endcode
*
* This code will preserve the original status of the interrupt enable flag.
*
* \return Interrupt exception enable flag before call
*/
uint32_t or1k_interrupts_disable(void);

/*!
* Enable interrupt exception
*
* Enable the interrupt exception. Beside the interrupt exception, it is also
* necessary to enable the individual interrupt lines using
* or1k_interrupt_enable().
*
* You should avoid using this function together with or1k_interrupts_disable()
* to guard atomic blocks as it unconditionally enables the interrupt
* exception (see documentation of or1k_interrupts_disable()).
*/
void or1k_interrupts_enable(void);

/*!
* Restore interrupt exception enable flag
*
* This function restores the given status to the processor.
* or1k_interrupts_restore(0) is identical to or1k_interrupts_disable() and
* or1k_interrupts_restore(SPR_SR_IEE) is identical to or1k_interrupts_enable().
*
* It is for example used to guard an atomic block and restore the original
* status of the interrupt exception enable flag as returned by
* or1k_interrupts_disable(). See the documentation of or1k_interrupts_disable()
* for a usage example.
*
* \param status Status of the flag to restore
*/
void or1k_interrupts_restore(uint32_t status);

/*!
 * Disable timer and interrupt exception
 *
 * This function disables the timer and interrupt exception to guard critical
 * sections. It returns the status of the enable bits before the critical
 * section, that is restored with or1k_critical_end().
 *
 * Example:
 * \code
 * ...
 * uint32_t status = or1k_critical_start();
 * // critical part
 * or1k_critical_end(status);
 * ...
 * \endcode
 *
 * \return Status of timer and interrupt exception at time of call
 */
uint32_t or1k_critical_begin();

/*!
 * Enable timer and interrupt exception
 *
 * Restore the timer and interrupt exception enable. The restore value is the
 * return value from or1k_critical_start().
 *
 * \param restore Interrupt and timer exception enable restore value
 */
void or1k_critical_end(uint32_t restore);
/*!
* @}
*/

/*!
* \defgroup or1k_exception Exception handling
* @{
*/
/*! Function pointer to an exception handler function */
typedef void (*or1k_exception_handler_fptr)(void);

/*!
* Register exception handler
*
* Register an exception handler for the given exception id. This handler is
* in the following then called when the exception occurs. You can thereby
* individually handle those exceptions.
*
* \param id Exception id
* \param handler Handler callback
*/
void or1k_exception_handler_add(int id, or1k_exception_handler_fptr handler);
/*!
* @}
*/

/*!
* \defgroup or1k_spr SPR access
* @{
*/

/*!
* Move value to special purpose register
*
* Move data value to a special purpose register
*
* \param spr SPR identifier, see spr-defs.h for macros
* \param value value to move to SPR
*/
static inline void or1k_mtspr (uint32_t spr, uint32_t value)
{
	__asm__ __volatile__ ("l.mtspr\t\t%0,%1,0": : "r" (spr), "r" (value));
}

/*!
* Copy value from special purpose register
*
* Copy a data value from the given special purpose register.
*
* \param spr SPR identifier, see spr-defs.h for macros
* \return SPR data value
*/
static inline uint32_t or1k_mfspr (uint32_t spr) {
	uint32_t value;
	__asm__ __volatile__ ("l.mfspr\t\t%0,%1,0" : "=r" (value) : "r" (spr));
	return value;
}
/*!
* @}
*/

/*!
* \defgroup or1k_util Miscellaneous utility functions
*
* @{
*/

/*!
* Report value to simulator
*
* Uses the built-in simulator functionality.
*
* \param value Value to report
*/
void or1k_report (unsigned long int value);

/*!
* Get (pseudo) random number
*
* This should return pseudo-random numbers, based on a Galois LFSR.
*
* \return (Pseudo) Random number
*/
unsigned long int or1k_rand(void);

/*!
 * Register UART callback
 *
 * This function sets a callback function that is called when a character is
 * received via UART. The callback function has no return and a gets the
 * character as parameter. When a character is received, the function is called.
 *
 * Example (UART echo):
 * \code
 * void uart_in(char c) {
 *   printf("%c", c); // Echo
 * }
 *
 * int main() {
 *   or1k_uart_set_read_cb(&uart_in);
 *   or1k_interrupts_enable();
 *
 *   while (1) {}
 * }
 * \endcode
 */
void or1k_uart_set_read_cb(void (*cb)(char c));
/*!
* @}
*/

/*!
* \defgroup or1k_cache Cache control
*
* @{
*/

/*!
* Enable instruction cache
*/
void or1k_icache_enable(void);

/*!
* Disable instruction cache
*/
void or1k_icache_disable(void);

/*!
* Flush instruction cache
*
* Invalidate instruction cache entry
*
* \param entry Entry to invalidate
*/
void or1k_icache_flush(uint32_t entry);

/*!
* Enable data cache
*/
void or1k_dcache_enable(void);

/*!
* Disable data cache
*/
void or1k_dcache_disable(void);

/*!
* Flush data cache
*
* Invalidate data cache entry
*
* \param entry Entry to invalidate
*/
void or1k_dcache_flush(unsigned long entry);
/*!
* @}
*/

/*!
* \defgroup or1k_mmu MMU control
* @{
*/

/*!
* Enable instruction MMU
*/
void or1k_immu_enable(void);

/*!
* Disable instruction MMU
*/
void or1k_immu_disable(void);

/*!
* Enable data MMU
*/
void or1k_dmmu_enable(void);

/*!
* Disable data MMU
*/
void or1k_dmmu_disable(void);
/*!
* @}
*/

/*!
* \defgroup or1k_timer Timer control
*
* The tick timer can be used for time measurement, operating system scheduling
* etc. By default it is initialized to continuously count the ticks of a
* certain period after calling or1k_timer_init(). The period can later be
* changed using or1k_timer_set_period().
*
* The timer is controlled using or1k_timer_enable(), or1k_timer_disable(),
* or1k_timer_restore(), or1k_timer_pause(). After initialization it is required
* to enable the timer the first time using or1k_timer_enable().
* or1k_timer_disable() only disables the tick timer interrupts, it does not
* disable the timer counting. If you plan to use a pair of or1k_timer_disable()
* and or1k_timer_enable() to protect sections of your code against interrupts
* you should use or1k_timer_disable() and or1k_timer_restore(), as it may be
* possible that the timer interrupt was not enabled before disabling it,
* enable would then start it unconditionally. or1k_timer_pause() pauses the
* counting.
*
* In the default mode you can get the tick value using or1k_timer_get_ticks()
* and reset this value using or1k_timer_reset_ticks().
*
* Example for using the default mode:
*
* \code
* int main() {
*   uint32_t ticks = 0;
*   uint32_t timerstate;
*   or1k_timer_init(100);
*   or1k_timer_enable();
*   while (1) {
*     while (ticks == or1k_timer_get_ticks()) { }
*     timerstate = or1k_timer_disable();
*     // do something atomar
*     or1k_timer_restore(timerstate);
*     if (ticks == 100) {
*       printf("A second elapsed\n");
*       or1k_timer_reset_ticks();
*       ticks = 0;
*     }
*   }
* }
* \endcode
*
* It is possible to change the mode of the tick timer using
* or1k_timer_set_mode(). Allowed values are the correct bit pattern (including
* the bit positions) for the TTMR register, it is recommended to use the macros
* defined in spr-defs.h. For example, implementing an operating system with
* scheduling decisions of varying duration favors the implementation of single
* run tick timer. Here, each quantum is started before leaving the operating
* system kernel. The counter can be restarted with or1k_timer_reset().
* Example:
*
* \code
* void tick_handler(void) {
*   // Make schedule decision
*   // and set new thread
*   or1k_timer_reset();
*   // End of exception, new thread will run
* }
*
* int main() {
*   // Configure operating system and start threads..
*
*   // Configure timer
*   or1k_timer_init(50);
*   or1k_timer_set_handler(&tick_handler);
*   or1k_timer_set_mode(SPR_TTMR_SR);
*   or1k_timer_enable();
*
*   // Schedule first thread and die..
* }
* \endcode
*
* @{
*/

/*!
* Initialize tick timer
*
* This initializes the tick timer in default mode (see \ref or1k_timer for
* details).
*
* \param hz Initial period of the tick timer
* \return 0 if successful, -1 if timer not present
*/
int or1k_timer_init(unsigned int hz);

/*!
* Set period of timer
*
* Set the period of the timer to a value in Hz. The frequency from the board
* support package is used to determine the match value.
*/
void or1k_timer_set_period(uint32_t hz);

/*!
* Replace the timer interrupt handler
*
* By default the tick timer is used to handle timer ticks. The user can replace
* this with an own handler for example when implementing an operating system.
*
* \param handler The callback function pointer to the handler
*/
void or1k_timer_set_handler(void (*handler)(void));

/*!
* Set timer mode
*
* The timer has different modes (see architecture manual). The default is to
* automatically restart counting (SPR_TTMR_RT), others are single run
* (SPR_TTMR_SR) and continuous run (SPR_TTMR_CR).
*
* \param mode a valid mode (use definitions from spr-defs.h as it is important
* that those are also at the correct position in the bit field!)
*/
void or1k_timer_set_mode(uint32_t mode);

/*!
* Enable timer interrupt
*
* Enable the timer interrupt exception, independent of the status before.
* If you want to enable the timer conditionally, for example to implement a
* non-interruptible sequence of code, you should use or1k_timer_restore(). See
* the description of or1k_timer_disable() for more details.
*
* The enable will also restore the mode if the timer was paused previously.
*/
void or1k_timer_enable(void);

/*!
* Disable timer interrupt
*
* This disables the timer interrupt exception and returns the state of the
* interrupt exception enable flag before the call. This can be used with
* or1k_timer_restore() to implement sequences of code that are not allowed to
* be interrupted. Using or1k_timer_enable() will unconditionally enable the
* interrupt independent of the state before calling or1k_timer_disable().
* For an example see \ref or1k_timer.
*
* \return Status of timer interrupt before call
*/
uint32_t or1k_timer_disable(void);

/*!
* Restore timer interrupt exception flag
*
* Restores the timer interrupt exception flag as returned by
* or1k_timer_disable(). See the description of or1k_timer_disable() and \ref
* or1k_timer for details and an example.
*
* \param sr_tee Status of timer interrupt
*/
void or1k_timer_restore(uint32_t sr_tee);

/*!
* Pause timer counter
*
* Pauses the counter of the tick timer. The counter will hold its current value
* and it can be started again with or1k_timer_enable() which will restore the
* configured mode.
*/
void or1k_timer_pause(void);

/*!
* Reset timer counter
*/
void or1k_timer_reset(void);

/*!
* Get timer ticks
*
* Get the global ticks of the default configuration. This will increment the
* tick counter according to the preconfigured period.
*
* \return Current value of ticks
*/
unsigned long or1k_timer_get_ticks(void);

/*!
* Reset timer ticks
*
* Resets the timer ticks in default configuration to 0.
*/
void or1k_timer_reset_ticks(void);
/*!
* @}
*/

/*!
 * \defgroup or1k_multicore Multicore and Synchronization Support
 *
 * @{
 */

/*!
 * Compiled with multicore support
 *
 * \return 1 if compiled with multicore support, 0 otherwise
 */
uint32_t or1k_has_multicore_support(void);

/*!
 * Read core identifier
 *
 * \return Core identifier
 */
uint32_t or1k_coreid(void);

/*!
 * Read number of cores
 *
 * \return Total number of cores
 */
uint32_t or1k_numcores(void);

/*!
 * Load linked
 *
 * Load a value from the given address and link it. If the following
 * or1k_sync_sc() goes to the same address and there was no conflicting access
 * between loading and storing, the value is written back, else the write fails.
 *
 * \param address Address to load value from
 * \return Value read from the address
 */
uint32_t or1k_sync_ll(void *address);

/**
 * Store conditional
 *
 * Conditionally store a value to the address. The address must have been read
 * before using or1k_sync_ll() and there must be no other load link after that,
 * otherwise this will always fail. In case there was no other write to the same
 * address in between the load link and the store conditional, the store is
 * successful, otherwise it will also fail.
 *
 * \param address Address to conditionally store to
 * \param value Value to write to address
 * \return 1 if success, 0 if fail
 */
int or1k_sync_sc(void *address, uint32_t value);

/*!
 * Compare and Swap
 *
 * Loads a data item from the memory and compares a given value to it. If the
 * values match, a new value is written to the memory, if they mismatch, the
 * operation is aborted. The whole operation is atomic, i.e., it is guaranteed
 * that no other core changes the value between the read and the write.
 *
 * \param address Address to operate on
 * \param compare Compare value
 * \param swap New value to write
 * \return The value read from memory (can be used to check for success)
 */
uint32_t or1k_sync_cas(void *address, uint32_t compare, uint32_t swap);

/*!
 * Test and Set Lock
 *
 * Check for a lock on an address. This means, if there is 0 at an address it
 * will overwrite it with 1 and return 0. If the lock was already set (value
 * 1 read from address), the function returns 1. The operation is atomic.
 *
 * \param address Address of the lock
 * \return 0 if success, 1 if failed
 */
int or1k_sync_tsl(void *address);
/*!
 * @}
 */

#endif /* __NEWLIB_OR1K_SUPPORT_H__ */
