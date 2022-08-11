/*
FUNCTION
	<<newlocale>>---create or modify a locale object

INDEX
	newlocale

INDEX
	_newlocale_r

SYNOPSIS
	#include <locale.h>
	locale_t newlocale(int <[category_mask]>, const char *<[locale]>,
			   locale_t <[locobj]>);

	locale_t _newlocale_r(void *<[reent]>, int <[category_mask]>,
			      const char *<[locale]>, locale_t <[locobj]>);

DESCRIPTION
The <<newlocale>> function shall create a new locale object or modify an
existing one.  If the base argument is (locale_t) <<0>>, a new locale
object shall be created. It is unspecified whether the locale object
pointed to by base shall be modified, or freed and a new locale object
created.

The category_mask argument specifies the locale categories to be set or
modified.  Values for category_mask shall be constructed by a
bitwise-inclusive OR of the symbolic constants LC_CTYPE_MASK,
LC_NUMERIC_MASK, LC_TIME_MASK, LC_COLLATE_MASK, LC_MONETARY_MASK, and
LC_MESSAGES_MASK, or any of the other implementation-defined LC_*_MASK
values defined in <locale.h>.

For each category with the corresponding bit set in category_mask the
data from the locale named by locale shall be used. In the case of
modifying an existing locale object, the data from the locale named by
locale shall replace the existing data within the locale object. If a
completely new locale object is created, the data for all sections not
requested by category_mask shall be taken from the default locale.

The following preset values of locale are defined for all settings of
category_mask:

"POSIX"	Specifies the minimal environment for C-language translation
called the POSIX locale.

"C"	Equivalent to "POSIX".

""	Specifies an implementation-defined native environment.  This
	corresponds to the value of the associated environment variables,
	LC_* and LANG; see the Base Definitions volume of POSIX.1‐2008,
	Chapter 7, Locale and Chapter 8, Environment Variables.

If the base argument is not (locale_t) <<0>> and the <<newlocale>>
function call succeeds, the contents of base are unspecified.
Applications shall ensure that they stop using base as a locale object
before calling <<newlocale>>.  If the function call fails and the base
argument is not (locale_t) <<0>>, the contents of base shall remain
valid and unchanged.

The behavior is undefined if the base argument is the special locale
object LC_GLOBAL_LOCALE, or is not a valid locale object handle and is
not (locale_t) <<0>>.

RETURNS
Upon successful completion, the <<newlocale>> function shall return a
handle which the caller may use on subsequent calls to <<duplocale>>,
<<freelocale>>, and other functions taking a locale_t argument.

Upon failure, the <<newlocale>> function shall return (locale_t) <<0>>
and set errno to indicate the error.

PORTABILITY
<<newlocale>> is POSIX-1.2008.
*/

#include <newlib.h>
#include <errno.h>
#include <reent.h>
#include <stdlib.h>
#include "setlocale.h"

#define LC_VALID_MASK	(LC_COLLATE_MASK | LC_CTYPE_MASK | LC_MONETARY_MASK \
			 | LC_NUMERIC_MASK | LC_TIME_MASK | LC_MESSAGES_MASK)

struct __locale_t *
_newlocale_r (struct _reent *p, int category_mask, const char *locale,
	      struct __locale_t *base)
{
#ifndef _MB_CAPABLE
  return __get_C_locale ();
#else /* _MB_CAPABLE */
  char new_categories[_LC_LAST][ENCODING_LEN + 1];
  struct __locale_t tmp_locale, *new_locale;
  int i;

