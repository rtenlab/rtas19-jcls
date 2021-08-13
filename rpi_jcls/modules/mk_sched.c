#include "mk.h"

struct task_struct* worker;
raw_spinlock_t mon_lock;

void update_mu_pattern(struct task_struct *p) {
	unsigned long flags;
	raw_spin_lock_irqsave(&mon_lock, flags);

	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "UPDATE_MU_PATTERN : update mu-pattern of PID %d.\n", p->pid);
	#endif
	
	ktime_t start, finish;
	start = ktime_get();
	
	int i;
	for (i = 0; i < SIZE_PATTERN-1; i++) {
		p->mu_pattern[i] = p->mu_pattern[i+1];
		#if(MK_DEBUG_MSG)
		printk(KERN_INFO "UPDATE_MU_PATTERN : mu-pattern[%d] is %d.\n", i, p->mu_pattern[i]);
		#endif
	}

	p->mu_pattern[SIZE_PATTERN-1] = p->stats.flag_job_complete;
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "UPDATE_MU_PATTERN : mu-pattern[%d] is %d.\n", SIZE_PATTERN-1, p->mu_pattern[SIZE_PATTERN-1]);
	#endif

	finish = ktime_get();
	timediff_for_ovhd(finish, start, p, 1);

	raw_spin_unlock_irqrestore(&mon_lock, flags);
}

void update_job_class_idx(struct task_struct *p) {
	unsigned long flags;
	raw_spin_lock_irqsave(&mon_lock, flags);

	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "UPDATE_JOB_CLASS_IDX : Update job-class index of PID %d.\n", p->pid);	
	#endif

	ktime_t start, finish;
	start = ktime_get();

	int i, prev_re, p_cnt, n_cnt;
	p_cnt = 0;
	n_cnt = 0;

	// Update job-class index using mu-pattern
	prev_re = p->mu_pattern[SIZE_PATTERN - 1];
	if (prev_re == 1) p_cnt = 1; else n_cnt = 1;
	for (i = 2; i <= SIZE_PATTERN; i++) {
		if (p_cnt) {
			if (p->mu_pattern[SIZE_PATTERN - i] == 1)
				p_cnt++;
			else
				break;
		} else {
			if (p->mu_pattern[SIZE_PATTERN - i] == 0) {
				n_cnt++;
			} else {
				break;
			}
		}
	}

	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "UPDATE_JOB_CLASS_IDX : Priority of the task is %d.\n", p->rt_priority);
	#endif
	if (p_cnt) {
		if (p_cnt >= p->stats.num_job_cls)
			p->stats.job_class_idx = p->stats.num_job_cls - 1;
		else
			p->stats.job_class_idx = p_cnt;
	} else {
		if (n_cnt >= p->stats.w)
			p->stats.job_class_idx = 0;
		// Else => Keep current job-class index
	}

	push_to_kwork_queue(0, 0, (void *)p, (void *)0, (void *)p->rt_pri[p->stats.job_class_idx]);
	wake_up_process(per_cpu(worker,0));

	finish = ktime_get();
	timediff_for_ovhd(finish, start, p, 2);

	raw_spin_unlock_irqrestore(&mon_lock, flags);
}

