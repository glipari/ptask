#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER ptask_provider

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./tpt_provider.h"

#if !defined(_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TP_H

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT(
    ptask_provider,
    ptask_tracepoint,
    TP_ARGS(
		int, pid,
		int, tid,
        int, i,
		char*, flag,
		char*, state,
		long, time,
		int, priority,
		int, period,
		int, deadline
    ),
    TP_FIELDS(
		ctf_integer(int, ptask_pid, pid)
        ctf_integer(int, ptask_tid, tid)
        ctf_integer(int, ptask_index, i)
		ctf_string(ptask_flag, flag)
		ctf_string(ptask_state, state)
        ctf_integer(long, ptask_time, time)
        ctf_integer(int, ptask_priority, priority)
        ctf_integer(int, ptask_period, period)
        ctf_integer(int, ptask_deadline, deadline)
    )
)

TRACEPOINT_LOGLEVEL(ptask_provider, ptask_tracepoint, TRACE_INFO)

#endif /* _TP_H */
#include <lttng/tracepoint-event.h>
