#include "newlib.h"
#include <reent.h>
#include "setlocale.h"

struct lconv *
__localeconv_l (struct __locale_t *locale)
{
  struct lconv *lconv = &locale->lconv;
  if (locale == __get_C_locale ())
    return lconv;

#ifdef __HAVE_LOCALE_INFO__
  const struct lc_numeric_T *n = __get_numeric_locale (locale);
  const struct lc_monetary_T *m = __get_monetary_locale (locale);

  lconv->decimal_point = (char *) n->decimal_point;
  lconv->thousands_sep = (char *) n->thousands_sep;
  lconv->grouping = (char *) n->grouping;
  lconv->int_curr_symbol = (char *) m->int_curr_symbol;
  lconv->currency_symbol = (char *) m->currency_symbol;
  lconv->mon_decimal_point = (char *) m->mon_decimal_point;
  lconv->mon_thousands_sep = (char *) m->mon_thousands_sep;
  lconv->mon_grouping = (char *) m->mon_grouping;
  lconv->positive_sign = (char *) m->positive_sign;
  lconv->negative_sign = (char *) m->negative_sign;
  lconv->int_frac_digits = m->int_frac_digits[0];
  lconv->frac_digits = m->frac_digits[0];
  lconv->p_cs_precedes = m->p_cs_precedes[0];
  lconv->p_sep_by_space = m->p_sep_by_space[0];
  lconv->n_cs_precedes = m->n_cs_precedes[0];
  lconv->n_sep_by_space = m->n_sep_by_space[0];
  lconv->p_sign_posn = m->p_sign_posn[0];
  lconv->n_sign_posn = m->n_sign_posn[0];
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
  lconv->int_p_cs_precedes = m->int_p_cs_precedes[0];
  lconv->int_p_sep_by_space = m->int_p_sep_by_space[0];
  lconv->int_n_cs_precedes = m->int_n_cs_precedes[0];
  lconv->int_n_sep_by_space = m->int_n_sep_by_space[0];
  lconv->int_n_sign_posn = m->int_n_sign_posn[0];
  lconv->int_p_sign_posn = m->int_p_sign_posn[0];
#else /* !__HAVE_LOCALE_INFO_EXTENDED__ */
  lconv->int_p_cs_precedes = m->p_cs_precedes[0];
  lconv->int_p_sep_by_space = m->p_sep_by_space[0];
  lconv->int_n_cs_precedes = m->n_cs_precedes[0];
  lconv->int_n_sep_by_space = m->n_sep_by_space[0];
  lconv->int_n_sign_posn = m->n_sign_posn[0];
  lconv->int_p_sign_posn = m->p_sign_posn[0];
#endif /* !__HAVE_LOCALE_INFO_EXTENDED__ */
#endif /* __HAVE_LOCALE_INFO__ */
  return lconv;
}

struct lconv *
_DEFUN (_localeconv_r, (data), 
	struct _reent *data)
{
  /* Note that we always fall back to the global locale, even in case
     of specifying a reent.  Otherwise a call to _localeconv_r would just
     crash if the reent locale pointer is NULL. */
  return __localeconv_l (__get_current_locale ());
}

#ifndef _REENT_ONLY
struct lconv *
_DEFUN_VOID (localeconv)
{
  return __localeconv_l (__get_current_locale ());
}
#endif
