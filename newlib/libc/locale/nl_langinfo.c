/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _DEFAULT_SOURCE
#include "locale_private.h"
#include <langinfo.h>

char *
nl_langinfo_l (nl_item item, locale_t locale)
{
    switch (item) {
    case CODESET:
        return (char *) __locale_name(locale->id[LC_CTYPE]);
    case D_T_FMT:
        return TIME_C_FMT;
    case D_FMT:
        return TIME_X_FMT;
    case T_FMT:
        return TIME_UX_FMT;
    case T_FMT_AMPM:
        return TIME_AMPM_FMT;
    case AM_STR:
        return (char *) TIME_AM_PM[0];
    case PM_STR:
        return (char *) TIME_AM_PM[1];
    case DAY_1:
    case DAY_2:
    case DAY_3:
    case DAY_4:
    case DAY_5:
    case DAY_6:
    case DAY_7:
        return (char *) TIME_WEEKDAY[item-DAY_1];
    case ABDAY_1:
    case ABDAY_2:
    case ABDAY_3:
    case ABDAY_4:
    case ABDAY_5:
    case ABDAY_6:
    case ABDAY_7:
        return (char *) TIME_WDAY[item-ABDAY_1];
    case MON_1:
    case MON_2:
    case MON_3:
    case MON_4:
    case MON_5:
    case MON_6:
    case MON_7:
    case MON_8:
    case MON_9:
    case MON_10:
    case MON_11:
    case MON_12:
        return (char *) TIME_MONTH[item-MON_1];
    case ABMON_1:
    case ABMON_2:
    case ABMON_3:
    case ABMON_4:
    case ABMON_5:
    case ABMON_6:
    case ABMON_7:
    case ABMON_8:
    case ABMON_9:
    case ABMON_10:
    case ABMON_11:
    case ABMON_12:
        return (char *) TIME_MON[item-ABMON_1];
    case ERA:
        return TIME_ERA;
    case ERA_D_FMT:
        return TIME_ERA_D_FMT;
    case ERA_D_T_FMT:
        return TIME_ERA_D_T_FMT;
    case ERA_T_FMT:
        return TIME_ERA_T_FMT;
    case ALT_DIGITS:
        return TIME_ALT_DIGITS;
    case RADIXCHAR:
        return DECIMAL_POINT;
    case THOUSEP:
        return THOUSANDS_SEP;
    case YESEXPR:
        return "^[yY]";
    case NOEXPR:
        return "^[nN]";
    case CRNCYSTR:
        return "";
    default:
        return NULL;
    }
}

char *
nl_langinfo (nl_item item)
{
    return nl_langinfo_l (item, __get_current_locale());
}