  /* Convert LC_ALL_MASK to a mask containing all valid MASK values.
     This simplifies the code below. */
  if (category_mask & LC_ALL_MASK)
    {
      category_mask &= ~LC_ALL_MASK;
      category_mask |= LC_VALID_MASK;
    }
  /* Check for invalid mask values and valid locale ptr. */
  if ((category_mask & ~LC_VALID_MASK) || !locale)
    {
      p->_errno = EINVAL;
      return NULL;
    }
  /* If the new locale is supposed to be all default locale, just return
     a pointer to the default locale. */
  if ((!base && category_mask == 0)
      || (category_mask == LC_VALID_MASK
	  && (!strcmp (locale, "C") || !strcmp (locale, "POSIX"))))
    return __get_C_locale ();
  /* Start with setting all values to the default locale values. */
  tmp_locale = *__get_C_locale ();
  /* Fill out new category strings. */
  for (i = 1; i < _LC_LAST; ++i)
    {
      if (((1 << i) & category_mask) != 0)
	{
	  /* If locale is "", fetch from environment.  Otherwise use locale
	     name verbatim. */
	  const char *cat = (locale[0] == '\0') ? __get_locale_env (p, i)
						: locale;
	  if (strlen (cat) > ENCODING_LEN)
	    {
	      p->_errno = EINVAL;
	      return NULL;
	    }
	  strcpy (new_categories[i], cat);
	}
      else
	strcpy (new_categories[i], base ? base->categories[i] : "C");
    }
  /* Now go over all categories and set them. */
  for (i = 1; i < _LC_LAST; ++i)
    {
      /* If we have a base locale, and the category is not in category_mask
	 or the new category is the base categroy, just copy over. */
      if (base && (((1 << i) & category_mask) == 0
		   || !strcmp (base->categories[i], new_categories[i])))
	{
	  strcpy (tmp_locale.categories[i], new_categories[i]);
	  if (i == LC_CTYPE)
	    {
	      tmp_locale.wctomb = base->wctomb;
	      tmp_locale.mbtowc = base->mbtowc;
	      tmp_locale.cjk_lang = base->cjk_lang;
	      tmp_locale.ctype_ptr = base->ctype_ptr;
	    }
#ifdef __HAVE_LOCALE_INFO__
	  /* Mark the values as "has still to be copied".  We do this in
	     two steps to simplify freeing new locale types in case of a
	     subsequent error. */
	  tmp_locale.lc_cat[i].ptr = base->lc_cat[i].ptr;
	  tmp_locale.lc_cat[i].buf = (void *) -1;
#else /* !__HAVE_LOCALE_INFO__ */
	  if (i == LC_CTYPE)
	    strcpy (tmp_locale.ctype_codeset, base->ctype_codeset);
	  else if (i == LC_MESSAGES)
	    strcpy (tmp_locale.message_codeset, base->message_codeset);
#endif /* !__HAVE_LOCALE_INFO__ */
	}
      /* Otherwise, if the category is in category_mask, create entry. */
      else if (((1 << i) & category_mask) != 0)
	{
	  /* Nothing to do for "C"/"POSIX" locale. */
	  if (!strcmp (new_categories[i], "C")
	      || !strcmp (new_categories[i], "POSIX"))
	    continue;
	  /* Otherwise load locale data. */
	  else if (!__loadlocale (&tmp_locale, i, new_categories[i]))
	    goto error;
	}
    }
  /* Allocate new locale_t. */
  new_locale = (struct __locale_t *) _calloc_r (p, 1, sizeof *new_locale);
  if (!new_locale)
    goto error;
  if (base)
    {
#ifdef __HAVE_LOCALE_INFO__
      /* Step 2 of copying over..  Make sure to invalidate the copied buffer
	 pointers in base, so the subsequent _freelocale_r (base) doesn't free
	 the buffers now used in the new locale. */
      for (i = 1; i < _LC_LAST; ++i)
	if (tmp_locale.lc_cat[i].buf == (const void *) -1)
	  {
	    tmp_locale.lc_cat[i].buf = base->lc_cat[i].buf;
	    if (base != __get_C_locale ())
	      base->lc_cat[i].ptr = base->lc_cat[i].buf = NULL;
	  }
#endif /* __HAVE_LOCALE_INFO__ */
      _freelocale_r (p, base);
    }

  *new_locale = tmp_locale;
  return new_locale;

error:
  /* An error occured while we had already (potentially) allocated memory.
     Free memory and return NULL.  errno is supposed to be set already. */
#ifdef __HAVE_LOCALE_INFO__
  for (i = 1; i < _LC_LAST; ++i)
    if (((1 << i) & category_mask) != 0
	&& tmp_locale.lc_cat[i].buf
	&& tmp_locale.lc_cat[i].buf != (const void *) -1)
      {
	_free_r (p, (void *) tmp_locale.lc_cat[i].ptr);
	_free_r (p, tmp_locale.lc_cat[i].buf);
      }
#endif /* __HAVE_LOCALE_INFO__ */

  return NULL;
#endif /* _MB_CAPABLE */
}

struct __locale_t *
newlocale (int category_mask, const char *locale, struct __locale_t *base)
{
  return _newlocale_r (_REENT, category_mask, locale, base);
}
