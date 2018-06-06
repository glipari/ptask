#ifndef __TIMEUTILS_H__
#define __TIMEUTILS_H__

#include <time.h>

/** This is a shortcut for struct timespec. In this library
    it represents absolute time, as returned by the clock_gettime() */
typedef struct timespec tspec;

/** This represents a time interval. When using this type,
    you usually have to specify a time unit (from SEC to NANO). */
typedef long ptime;

/* time units */
#define SEC 0
#define MILLI 1
#define MICRO 2
#define NANO 3

extern const tspec tspec_zero;

/**     Initializes this library   */
void tspec_init();
/**     Returns the reference time */
tspec tspec_get_ref();
/**     Converts to a long expressed in unit                     */
ptime tspec_to(const tspec *t, int unit);
/**     From a long integer, expressed as unit, into a timespec. */
tspec tspec_from(ptime tu, int unit);
/**     Converts to a long expressed in units (rel to reference) */
ptime tspec_to_rel(const tspec *t, int unit);
/**     From units (relative to reference) to a timespec.        */
tspec tspec_from_rel(ptime tu, int unit);

/**     Returns the current time from the reference, in unit     */
ptime ptask_gettime(int unit);

/**     Computes s = a + delta       */
tspec tspec_add_delta(const tspec *a, ptime delta, int units);
/**     Computes s = a + b           */
tspec tspec_add(const tspec *a, const tspec *b);
/**     Compares two timespecs */
int tspec_cmp(const tspec *a, const tspec *b);
/**     delta = a - b */
tspec tspec_sub(const tspec *a, const tspec *b);
/**     d = a - delta */
tspec tspec_sub_delta(const tspec *a, ptime delta, int unit);

#endif
