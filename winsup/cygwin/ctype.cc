#include "winsup.h"
extern "C" {
#include <ctype.h>
#include <stdlib.h>
#include <wctype.h>

#define _CTYPE_DATA_0_127 \
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C, \
	_C,	_B|_C|_S, _C|_S, _C|_S,	_C|_S,	_C|_S,	_C,	_C, \
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C, \
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_N,	_N,	_N,	_N,	_N,	_N,	_N,	_N, \
	_N,	_N,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P, \
	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C

#define _CTYPE_DATA_128_256 \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0 

/* FIXME: These tables should rather be defined in newlib and we should
   switch to the newer __ctype_ptr method from newlib for new applications. */

static char __ctype_default[128] = { _CTYPE_DATA_128_256 };
static char __ctype_iso[15][128] = {
  /* ISO-8859-1 */
 {      _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C,
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C,
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C,
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C,
        _S|_B,  _P,     _P,     _P,     _P,     _P,     _P,     _P,
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P,
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P,
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P,
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U,
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U,
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _P,
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _L,
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L,
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L,
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _P,
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L },
  /* ISO-8859-2 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_U,	_P,	_U,	_P,	_U,	_U,	_P, 
	_P,	_U,	_U,	_U,	_U,	_P,	_U,	_U, 
	_P,	_L,	_P,	_L,	_P,	_L,	_L,	_P, 
	_P,	_L,	_L,	_L,	_L,	_P,	_L,	_L, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _P, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _P, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L },
  /* ISO-8859-3 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_U,	_P,	_P,	_P,	0,	_U,	_P, 
	_P,	_U,	_U,	_U,	_U,	_P,	0,	_U, 
	_P,	_L,	_P,	_P,	_P,	_L,	_L,	_P, 
	_P,	_L,	_L,	_L,	_L,	_P,	0,	_L, 
	_U,	_U,	_U,	0,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	0,	_U,	_U,	_U,	_U,	_U,	_U,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	0,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	0,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P },
  /* ISO-8859-4 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_U,	_L,	_U,	_P,	_U,	_U,	_P, 
	_P,	_U,	_U,	_U,	_U,	_P,	_U,	_P, 
	_P,	_L,	_P,	_L,	_P,	_L,	_L,	_P, 
	_P,	_L,	_L,	_L,	_L,	_P,	_L,	_L, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _P, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _P, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L },
  /* ISO-8859-5 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_P,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _P,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _P,     _L,     _L },
  /* ISO-8859-6 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	0,	0,	0,	_P,	0,	0,	0, 
	0,	0,	0,	0,	_P,	_P,	0,	0, 
	0,	0,	0,	0,	0,	0,	0,	0, 
	0,	0,	0,	_P,	0,	0,	0,	_P, 
	0,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	0,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	0,	0,	0,	0,	0, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	0,	0,	0,	0,	0, 
	0,	0,	0,	0,	0,	0,	0,	0 },
  /* ISO-8859-7 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_U,	_P, 
	_U,	_U,	_U,	_P,	_U,	_P,	_U,	_U, 
	_L,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P },
  /* ISO-8859-8 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	0,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	0, 
	0,	0,	0,	0,	0,	0,	0,	0, 
	0,	0,	0,	0,	0,	0,	0,	0, 
	0,	0,	0,	0,	0,	0,	0,	0, 
	0,	0,	0,	0,	0,	0,	0,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	0,	0,	_P,	_P,	0 },
  /* ISO-8859-9 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L },
  /* ISO-8859-10 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_U,	_U,	_U,	_U,	_U,	_U,	_P, 
	_U,	_U,	_U,	_U,	_U,	_P,	_U,	_U, 
	_P,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_L,	_P,	_L,	_L, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L },
  /* ISO-8859-11 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_P,	_L,	_L,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	0,	0,	0,	0,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	0,	0,	0,	0 },
  /* ISO-8859-13 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_U,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_L,	_P,	_P,	_P,	_P,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P },
  /* ISO-8859-14 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_U,	_L,	_P,	_U,	_L,	_U,	_P, 
	_U,	_P,	_U,	_L,	_U,	_P,	_P,	_U, 
	_U,	_L,	_U,	_L,	_U,	_L,	_P,	_U, 
	_L,	_L,	_L,	_U,	_L,	_U,	_L,	_L, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L },
  /* ISO-8859-15 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _S|_B,  _P,     _P,     _P,     _P,     _P,     _P,     _P, 
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, 
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, 
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _P, 
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _P, 
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L },
  /* ISO-8859-16 */
  {	_C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, 
	_S|_B,	_U,	_L,	_U,	_P,	_P,	_U,	_P, 
	_L,	_P,	_U,	_P,	_U,	_P,	_L,	_U, 
	_P,	_P,	_U,	_U,	_U,	_P,	_P,	_P, 
	_L,	_L,	_L,	_P,	_U,	_L,	_U,	_L, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L }
};
static char __ctype_cp[22][128] = {
  /* CP437 */
  {	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_U,	_U, 
	_U,	_L,	_U,	_L,	_L,	_L,	_L,	_L, 
	_L,	_U,	_U,	_P,	_P,	_P,	_P,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_L,	_U,	_L,	_U,	_L,	_P,	_L, 
	_U,	_U,	_U,	_L,	_P,	_L,	_L,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP720 */
  {	0,	0,	_L,	_L,	0,	_L,	0,	_L, 
	_L,	_L,	_L,	_L,	_L,	0,	0,	0, 
	0,	_P,	_P,	_L,	_P,	_P,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	0,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP737 */
  {	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_P,	_P,	_P,	_P,	_U,	_U,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP775 */
  {	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_U,	_L,	_L,	_U,	_U,	_U, 
	_U,	_L,	_U,	_L,	_L,	_U,	_P,	_U, 
	_L,	_U,	_U,	_P,	_P,	_P,	_P,	_P, 
	_U,	_U,	_L,	_U,	_L,	_L,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_U,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_U,	_U,	_U, 
	_U,	_P,	_P,	_P,	_P,	_U,	_U,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_U,	_U, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_U, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_U,	_L,	_U,	_U,	_L,	_U,	_L,	_L, 
	_U,	_L,	_U,	_L,	_L,	_U,	_U,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP850 */
  {	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_U,	_U, 
	_U,	_L,	_U,	_L,	_L,	_L,	_L,	_L, 
	_L,	_U,	_U,	_L,	_P,	_U,	_P,	_P, 
	_L,	_L,	_L,	_L,	_L,	_U,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_U,	_U,	_U, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_L,	_U, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_U,	_U,	_U,	_U,	_L,	_U,	_U, 
	_U,	_P,	_P,	_P,	_P,	_P,	_U,	_P, 
	_U,	_L,	_U,	_U,	_L,	_U,	_L,	_U, 
	_L,	_U,	_U,	_U,	_L,	_U,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP852 */
  {	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_U,	_L,	_L,	_U,	_U,	_U, 
	_U,	_U,	_L,	_L,	_L,	_U,	_L,	_U, 
	_L,	_U,	_U,	_U,	_L,	_U,	_P,	_L, 
	_L,	_L,	_L,	_L,	_U,	_L,	_U,	_L, 
	_U,	_L,	_P,	_L,	_U,	_L,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_U,	_U,	_U, 
	_U,	_P,	_P,	_P,	_P,	_U,	_L,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_U,	_L, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_U,	_U,	_U,	_L,	_U,	_U,	_U, 
	_L,	_P,	_P,	_P,	_P,	_U,	_U,	_P, 
	_U,	_L,	_U,	_U,	_L,	_L,	_U,	_L, 
	_U,	_U,	_L,	_U,	_L,	_U,	_L,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_L,	_U,	_L,	_P,	_S|_B },
  /* CP855 */
  {	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, 
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, 
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, 
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, 
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, 
	_L,	_U,	_L,	_U,	_L,	_U,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_L,	_U,	_L, 
	_U,	_P,	_P,	_P,	_P,	_L,	_U,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_L,	_U, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_U,	_L,	_U,	_L,	_U,	_L,	_U, 
	_L,	_P,	_P,	_P,	_P,	_U,	_L,	_P, 
	_U,	_L,	_U,	_L,	_U,	_L,	_U,	_L, 
	_U,	_L,	_U,	_L,	_U,	_L,	_U,	_P, 
	_P,	_L,	_U,	_L,	_U,	_L,	_U,	_L, 
	_U,	_L,	_U,	_L,	_U,	_P,	_P,	_S|_B },
  /* CP857 */
  {	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_U,	_U, 
	_U,	_L,	_U,	_L,	_L,	_L,	_L,	_L, 
	_U,	_U,	_U,	_L,	_P,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_U,	_U,	_L, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_U,	_U,	_U,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_L,	_U, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_U,	_U,	_U,	_L,	_U,	_U, 
	_U,	_P,	_P,	_P,	_P,	_P,	_U,	_P, 
	_U,	_L,	_U,	_U,	_L,	_U,	_L,	_L, 
	_P,	_U,	_U,	_U,	_L,	_L,	_P,	_P, 
	_P,	_P,	_L,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP858 */
  {	_U,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_U,	_U, 
	_U,	_L,	_U,	_L,	_L,	_L,	_L,	_L, 
	_L,	_U,	_U,	_L,	_P,	_U,	_P,	_P, 
	_L,	_L,	_L,	_L,	_L,	_U,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_U,	_U,	_U, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_L,	_U, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_U,	_U,	_U,	_U,	_P,	_U,	_U, 
	_U,	_P,	_P,	_P,	_P,	_P,	_U,	_P, 
	_U,	_L,	_U,	_U,	_L,	_U,	_L,	_U, 
	_L,	_U,	_U,	_U,	_L,	_U,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP862 */
  {	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_P, 
	_L,	_L,	_L,	_L,	_L,	_U,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_L,	_U,	_L,	_U,	_L,	_P,	_L, 
	_U,	_U,	_U,	_L,	_P,	_L,	_L,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP866 */
  {	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_U,	_L,	_U,	_L,	_U,	_L,	_U,	_L, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP874 */
  {	_P,	0,	0,	0,	0,	_P,	0,	0, 
	0,	0,	0,	0,	0,	0,	0,	0, 
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	0,	0,	0,	0,	0,	0,	0,	0, 
	_S|_B,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	0,	0,	0,	0,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_L,	_L,	0,	0,	0,	0 },
  /* CP1125 */
  {	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_U,	_L,	_U,	_L,	_U,	_L,	_U,	_L, 
	_U,	_L,	_P,	_P,	_P,	_P,	_P,	_S|_B },
  /* CP1250 */
  {	_P,	0,	_P,	0,	_P,	_P,	_P,	_P, 
	0,	_P,	_U,	_P,	_U,	_U,	_U,	_U, 
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	0,	_P,	_L,	_P,	_L,	_L,	_L,	_L, 
	_S|_B,	_P,	_P,	_U,	_P,	_U,	_P,	_P, 
	_P,	_P,	_U,	_P,	_P,	_P,	_P,	_U, 
	_P,	_P,	_P,	_L,	_P,	_P,	_P,	_P, 
	_P,	_L,	_L,	_P,	_U,	_P,	_L,	_L, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P },
  /* CP1251 */
  {	_U,	_U,	_P,	_L,	_P,	_P,	_P,	_P, 
	_P,	_P,	_U,	_P,	_U,	_U,	_U,	_U, 
	_L,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_P,	_L,	_L,	_L,	_L,	_P,	_U, 
	_S|_B,	_U,	_L,	_U,	_P,	_U,	_P,	_P, 
	_U,	_P,	_U,	_P,	_P,	_P,	_P,	_U, 
	_P,	_P,	_U,	_L,	_L,	_P,	_P,	_P, 
	_L,	_P,	_L,	_P,	_L,	_U,	_L,	_L, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L },
  /* CP1252 */
  {	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, 
	_P,	_P,	_U,	_P,	_U,	_U,	0,	0, 
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_L,	_P,	_L,	0,	_L,	_U, 
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L },
  /* CP1253 */
  {	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, 
	0,	_P,	0,	_P,	0,	0,	0,	0, 
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	0,	_P,	_P,	0,	0,	0,	0,	0, 
	_S|_B,	_P,	_U,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	0,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_U,	_U,	_U,	_P,	_U,	_P,	_U,	_U, 
	_L,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L },
  /* CP1254 */
  {	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, 
	_P,	_P,	_U,	_P,	_U,	0,	0,	0, 
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_L,	_P,	_L,	0,	0,	_U, 
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L },
  /* CP1255 */
  {	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, 
	_P,	_P,	0,	_P,	0,	0,	0,	0, 
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	0,	_P,	0,	0,	0,	0, 
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	0,	0,	0,	0,	0,	0,	0, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
	_L,	_L,	_L,	0,	0,	_P,	_P,	0 },
  /* CP1256 */
  {	_P,	_L,	_P,	_L,	_P,	_P,	_P,	_P, 
	_P,	_P,	_L,	_P,	_U,	_L,	_L,	_L, 
	_L,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_P,	_L,	_P,	_L,	_P,	_P,	_L, 
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_L,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_P,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
	_P,	_P,	_P,	_P,	_L,	_P,	_P,	_P, 
	_P,	_L,	_P,	_L,	_L,	_P,	_P,	_L },
  /* CP1257 */
  {	_P,	0,	_P,	0,	_P,	_P,	_P,	_P, 
	0,	_P,	0,	_P,	0,	_P,	_P,	_P, 
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	0,	_P,	0,	_P,	0,	_P,	_P,	0, 
	_S|_B,	0,	_P,	_P,	_P,	0,	_P,	_P, 
	_U,	_P,	_U,	_P,	_P,	_P,	_P,	_U, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_L,	_P,	_L,	_P,	_P,	_P,	_P,	_L, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_P },
  /* CP1258 */
  {	_P,	0,	_P,	_L,	_P,	_P,	_P,	_P, 
	_P,	_P,	0,	_P,	_U,	0,	0,	0, 
	0,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	0,	_P,	_L,	0,	0,	_U, 
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, 
	_U,	_U,	_U,	_U,	_P,	_U,	_U,	_U, 
	_U,	_U,	_P,	_U,	_U,	_U,	_U,	_P, 
	_U,	_U,	_U,	_U,	_U,	_U,	_P,	_L,
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, 
	_L,	_L,	_L,	_L,	_P,	_L,	_L,	_L, 
	_L,	_L,	_P,	_L,	_L,	_L,	_L,	_P, 
	_L,	_L,	_L,	_L,	_L,	_L,	_P,	_L }
};

char ctype_b[128 + 256] = {
	_CTYPE_DATA_128_256,
	_CTYPE_DATA_0_127,
	_CTYPE_DATA_128_256
};

__asm__ ("					\n\
	.data					\n\
	.globl	__ctype_			\n\
	.set    __ctype_,_ctype_b+127		\n\
	.text					\n\
");

#define makefunc(x) \
  static int __cdecl \
  c_##x (int c) \
  { \
    return x (c); \
  } \
  EXPORT_ALIAS(c_##x, x)

makefunc(isalnum)
makefunc(isalpha)
makefunc(iscntrl)
makefunc(isdigit)
makefunc(isgraph)
makefunc(islower)
makefunc(isprint)
makefunc(ispunct)
makefunc(isspace)
makefunc(isupper)
makefunc(isxdigit)
makefunc(isblank)
makefunc(isascii)
makefunc(toascii)

static int __cdecl
c_tolower (int c)
{
  if ((unsigned char) c <= 0x7f)
    return isupper (c) ? c + 0x20 : c;

  char s[8] = { c, '\0' };
  wchar_t wc;
  if (mbtowc (&wc, s, 1) >= 0
      && wctomb (s, (wchar_t) towlower ((wint_t) wc)) == 1)
    c = s[0];
  return c;
}
EXPORT_ALIAS(c_tolower, tolower)

static int __cdecl
c_toupper (int c)
{
  if ((unsigned char) c <= 0x7f)
    return islower (c) ? c - 0x20 : c;

  char s[8] = { c, '\0' };
  wchar_t wc;
  if (mbtowc (&wc, s, 1) >= 0
      && wctomb (s, (wchar_t) towupper ((wint_t) wc)) == 1)
    c = s[0];
  return c;
}
EXPORT_ALIAS(c_toupper, toupper)

/* Called from newlib's setlocale().  What we do here is to copy the
   128 bytes of charset specific ctype data into the array at _ctype_b.
   Given that the functionality is usually implemented locally in the
   application, that's the only backward compatible way to do it.
   Setlocale is usually only called once in an application, so this isn't
   time-critical anyway. */
int __iso_8859_index (const char *charset_ext);	/* Newlib */
int __cp_index (const char *charset_ext);	/* Newlib */

void
__set_ctype (const char *charset)
{
  int idx;

  switch (*charset)
    {
    case 'I':
      idx = __iso_8859_index (charset + 9);
      /* Our ctype table has a leading ISO-8859-1 element. */
      if (idx < 0)
      	idx = 0;
      else
	++idx;
      memcpy (ctype_b, __ctype_iso[idx], 128);
      memcpy (ctype_b + 256, __ctype_iso[idx], 128);
      return;
    case 'C':
      idx = __cp_index (charset + 2);
      if (idx < 0)
	break;
      memcpy (ctype_b, __ctype_cp[idx], 128);
      memcpy (ctype_b + 256, __ctype_cp[idx], 128);
      return;
    default:
      break;
    }
  memcpy (ctype_b, __ctype_default, 128);
  memcpy (ctype_b + 256, __ctype_default, 128);
}

} /* extern "C" */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

