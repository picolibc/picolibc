#if LDBL_MANT_DIG == 64
#include "../ld80/invtrig.h"
#elif LDBL_MANT_DIG == 113
#include "../ld128/invtrig.h"
#else
#error "Unsupported long double format"
#endif
