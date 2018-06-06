#ifndef _LIBDL_H_
#define _LIBDL_H_

#define _GNU_SOURCE

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>

/* XXX use the proper syscall numbers */

/* __NR_sched_setattr number */
#ifndef __NR_sched_setattr
#ifdef __x86_64__
#define __NR_sched_setattr 314
#endif

#ifdef __i386__
#define __NR_sched_setattr 351
#endif

#ifdef __arm__
#define __NR_sched_setattr 380
#endif
#endif /* __NR_sched_setattr */

/* __NR_sched_getattr number */
#ifndef __NR_sched_getattr
#ifdef __x86_64__
#define __NR_sched_getattr 315
#endif

#ifdef __i386__
#define __NR_sched_getattr 352
#endif

#ifdef __arm__
#define __NR_sched_getattr 381
#endif
#endif /* __NR_sched_getattr */

struct sched_attr {
    __u32 size;

    __u32 sched_policy;
    __u64 sched_flags;

    /* SCHED_NORMAL, SCHED_BATCH */
    __s32 sched_nice;

    /* SCHED_FIFO, SCHED_RR */
    __u32 sched_priority;

    /* SCHED_DEADLINE */
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags);

int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size,
                  unsigned int flags);

pid_t gettid(void);

#endif /* _LIBDL_H_ */
