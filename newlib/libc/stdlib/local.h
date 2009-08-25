/* Misc. local definitions for libc/stdlib */

#ifndef _LOCAL_H_
#define _LOCAL_H_

char *	_EXFUN(_gcvt,(struct _reent *, double , int , char *, char, int));

char *__locale_charset(_NOARGS);

#ifndef __mbstate_t_defined
#include <wchar.h>
#endif

extern int (*__wctomb) (struct _reent *, char *, wchar_t, const char *,
			mbstate_t *);
int __ascii_wctomb (struct _reent *, char *, wchar_t, const char *,
		    mbstate_t *);
#ifdef _MB_CAPABLE
int __utf8_wctomb (struct _reent *, char *, wchar_t, const char *, mbstate_t *);
int __sjis_wctomb (struct _reent *, char *, wchar_t, const char *, mbstate_t *);
int __eucjp_wctomb (struct _reent *, char *, wchar_t, const char *,
		    mbstate_t *);
int __jis_wctomb (struct _reent *, char *, wchar_t, const char *, mbstate_t *);
int __iso_wctomb (struct _reent *, char *, wchar_t, const char *, mbstate_t *);
int __cp_wctomb (struct _reent *, char *, wchar_t, const char *, mbstate_t *);
#ifdef __CYGWIN__
int __gbk_wctomb (struct _reent *, char *, wchar_t, const char *, mbstate_t *);
int __kr_wctomb (struct _reent *, char *, wchar_t, const char *, mbstate_t *);
int __big5_wctomb (struct _reent *, char *, wchar_t, const char *, mbstate_t *);
#endif
#endif

extern int (*__mbtowc) (struct _reent *, wchar_t *, const char *, size_t,
			const char *, mbstate_t *);
int __ascii_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		    const char *, mbstate_t *);
#ifdef _MB_CAPABLE
int __utf8_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		   const char *, mbstate_t *);
int __sjis_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		   const char *, mbstate_t *);
int __eucjp_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		    const char *, mbstate_t *);
int __jis_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		  const char *, mbstate_t *);
int __iso_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		  const char *, mbstate_t *);
int __cp_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		 const char *, mbstate_t *);
#ifdef __CYGWIN__
int __gbk_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		  const char *, mbstate_t *);
int __kr_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		  const char *, mbstate_t *);
int __big5_mbtowc (struct _reent *, wchar_t *, const char *, size_t,
		 const char *, mbstate_t *);
#endif
#endif

extern wchar_t __iso_8859_conv[14][0x60];
int __iso_8859_index (const char *);

extern wchar_t __cp_conv[][0x80];
int __cp_index (const char *);

#endif
