#include "mk.h"

raw_spinlock_t sig_lock;

// Generate SIGUSR2 to jump the starting point of a task
void generate_signal_for_restart(struct task_struct *p) {
	unsigned long flags;
	raw_spin_lock_irqsave(&sig_lock, flags);

	// Generate SIGUSR2 for requesting restart of a task when DROP_MODE 1
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "SIGUSR2 of Job ID %d.\n", p->pid);
	#endif
	
	ktime_t start, finish;
	start = ktime_get();
				
	struct siginfo info;
	// Send a SIG to the thread
	memset(&info, 0, sizeof(info));
	info.si_signo = SIGUSR2;
	//info.si_int = 1234;
	info.si_code = p->rt_pri[p->stats.job_class_idx];
	info.si_pid = p->pid;
	
//	rcu_read_lock();
	p->signal->curr_target = p;
	if (send_sig_info(SIGUSR2, &info, p) < 0)
		printk(KERN_INFO "SIG Kernel to user : Fail to send a SIGUSR2 for requesting restart task.\n");
//	rcu_read_unlock();

	finish = ktime_get();
	timediff_for_ovhd(finish, start, p, 4);		

	raw_spin_unlock_irqrestore(&sig_lock, flags);	
}

// Generate SIGUSR1 to set cancel flag in user-space
void generate_signal_cancel(struct task_struct *p) {
	unsigned long flags;
	raw_spin_lock_irqsave(&sig_lock, flags);

	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "SIGUSR1(cancel) of Job ID %d.\n", p->pid);
	#endif

	struct siginfo info;
	// Send a SIG to the thread
	memset(&info, 0, sizeof(info));
	info.si_signo = SIGUSR1;
	info.si_int = 1;
	info.si_pid = p->pid;

//	rcu_read_lock();
	p->signal->curr_target = p;
	if (send_sig_info(SIGUSR1, &info, p) < 0)
		printk(KERN_INFO "SIG Kernel to user : Fail to send a SIGUSR1 for cancel the task.\n");
//	rcu_read_unlock();	

	raw_spin_unlock_irqrestore(&sig_lock, flags);
}