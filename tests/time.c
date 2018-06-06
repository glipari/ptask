#include <assert.h>
#include <ptime.h>
#include <stdio.h>

int main() {
    long delta1, delta2, delta3;
    tspec t1, t2, t3;

    tspec_init();

    delta1 = ptask_gettime(MICRO);

    t1 = tspec_from(delta1, MICRO);

    // tests adding zero
    t2 = tspec_add_delta(&t1, 0, MILLI);
    assert(tspec_cmp(&t1, &t2) == 0);

    // tests adding 0 microseconds
    t1 = tspec_add_delta(&t1, 0, MICRO);
    assert(tspec_cmp(&t1, &t2) == 0);

    // tests adding and a number of milliseconds
    t1 = tspec_add_delta(&t1, 12345678, MICRO);
    t2 = tspec_add_delta(&t2, 345678, MICRO);
    t2 = tspec_add_delta(&t2, 6000000, MICRO);
    t2 = tspec_add_delta(&t2, 6000000, MICRO);
    assert(tspec_cmp(&t1, &t2) == 0);

    t2 = tspec_add_delta(&t2, 1, MICRO);
    assert(tspec_cmp(&t1, &t2) == -1);

    delta2 = ptask_gettime(NANO);

    // test conversion
    t1 = tspec_get_ref();
    t1.tv_sec += 1;
    t2 = tspec_from_rel(1, SEC);
    // printf("t2 = {%ld, %ld}\n", t2.tv_sec, t2.tv_nsec);
    t3 = tspec_from_rel(1, MILLI);
    assert(tspec_cmp(&t1, &t2) == 0);
    assert(tspec_cmp(&t1, &t3) != 0);
    delta3 = tspec_to_rel(&t1, MILLI) / 1000;
    assert(delta3 == 1);

    delta3 = tspec_to_rel(&t1, MICRO);
    assert(delta3 == 1000000l);

    delta3 = tspec_to_rel(&t1, NANO);
    assert(delta3 == 1000000000l);

    // test that time progresses
    delta3 = ptask_gettime(NANO);
    assert(delta3 > delta2);

    // test the subtraction
    t2 = tspec_from(delta2, MICRO);
    t3 = tspec_from(delta3, MICRO);
    tspec diff = tspec_sub(&t2, &t3);
    delta1 = tspec_to(&diff, MICRO);
    assert(delta1 == (delta3 - delta2));

    return 0;
}
