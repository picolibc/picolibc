/*
Copyright (c) 2017 Thomas Wolff towo@towo.net
 */
/* category data */

#include <stdint.h>

enum category {
#include "categories.cat"
    CAT_error = -1,
};

extern enum category category(uint32_t ucs);
