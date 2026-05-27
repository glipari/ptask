#include <check.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

/*
 * We simulate the vulnerable allocation pattern from src/rtmode.c:
 *   g->modes = (tasklist_t *)malloc(sizeof(tasklist_t) * nmodes);
 *
 * Security invariant: The allocation size must never overflow size_t,
 * and the allocated buffer must be large enough to hold all nmodes entries
 * without out-of-bounds access.
 *
 * A safe implementation must:
 *   1. Reject nmodes values that would cause sizeof(element) * nmodes to overflow.
 *   2. Only proceed with allocation when the product is representable in size_t.
 *   3. Never write beyond the allocated region.
 */

/* Simulate a tasklist_t structure similar to what rtmode.c would use */
typedef struct {
    int   task_id;
    int   priority;
    void *task_fn;
    char  name[64];
    long  deadline;
    long  period;
} tasklist_t;

/* Safe allocation function that encodes the security invariant:
 * returns NULL if nmodes is 0, too large, or would cause overflow */
static tasklist_t *safe_alloc_modes(size_t nmodes)
{
    /* Reject zero */
    if (nmodes == 0) {
        return NULL;
    }

    /* Check for multiplication overflow before allocating */
    if (nmodes > SIZE_MAX / sizeof(tasklist_t)) {
        /* Overflow would occur — reject */
        return NULL;
    }

    size_t alloc_size = sizeof(tasklist_t) * nmodes;
    tasklist_t *modes = (tasklist_t *)malloc(alloc_size);
    if (modes == NULL) {
        return NULL;
    }

    /* Initialize all entries — this is what rtmode.c does after allocation */
    memset(modes, 0, alloc_size);
    return modes;
}

/* Unsafe allocation function that mirrors the vulnerable code in rtmode.c */
static tasklist_t *unsafe_alloc_modes(size_t nmodes)
{
    /* Vulnerable: no overflow check before multiplication */
    tasklist_t *modes = (tasklist_t *)malloc(sizeof(tasklist_t) * nmodes);
    return modes;
}

START_TEST(test_nmodes_overflow_rejected)
{
    /* Invariant: allocation with nmodes values that would overflow size_t
     * must be rejected (return NULL) and must never succeed with an
     * undersized buffer that would be written beyond its bounds. */

    /* Adversarial nmodes values — large values that cause overflow */
    size_t adversarial_nmodes[] = {
        /* Values that overflow size_t when multiplied by sizeof(tasklist_t) */
        SIZE_MAX,
        SIZE_MAX / sizeof(tasklist_t) + 1,
        SIZE_MAX / sizeof(tasklist_t) + 2,
        SIZE_MAX / 2,
        SIZE_MAX / 4,
        /* Boundary: exactly at overflow threshold */
        (SIZE_MAX / sizeof(tasklist_t)) + 1,
        /* Very large values */
        (size_t)0xFFFFFFFF,
        (size_t)0xFFFFFFFE,
        /* Values that look like 2x, 10x oversized relative to a "normal" max */
        200000000UL,
        1000000000UL,
        /* INT_MAX and UINT_MAX style values */
        (size_t)INT_MAX,
        (size_t)UINT_MAX,
        /* Zero — also invalid */
        0,
    };

    int num_adversarial = (int)(sizeof(adversarial_nmodes) / sizeof(adversarial_nmodes[0]));

    for (int i = 0; i < num_adversarial; i++) {
        size_t nmodes = adversarial_nmodes[i];

        /* The safe implementation must reject overflow-inducing values */
        tasklist_t *result = safe_alloc_modes(nmodes);

        if (nmodes == 0) {
            /* Zero modes must be rejected */
            ck_assert_msg(result == NULL,
                "FAIL: safe_alloc_modes(0) should return NULL but returned non-NULL");
        } else if (nmodes > SIZE_MAX / sizeof(tasklist_t)) {
            /* Overflow-inducing values must be rejected */
            ck_assert_msg(result == NULL,
                "FAIL: safe_alloc_modes(%zu) should return NULL due to overflow "
                "but returned non-NULL (undersized buffer risk)", nmodes);
        } else {
            /* If allocation succeeded for a valid (non-overflowing) large value,
             * it may legitimately fail due to OOM — that is acceptable */
            if (result != NULL) {
                free(result);
            }
        }
    }
}
END_TEST

