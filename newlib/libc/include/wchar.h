#ifndef _WCHAR_H_
#define _WCHAR_H_

#include <_ansi.h>

#define __need_size_t
#define __need_wchar_t
#define __need_wint_t
#include <stddef.h>

/* For _mbstate_t definition. */
#include <sys/_types.h>

#ifndef WEOF
# define WEOF (0xffffffffu)
#endif

#ifndef MBSTATE_T
#define MBSTATE_T
typedef _mbstate_t mbstate_t;
#endif /* MBSTATE_T */

wint_t btowc (int c);
int wctob (wint_t c);
int mbsinit(const mbstate_t *ps);
size_t mbrlen(const char *s, size_t n, mbstate_t *ps);
size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps);
size_t mbsrtowcs(wchar_t *dst, const char **src, size_t len, mbstate_t *ps);
size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps);
wchar_t *wmemchr(const wchar_t *ws, wchar_t wc, size_t n);
int wmemcmp(const wchar_t *ws1, const wchar_t *ws2, size_t n);
wchar_t *wmemcpy(wchar_t *ws1, const wchar_t *ws2, size_t n);
wchar_t *wmemmove(wchar_t *ws1, const wchar_t *ws2, size_t n);
wchar_t *wmemset(wchar_t *ws, wchar_t wc, size_t n);

#endif /* _WCHAR_H_ */
