/*
 * month_lengths.c
 *
 * Array __month_lengths[] is (indirectly) needed by tzset(), mktime(),
 * gmtime() and localtime(). To break any dependencies, this array is moved to
 * separate source file.
 */

#include "local.h"

const int __month_lengths[2][MONSPERYEAR] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
} ;
