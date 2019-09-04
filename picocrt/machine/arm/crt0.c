#include <string.h>

extern int main(void);

extern char __data_source__[];
extern char __data_start__[];
extern char __data_end__[];
extern char __bss_start__[];
extern char __bss_end__[];

static void *__tls;

void *
__aeabi_read_tp(void)
{
	return __tls;
}

void
_set_tls(void *tls)
{
	__tls = tls;
}

extern char __tls_base__[];
extern char __tbss_size__[];
extern char __tdata_size__[];
extern char __tdata_source__[];
extern char __tdata_size__[];

void
_init_tls(void *__tls)
{
	char *tls = __tls;
	/* Copy tls initialized data */
	memcpy(tls - (int) &__tdata_size__, __tdata_source__, (int) &__tdata_size__);
	/* Clear tls zero data */
	memset(tls, '\0', (int) &__tbss_size__);
}

#ifdef HAVE_INITFINI_ARRAY
extern void __libc_init_array(void);
extern void __libc_fini_array(void);
#endif

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
	_set_tls(__tls_base__);
#ifdef HAVE_INITFINI_ARRAY
	__libc_init_array();
#endif
	main();
#ifdef HAVE_INITFINI_ARRAY
	__libc_fini_array();
#endif
}
