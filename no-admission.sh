#
# This script disables the admission control for the SCHED_DEADLINE
# scheduling policy.

# It is necessary to launch this script before using a partitioned EDF
# scheduling in Ptask, otherwise the scheduling policy cannot be set.
#

echo -1 > /proc/sys/kernel/sched_rt_runtime_us
