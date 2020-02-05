/*
Copyright (c) 2017 Thomas Wolff towo@towo.net
 */
/* category data */

enum category {
#include "categories.cat"
};

extern enum category category(wint_t ucs);
