#include <string.h>

extern int main(void);

extern char __data_source__[];
extern char __data_start__[];
extern char __data_end__[];
extern char __bss_start__[];
extern char __bss_end__[];

void
_set_tls(void *tls)
{
	asm("la tp, %0" : "=r" (tls));
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
	asm("la gp, __global_pointer$");
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
