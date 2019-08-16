#include <string.h>

extern int main(void);

extern char __data_source__[];
extern char __data_start__[];
extern char __data_end__[];
extern char __bss_start__[];
extern char __bss_end__[];

int
_start(void)
{
#ifndef __SOFTFP__
#define FPSCR_FZ		(1 << 24)

	unsigned int fpscr_save;

	/* Set the FZ (flush-to-zero) bit in FPSCR.  */
	__asm__("vmrs %0, fpscr" : "=r" (fpscr_save));
	fpscr_save |= FPSCR_FZ;
	__asm__("vmsr fpscr, %0" : : "r" (fpscr_save));
#endif
	memcpy(__data_start__, __data_source__,
	       __data_end__ - __data_start__);
	memset(__bss_start__, '\0',
	       __bss_end__ - __bss_start__);
	return main();
}
