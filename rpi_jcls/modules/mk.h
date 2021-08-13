#ifndef _mk_h_
#define _mk_h_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <asm/cacheflush.h>
#include <uapi/asm/unistd.h>
#include <linux/vmalloc.h>

/* Reserve kthread task */
#include <linux/cpumask.h>
#include <linux/kthread.h>
#include <asm/percpu.h>
#include <linux/slab.h>

#define CRITICAL_INSTANT 5000000000 // 5 seconds
#define COPY_PERIOD 10				// mu-pattern size
#define SIZE_PATTERN 10
#define KWORK_MAX 256				// MAX of kthread work
#define MK_DEBUG_MSG 0				// Display all message for debugging (1: on, 0: off)

#define MONITOR_MODE 2					// 1: monitor scheduling, 2: monitor overhead

/* Data structure for kthread task */
struct kwork_info {
	int type;
	void *args[3];
};
struct kwork_queue {
	raw_spinlock_t lock;
	int cur_pos;
	int cur_size;
	struct kwork_info work[KWORK_MAX];
};




/* Declare kthread task functions */
void ktask_reserve_init(void);
void ktask_reserve_cleanup(void);
void ktask_update_task_priority(int type, struct task_struct *task, int cpunum, int prio);
int push_to_kwork_queue(int dest_cpu, int type, void* arg1, void* arg2, void* arg3);
//static int pop_from_kwork_queue(int dest_cpu, struct work_info *output);

/* Stamp traces */ 
void stamp_fake_trace(struct task_struct *p, bool flag);
void stamp_complete_time(struct task_struct *p);
void stamp_release_time(struct task_struct *p);
void timediff_for_ovhd(ktime_t lhs, ktime_t rhs, struct task_struct *p, int flag);

/* Copy data to user-space */
unsigned long copy_monitor_data(struct task_struct *p, struct task_stats *stats, struct timespec *trace, 
	struct timespec *release, struct timespec *complete, struct timespec *fake);

/* Generate signals (SIGUSR1, SIGUSR2) */
void generate_signal_cancel(struct task_struct *p);
void generate_signal_for_restart(struct task_struct *p);

/* Update job-class-level scheduling params */
void update_mu_pattern(struct task_struct *p);
void update_job_class_idx(struct task_struct *p);

/* Module hook */
asmlinkage long *set_rsv(void *arg);
asmlinkage long *cancel_rsv(void *arg);
asmlinkage long *wait_until_next_period(void *arg);
asmlinkage long *copy_data_to_user(void *arg1, void *arg2, void *arg3, void *arg4, void *arg5);



#endif