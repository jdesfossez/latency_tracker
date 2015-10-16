/* subsystem name is "latency_tracker" */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM latency_tracker

#if !defined(_TRACE_LATENCY_TRACKER_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_LATENCY_TRACKER_H

#include <linux/tracepoint.h>
#include <linux/netdevice.h>

TRACE_EVENT(
	latency_tracker_wakeup,
	TP_PROTO(char comm[TASK_COMM_LEN], pid_t pid, u64 delay, unsigned int flag),
	TP_ARGS(comm, pid, delay, flag),
	TP_STRUCT__entry(
		__array(char, comm, TASK_COMM_LEN)
		__field(int, pid)
		__field(u64, delay)
		__field(unsigned int, flag)
	),
	TP_fast_assign(
		memcpy(__entry->comm, comm, TASK_COMM_LEN);
		entry->pid = pid;
		entry->delay = delay;
		entry->flag = flag;
	),
	TP_printk("comm=%s, pid=%d, delay=%llu, flag=%u", __entry->comm,
		__entry->pid, __entry->delay, __entry->flag)
   );

TRACE_EVENT(
	latency_tracker_offcpu_sched_switch,
	TP_PROTO(char comm[TASK_COMM_LEN], pid_t pid, u64 delay,
		unsigned int flag, char stack[256]),
	TP_ARGS(comm, pid, delay, flag, stack),
	TP_STRUCT__entry(
		 __array(char, comm, TASK_COMM_LEN)
		__field(int, pid)
		__field(u64, delay)
		__field(unsigned int, flag)
		__array(char, stack, 256)
		),
	TP_fast_assign(
		memcpy(__entry->comm, comm, TASK_COMM_LEN);
		entry->pid = pid;
		entry->delay = delay;
		entry->flag = flag;
		memcpy(__entry->stack, stack, 256);
		),
	TP_printk("comm=%s, pid=%d, delay=%llu, flag=%u, stack=%s",
		__entry->comm, __entry->pid, __entry->delay, __entry->flag,
		__entry->stack)
   );

TRACE_EVENT(
	latency_tracker_offcpu_sched_wakeup,
	TP_PROTO(struct task_struct *waker, char waker_stack[256],
		struct task_struct *wakee,
		u64 wakee_offcpu_delay, unsigned int flag),
	TP_ARGS(waker, waker_stack,
		wakee,
		wakee_offcpu_delay, flag),
	TP_STRUCT__entry(
		__field(int, waker_pid)
		__array(char, waker_comm, TASK_COMM_LEN)
		__array(char, waker_stack, 256)
		__field(int, wakee_pid)
		__array(char, wakee_comm, TASK_COMM_LEN)
		__field(u64, wakee_offcpu_delay)
		__field(unsigned int, flag)
		),
	TP_fast_assign(
		entry->waker_pid = waker->pid;
		memcpy(__entry->waker_comm, waker->comm, TASK_COMM_LEN);
		memcpy(__entry->waker_stack, waker_stack, 256);
		entry->wakee_pid = wakee->pid;
		memcpy(__entry->wakee_comm, wakee->comm, TASK_COMM_LEN);
		entry->wakee_offcpu_delay = wakee_offcpu_delay;
		entry->flag = flag;
		),
	TP_printk("waker_comm=%s (%d), wakee_comm=%s (%d), wakee_offcpu_delay=%llu, "
			"flag=%u, waker_stack=%s",
		__entry->waker_comm, __entry->waker_pid,
		__entry->wakee_comm, __entry->wakee_pid,
		__entry->wakee_offcpu_delay, __entry->flag,
		__entry->waker_stack)
   );

TRACE_EVENT(
	latency_tracker_syscall_stack,
	TP_PROTO(char comm[TASK_COMM_LEN], pid_t pid, u64 start_ts, u64 delay,
		unsigned int flag, char stack[256]),
	TP_ARGS(comm, pid, start_ts, delay, flag, stack),
	TP_STRUCT__entry(
		__array(char, comm, TASK_COMM_LEN)
		__field(int, pid)
		__field(u64, start_ts)
		__field(u64, delay)
		__field(unsigned int, flag)
		__array(char, stack, 256)
		),
	TP_fast_assign(
		memcpy(__entry->comm, comm, TASK_COMM_LEN);
		entry->pid = pid;
		entry->start_ts = start_ts;
		entry->delay = delay;
		entry->flag = flag;
		memcpy(__entry->stack, stack, 256);
		),
	TP_printk("comm=%s, pid=%d, start_ts=%llu, delay=%llu flag=%u, stack=%s",
		 __entry->comm, __entry->pid, __entry->start_ts,
		 __entry->delay, __entry->flag, __entry->stack)
   );

