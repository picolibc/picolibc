/* local header used by libc/time routines */
#include <_ansi.h>
#include <time.h>

#define SECSPERMIN	60L
#define MINSPERHOUR	60L
#define HOURSPERDAY	24L
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	(SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK	7
#define MONSPERYEAR	12

#define YEAR_BASE	1900
#define EPOCH_YEAR      1970
#define EPOCH_WDAY      4
#define EPOCH_YEARS_SINCE_LEAP 2
#define EPOCH_YEARS_SINCE_CENTURY 70
#define EPOCH_YEARS_SINCE_LEAP_CENTURY 370

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

extern time_t __tzstart_std;
extern time_t __tzstart_dst;
extern int __tznorth;
extern int __tzyear;

typedef struct __tzrule_struct
{
  char ch;
  int m;
  int n;
  int d;
  int s;
  time_t change;
  int offset;
} __tzrule_type;

extern __tzrule_type __tzrule[2];

struct tm * _EXFUN (_mktm_r, (_CONST time_t *, struct tm *, int __is_gmtime));
int         _EXFUN (__tzcalc_limits, (int __year));

/* locks for multi-threading */
#ifdef __SINGLE_THREAD__
#define TZ_LOCK
#define TZ_UNLOCK
#else
#define TZ_LOCK __tz_lock()
#define TZ_UNLOCK __tz_unlock()
#endif

void _EXFUN(__tz_lock,(_VOID));
void _EXFUN(__tz_unlock,(_VOID));