START_TEST(test_nmodes_overflow_detection)
{
    /* Invariant: the product sizeof(tasklist_t) * nmodes must be detectable
     * as overflowing before any allocation or write occurs. */

    size_t overflow_cases[] = {
        SIZE_MAX,
        SIZE_MAX / sizeof(tasklist_t) + 1,
        SIZE_MAX / sizeof(tasklist_t) + 100,
        SIZE_MAX / 2 + 1,
        (size_t)UINT_MAX + (size_t)1,  /* may wrap on 32-bit, test portably */
    };

    int num_cases = (int)(sizeof(overflow_cases) / sizeof(overflow_cases[0]));

    for (int i = 0; i < num_cases; i++) {
        size_t nmodes = overflow_cases[i];

        /* Verify overflow detection logic */
        int would_overflow = (nmodes > SIZE_MAX / sizeof(tasklist_t));

        if (would_overflow) {
            /* The safe allocator must return NULL */
            tasklist_t *result = safe_alloc_modes(nmodes);
            ck_assert_msg(result == NULL,
                "FAIL: overflow not caught for nmodes=%zu, "
                "sizeof(tasklist_t)=%zu, product would overflow size_t",
                nmodes, sizeof(tasklist_t));
        }
    }
}
END_TEST

START_TEST(test_valid_nmodes_allocation)
{
    /* Invariant: valid (non-overflowing, non-zero) nmodes values must
     * produce a buffer large enough to safely write all entries. */

    size_t valid_nmodes[] = {
        1,
        2,
        10,
        100,
        1000,
        /* Boundary: largest safe value that won't OOM in practice */
        /* We use a small-ish value to avoid actual OOM in test */
        4096,
    };

    int num_valid = (int)(sizeof(valid_nmodes) / sizeof(valid_nmodes[0]));

    for (int i = 0; i < num_valid; i++) {
        size_t nmodes = valid_nmodes[i];

        /* Verify no overflow would occur */
        ck_assert_msg(nmodes <= SIZE_MAX / sizeof(tasklist_t),
            "Test setup error: nmodes=%zu is not actually safe", nmodes);

        tasklist_t *modes = safe_alloc_modes(nmodes);

        if (modes != NULL) {
            /* Write to every entry to verify buffer is large enough */
            for (size_t j = 0; j < nmodes; j++) {
                modes[j].task_id  = (int)j;
                modes[j].priority = 0;
                modes[j].task_fn  = NULL;
                modes[j].deadline = 0L;
                modes[j].period   = 0L;
                memset(modes[j].name, 0, sizeof(modes[j].name));
            }

            /* Verify writes were not corrupted (basic sanity) */
            for (size_t j = 0; j < nmodes; j++) {
                ck_assert_int_eq(modes[j].task_id, (int)j);
            }

            free(modes);
        }
        /* If malloc returns NULL due to OOM, that is acceptable behavior */
    }
}
END_TEST

START_TEST(test_unsafe_vs_safe_overflow_behavior)
{
    /* Invariant: for any nmodes that would cause sizeof(tasklist_t)*nmodes
     * to overflow, the safe allocator must return NULL while the unsafe
     * allocator may return a non-NULL (dangerously undersized) pointer.
     * This test documents the difference and asserts the safe path is taken. */

    /* Pick a value that definitely overflows */
    size_t overflow_nmodes = SIZE_MAX / sizeof(tasklist_t) + 1;

    int would_overflow = (overflow_nmodes > SIZE_MAX / sizeof(tasklist_t));
    ck_assert_msg(would_overflow,
        "Test setup: overflow_nmodes should trigger overflow detection");

    /* Safe allocator must reject this */
    tasklist_t *safe_result = safe_alloc_modes(overflow_nmodes);
    ck_assert_msg(safe_result == NULL,
        "FAIL: safe_alloc_modes must return NULL for overflow-inducing nmodes=%zu",
        overflow_nmodes);

    /* Document: the unsafe allocator (mirroring rtmode.c) may NOT return NULL
     * because the overflow wraps to a small value, causing malloc to succeed
     * with an undersized buffer. We do NOT call it here to avoid heap corruption,
     * but we assert the overflow condition is real. */
    size_t unsafe_product = sizeof(tasklist_t) * overflow_nmodes; /* intentional overflow */
    (void)unsafe_product; /* product is smaller than expected — the bug */

    /* The invariant: safe_result is NULL, proving the safe path rejects it */
    ck_assert_ptr_null(safe_result);
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_nmodes_overflow_rejected);
    tcase_add_test(tc_core, test_nmodes_overflow_detection);
    tcase_add_test(tc_core, test_valid_nmodes_allocation);
    tcase_add_test(tc_core, test_unsafe_vs_safe_overflow_behavior);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}