TRACE_EVENT(
	latency_tracker_syscall,
	TP_PROTO(char comm[TASK_COMM_LEN],
		pid_t pid, u64 start_ts, u64 delay),
	TP_ARGS(comm, pid, start_ts, delay),
	TP_STRUCT__entry(
		__array(char, comm, TASK_COMM_LEN)
		__field(int, pid)
		__field(u64, start_ts)
		__field(u64, delay)
		),
	TP_fast_assign(
		memcpy(__entry->comm, comm, TASK_COMM_LEN);
		entry->pid = pid;
		entry->start_ts = start_ts;
		entry->delay = delay;
		),
	TP_printk("comm=%s, pid=%d, start_ts=%llu, delay=%llu",
		__entry->comm, __entry->pid, __entry->start_ts, __entry->delay)
   );

TRACE_EVENT(
	latency_tracker_syscall_fd,
	TP_PROTO(char comm[TASK_COMM_LEN],
		pid_t pid, u64 start_ts, u64 delay,
		int fd, char path[256]),
	TP_ARGS(comm, pid, start_ts, delay, fd, path),
	TP_STRUCT__entry(
		__array(char, comm, TASK_COMM_LEN)
		__field(int, pid)
		__field(u64, start_ts)
		__field(u64, delay)
		__field(int, fd)
		__array(char, path, 256)
		),
	TP_fast_assign(
		memcpy(__entry->comm, comm, TASK_COMM_LEN);
		entry->pid = pid;
		entry->start_ts = start_ts;
		entry->delay = delay;
		entry->fd = fd;
		memcpy(__entry->path, path, 256);
		),
	TP_printk("comm=%s, pid=%d, start_ts=%llu, delay=%llu, fd=%d, path=%s",
		__entry->comm, __entry->pid, __entry->start_ts, __entry->delay,
		__entry->fd, __entry->path)
   );

TRACE_EVENT(
	latency_tracker_block,
	TP_PROTO(dev_t dev, sector_t sector, u64 delay),
	TP_ARGS(dev, sector, delay),
	TP_STRUCT__entry(
		__field(u32, major)
		__field(u32, minor)
		__field(u64, sector)
		__field(u64, delay)
	),
	TP_fast_assign(
		entry->major = MAJOR(dev);
		entry->minor = MINOR(dev);
		entry->sector = sector;
		entry->delay = delay;
	),
	TP_printk("dev=(%u,%u), sector=%llu, delay=%llu",
		__entry->major, __entry->minor, __entry->sector,
		__entry->delay)
   );

TRACE_EVENT(
	latency_tracker_net,
	TP_PROTO(struct net_device *dev, u64 delay,
		unsigned int flag, unsigned int out_id),
	TP_ARGS(dev, delay, flag, out_id),
	TP_STRUCT__entry(
		__string(name, dev->name)
		__field(u64, delay)
		__field(unsigned int, flag)
		__field(unsigned int, out_id)
	),
	TP_fast_assign(
		__assign_str(name, dev->name);
		entry->delay = delay;
		entry->flag = flag;
		entry->out_id = out_id;
	),
	TP_printk("iface=%s, delay=%llu, flag=%u, out_id=%u",
		__get_str(name), __entry->delay, __entry->flag,
		__entry->out_id)
   );

TRACE_EVENT(
	latency_tracker_critical_timing_stack,
	TP_PROTO(char comm[TASK_COMM_LEN], pid_t pid, char stack[256]),
	TP_ARGS(comm, pid, stack),
	TP_STRUCT__entry(
		 __array(char, comm, TASK_COMM_LEN)
		__field(int, pid)
		__array(char, stack, 256)
		),
	TP_fast_assign(
		memcpy(__entry->comm, comm, TASK_COMM_LEN);
		entry->pid = pid;
		memcpy(__entry->stack, stack, 256);
		),
	TP_printk("comm=%s, pid=%d, stack=%s",
		__entry->comm, __entry->pid, __entry->stack)
   );


#endif /* _TRACE_LATENCY_TRACKER_H */

/* this part must be outside protection */
#include <trace/define_trace.h>
