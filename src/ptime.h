#ifndef __TIMEUTILS_H__
#define __TIMEUTILS_H__

#include <time.h>

/** This is a shortcut for struct timespec. In this library 
    it represents absolute time, as returned by the clock_gettime() */
typedef struct timespec tspec_t;
/** This represents a time interval. When using this type, 
    you usually have to specify a time unit (from SEC to NANO). */
typedef long ptime_t;

/* time units */
#define	SEC	0
#define	MILLI	1
#define	MICRO	2
#define	NANO	3

/**     Initializes this library   */
void    tspec_init();
/**     Returns the reference time */
tspec_t tspec_get_ref();
/**     Converts to a long expressed in unit                     */
ptime_t tspec_to(const tspec_t *t, int unit);
/**     From a long integer, expressed as unit, into a timespec. */
tspec_t tspec_from(ptime_t tu, int unit);
/**     Converts to a long expressed in units (rel to reference) */
ptime_t tspec_to_rel(const tspec_t *t, int unit);
/**     From units (relative to reference) to a timespec.        */
tspec_t tspec_from_rel(ptime_t tu, int unit);
/**     Returns the current time from the reference, in unit     */
ptime_t tspec_gettime(int unit);
/**     Computes s = a + delta       */
tspec_t tspec_add_delta(const tspec_t *a, ptime_t delta, int units);
/**     Computes s = a + b           */
tspec_t tspec_add(const tspec_t *a, const tspec_t *b);
/**     Compares two timespecs */
int     tspec_cmp(const tspec_t *a, const tspec_t *b);
/**     delta = a - b */
tspec_t tspec_sub(const tspec_t *a, const tspec_t *b);
/**     d = a - delta */
tspec_t tspec_sub_delta(const tspec_t *a, ptime_t delta, int unit);


#endif 
