#ifndef	_MACHTYPES_H_
#define	_MACHTYPES_H_

#define	_CLOCK_T_	unsigned long		/* clock() */
#define	_TIME_T_	long			/* time() */
#define _CLOCKID_T_ 	unsigned long
#define _TIMER_T_   	unsigned long

#ifndef _HAVE_SYSTYPES
typedef long int __off_t;
typedef int __pid_t;
#ifdef __GNUC__
__extension__ typedef long long int __loff_t;
#else
typedef long int __loff_t;
#endif
#endif

#endif	/* _MACHTYPES_H_ */


