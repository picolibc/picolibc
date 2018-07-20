#include <wctype.h>
#include <stdint.h>
#include "categories.h"

struct _category {
  enum category cat: 8;
  uint_least32_t first: 24;
  uint_least16_t delta;
} __attribute__((packed));

static const struct _category categories[] = {
#include "categories.t"
};

static enum category
bisearch_cat(wint_t ucs, const struct _category *table, int max)
{
  int min = 0;
  int mid;

  if (ucs < table[0].first || ucs > table[max].first + table[max].delta)
    return 0;
  while (max >= min)
    {
      mid = (min + max) / 2;
      if (ucs > table[mid].first + table[mid].delta)
	min = mid + 1;
      else if (ucs < table[mid].first)
	max = mid - 1;
      else
	return table[mid].cat;
    }
  return -1;
}

enum category category(wint_t ucs)
{
  return bisearch_cat(ucs, categories,
		      sizeof(categories) / sizeof(*categories) - 1);
}
