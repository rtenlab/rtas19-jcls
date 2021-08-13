#include "mk.h"

int JOB_MODE;						// Job mode 1: DROP, 2: CONTINUE, 3: SKIP
ktime_t c_start;					// To sync inital release time between tasks

asmlinkage long *copy_data_to_user(void *arg1, void *arg2, void *arg3, void *arg4, void *arg5) {
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "MODULE SYSCALL : copy_data_to_user intercept flex_bridge4 successfully.\n");
	#endif
	//struct task_stats *buff = (struct task_stats *)original_flex_bridge4(arg);
	struct task_stats *buff_stats = (struct task_stats *)arg1;
	struct timespec *buff_trace = (struct timespec *)arg2;
	struct timespec *buff_release = (struct timespec *)arg3;
	struct timespec *buff_complete = (struct timespec *)arg4;
	struct timespec *buff_fake = (struct timespec *)arg5;
	/*
	struct timespec *buff_ovhd1 = (struct timespec *)arg6;
	struct timespec *buff_ovhd2 = (struct timespec *)arg7;
	struct timespec *buff_ovhd3 = (struct timespec *)arg8;
	*/
	unsigned long ret;
	//struct task_stats *buff;

	struct task_struct *pid_task_ptr;
	static pid_t c_pid;
	
	c_pid = current->pid;
	
	rcu_read_lock();
	pid_task_ptr = pid_task(find_vpid(c_pid), PIDTYPE_PID);
	rcu_read_unlock();

	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "COPY_DATA_TO_USER : copy_monitor_data of PID is %d.\n", pid_task_ptr->pid);
	#endif

	ret = copy_monitor_data(pid_task_ptr, buff_stats, buff_trace, buff_release, buff_complete, buff_fake);
										 
	if (ret != 0) {
		printk(KERN_INFO "COPY_DATA_TO_USER : copy_monitor_data has not copied %d bytes.\n", ret);
		return ret;
	} else {
		#if(MK_DEBUG_MSG)
		printk(KERN_INFO "COPY_DATA_TO_USER : copy_monitor_data copied to user space.\n");
		#endif
		//write_lock(&mr_rwlock);
		pid_task_ptr->stats.trace_idx = 0;
		pid_task_ptr->stats.complete_idx = 0;
		pid_task_ptr->stats.release_idx = 0;
		pid_task_ptr->stats.fake_idx = 0;
		pid_task_ptr->stats.ovhd1_idx = 0;
		pid_task_ptr->stats.ovhd2_idx = 0;
		pid_task_ptr->stats.ovhd3_idx = 0;
		pid_task_ptr->stats.ovhd4_idx = 0;
		//write_unlock(&mr_rwlock);
	}

	return 0;

}

// Periodic HRTimer restart at every period of a task
enum hrtimer_restart periodic_timer(struct hrtimer *p_timer) {
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "PERIODIC_TIMER : Job ID %d, Start of periodic_timer.\n", p_timer->task_ptr->pid);
	#endif
	
	static struct timespec exec_time, task_T;

	// Period of a task
	task_T = p_timer->task_ptr->T;

	/* Job index var[0] is incremented at every period,
		wait_until_next_period counter is incremented whenever a job is completed. 
	    so, Compare the current job index is same as wait_until_next_period counter. */
	if (p_timer->task_ptr->stats.wait_until_cnt == p_timer->task_ptr->stats.job_idx) {
		// Update mu-pattern
		//if (p_timer->task_ptr->stats.complete_stamp_flag == 0)
		//	update_mu_pattern(p_timer->task_ptr);
		p_timer->task_ptr->stats.complete_stamp_flag = 0;
		
		// Update job-class index (priority)
		//update_job_class_idx(p_timer->task_ptr);

		stamp_release_time(p_timer->task_ptr);
		// Wake up a task
		wake_up_process(p_timer->task_ptr);

	} else {
		// There exist delayed jobs, following protocol based on a mode
		// DROP if it doesn't finish its execution within its deadline
		if (JOB_MODE == 1) {	
			// Job is still executing
			if (p_timer->task_ptr->stats.flag_job_complete == 0) { 
				#if(MK_DEBUG_MSG)
				printk(KERN_INFO "PERIODIC_TIMER : JOB_MODE 1 - Job ID %d, missed.\n", p_timer->task_ptr->pid);
				#endif

				// Update mu-pattern
				if (p_timer->task_ptr->stats.complete_stamp_flag == 0)
					update_mu_pattern(p_timer->task_ptr);
				p_timer->task_ptr->stats.complete_stamp_flag = 0;

				// Update job-class index (priority)
				update_job_class_idx(p_timer->task_ptr);
				
				// Increment miss count
				p_timer->task_ptr->stats.miss++;
				
				// Stamp face trace in and out
				stamp_fake_trace(p_timer->task_ptr, 0);
				
				// Generate restart signal
				generate_signal_for_restart(p_timer->task_ptr);
				
				stamp_release_time(p_timer->task_ptr);
				stamp_fake_trace(p_timer->task_ptr, 1);

			} else {
				#if(MK_DEBUG_MSG)
				printk(KERN_INFO "PERIODIC_TIMER : JOB_MODE 1 - Job ID %d, is met.\n", p_timer->task_ptr->pid);
				#endif

				// Update mu-pattern
				//if (p_timer->task_ptr->stats.complete_stamp_flag == 0)
				//	update_mu_pattern(p_timer->task_ptr);
				p_timer->task_ptr->stats.complete_stamp_flag = 0;
				
				// Update job-class index (priority)
				//update_job_class_idx(p_timer->task_ptr);

				stamp_release_time(p_timer->task_ptr);

				// Wake up a next job
				wake_up_process(p_timer->task_ptr);				
			}
		} else if (JOB_MODE == 2) {

		}
	}

	if (p_timer->task_ptr->stats.job_idx > p_timer->task_ptr->stats.num_repeats) {
		generate_signal_cancel(p_timer->task_ptr);
		#if(MK_DEBUG_MSG)
		printk(KERN_INFO "PERIODIC_TIMER : Job ID %d, Stop hrtimer for next period.\n", p_timer->task_ptr->pid);
		#endif
		return HRTIMER_NORESTART;
	} else {
		#if(MK_DEBUG_MSG)
		printk(KERN_INFO "PERIODIC_TIMER : Job ID %d, Forward hrtimer for next period.\n", p_timer->task_ptr->pid);
		#endif
		hrtimer_forward_now(p_timer, timespec_to_ktime(task_T));
		return HRTIMER_RESTART;
	}
}

asmlinkage long *wait_until_next_period(void *arg) {
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "MODULE SYSCALL : wait_until_next_period intercept flex_bridge3 successfully.\n");
	#endif
	struct set_task_param *set = (struct set_task_param *)arg;

	struct task_struct *pid_task_ptr;
	static pid_t c_pid;
	struct siginfo info;
	struct sched_param param;
	int re_sched;

	c_pid = current->pid;
	
	rcu_read_lock();
	pid_task_ptr = pid_task(find_vpid(c_pid), PIDTYPE_PID);
	rcu_read_unlock();

    // Update wait_until_next_period counter, only update request from after completing execution
    if (set->wait_until_mode) {
    	// Increment wait_until_counter
    	pid_task_ptr->stats.wait_until_cnt++;

		// In this loop, it means that a job complete its execution even though there exists 
		// a difference between job index and wait_until_counter 
		if (JOB_MODE == 1) {
			#if(MK_DEBUG_MSG)
			printk(KERN_INFO "WAIT_UNTIL_NEXT_PERIOD : Job ID %d, Job index is %d.\n", pid_task_ptr->pid, pid_task_ptr->stats.job_idx);
			#endif

			if (pid_task_ptr->stats.job_idx >= pid_task_ptr->stats.num_repeats) {
				#if(MK_DEBUG_MSG)
				printk(KERN_INFO "WAIT_UNTIL_NEXT_PERIOD : DROP_MODE 1 - Job ID %d, set cancel_flag.\n", pid_task_ptr->pid);
				#endif

				stamp_complete_time(pid_task_ptr);

				// Update mu-pattern
				update_mu_pattern(pid_task_ptr);
				pid_task_ptr->stats.complete_stamp_flag = 1;

				generate_signal_cancel(pid_task_ptr);
				
			} else {
				#if(MK_DEBUG_MSG)
				printk(KERN_INFO "WAIT_UNTIL_NEXT_PERIOD : DROP_MODE 1 - Job ID %d, complete execution.\n", pid_task_ptr->pid);
				#endif

				stamp_complete_time(pid_task_ptr);
				
				// Update mu-pattern
				update_mu_pattern(pid_task_ptr);
				pid_task_ptr->stats.complete_stamp_flag = 1;
				
				// Update job-class index (priority)
				update_job_class_idx(pid_task_ptr);
				
				set_current_state(TASK_INTERRUPTIBLE);
		    	schedule();
			}

	    }
    } else {
		#if(MK_DEBUG_MSG)
    	printk(KERN_INFO "WAIT_UNTIL_NEXT_PERIOD : Initial wait status of a thread PID %d.\n", pid_task_ptr->pid);
    	#endif

    	// Initial request of wait_until_next_period
	    set_current_state(TASK_INTERRUPTIBLE);
    	schedule();
    }
    return 0;
}

asmlinkage long *cancel_rsv(void *arg) {
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "MODULE SYSCALL : cancel_rsv intercept flex_bridge2 successfully.\n");
	#endif

	pid_t pid = current->pid;
	struct task_struct *pid_task_ptr, *g, *t;
	unsigned int pid_rt_prio;
	struct	sched_param param;

	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "CANCEL_RSV : Current PID is %d.\n", (int)pid);
	#endif

	rcu_read_lock();
	pid_task_ptr = pid_task(find_vpid(pid), PIDTYPE_PID);
	rcu_read_unlock();

	if (pid_task_ptr == NULL) {
		printk(KERN_INFO "CANCEL_RSV : PID is NULL\n");
		return (long *)1;
	} else {
		if (pid_task_ptr->C.tv_sec == 0 && pid_task_ptr->C.tv_nsec == 0) return (long *)1;
		if (pid_task_ptr->T.tv_sec == 0 && pid_task_ptr->T.tv_nsec == 0) return (long *)1;
		
		pid_task_ptr->C.tv_sec = 0;
		pid_task_ptr->C.tv_nsec = 0;
		pid_task_ptr->T.tv_sec = 0;
		pid_task_ptr->T.tv_nsec = 0;
		pid_task_ptr->reserved = 0;

		hrtimer_cancel(&pid_task_ptr->p_timer);
		printk(KERN_INFO "CANCEL_RSV : The reservation for PID %d is canceled\n", (int)pid);
	}

	// Free memory allocations
	vfree(pid_task_ptr->stamp_trace);
	vfree(pid_task_ptr->stamp_release);
	vfree(pid_task_ptr->stamp_complete);
	vfree(pid_task_ptr->stamp_fake);
	vfree(pid_task_ptr->rt_pri);
	vfree(pid_task_ptr->mu_pattern);
	vfree(pid_task_ptr->ovhd1);
	vfree(pid_task_ptr->ovhd2);
	vfree(pid_task_ptr->ovhd3);
	vfree(pid_task_ptr->ovhd4);
	vfree(pid_task_ptr->ovhd5);
	c_start = ns_to_ktime(0);
	//JOB_MODE = 0;

	return (unsigned long)pid;
}


asmlinkage long *set_rsv(void *arg) {
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "MODULE SYSCALL : set_rsv intercept flex_bridge1 successfully.\n");
	#endif

//	struct task_stats *set_var = (struct task_stats *)original_flex_bridge1(arg);
	struct set_task_param *set_var = (struct set_task_param *)arg;

	pid_t pid = 0;
	struct task_struct *pid_task_ptr, *g, *t;
	pid_t tmp_pid, proc_pid;
	struct sched_param param;
	struct timespec now_t_spec;
	unsigned int min_prio = 99;
	unsigned int pid_rt_prio = 0;
	unsigned int first_t = 1;
	int i;

	// Check the arguments whether they are valid or not
	if (pid < 0) {
		printk(KERN_INFO "SET_RSV : PID is less than zero.\n");
		return (long *)(-1);
	}
	if ((set_var->C.tv_sec == 0 && set_var->C.tv_nsec == 0) || set_var->C.tv_sec < 0 || set_var->C.tv_nsec < 0) {
		printk(KERN_INFO "SET_RSV : Reservation time C is zero.\n");
		return (long *)(-1);
	}
	if ((set_var->T.tv_sec == 0 && set_var->T.tv_nsec == 0) || set_var->T.tv_sec < 0 || set_var->T.tv_nsec < 0) {
		printk(KERN_INFO "SET_RSV : Reservation time T is zero.\n");
		return (long *)(-1);
	}

	if (pid == 0) 
		tmp_pid = current->pid;
	else
		tmp_pid = pid;

	rcu_read_lock();
	pid_task_ptr = pid_task(find_vpid(tmp_pid), PIDTYPE_PID);
	rcu_read_unlock();
	proc_pid = pid_task_ptr->tgid;
	printk("SET_RSV : A process ID (TGID) is : %d\n", pid_task_ptr->tgid);

	if (pid_task_ptr == NULL) {
		printk(KERN_INFO "SET_RSV : PID is NULL\n");
		return (long *)(-1);
	}

	// Initialize stats parameters of the task
	pid_task_ptr->stats.trace_idx = 0;
	pid_task_ptr->stats.release_idx = 0;
	pid_task_ptr->stats.complete_idx = 0;
	pid_task_ptr->stats.fake_idx = 0;
	pid_task_ptr->stats.ovhd1_idx = 0;
	pid_task_ptr->stats.ovhd2_idx = 0;
	pid_task_ptr->stats.ovhd3_idx = 0;
	pid_task_ptr->stats.ovhd4_idx = 0;
	for (i = 0; i < 10; i++) {
		pid_task_ptr->mu_pattern[i] = 0;
		pid_task_ptr->rt_pri[i] = set_var->rt_pri[i];
	}
	// Assign C, T
	pid_task_ptr->C = set_var->C;
	pid_task_ptr->T = set_var->T;
	pid_task_ptr->stats.init_release = set_var->C;	// This is a dummy for copy, not used in scheduling
	pid_task_ptr->stats.cancel = set_var->T;		// This is a dummy for copy, not used in scheduling
	//pid_task_ptr->stats.ovhd1.tv_sec = 0;
	//pid_task_ptr->stats.ovhd1.tv_nsec = 0;
	printk(KERN_INFO "SET_RSV : PID %d is reserved as C : %ld.%09ld sec\t T : %ld.%09ld sec\n", (int)tmp_pid, pid_task_ptr->C.tv_sec, pid_task_ptr->C.tv_nsec, pid_task_ptr->T.tv_sec, pid_task_ptr->T.tv_nsec);

	// Assign weakly-hard constraints
	pid_task_ptr->stats.m = set_var->m;
	pid_task_ptr->stats.K = set_var->K;
	pid_task_ptr->stats.w = set_var->w;
	pid_task_ptr->stats.num_job_cls = set_var->num_job_cls;
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "SET_RSV : Number of job-classes is %d.\n", pid_task_ptr->stats.num_job_cls);
	#endif

	// Assign initial RT priority - the first job-class index
	pid_task_ptr->stats.job_class_idx = 0;
	param.sched_priority = pid_task_ptr->rt_pri[pid_task_ptr->stats.job_class_idx];
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "SET_RSV : rt_priority of the task %d is %d\n", (int)tmp_pid, param.sched_priority);
	#endif

	int re_sched;
	re_sched = sched_setscheduler(pid_task_ptr, SCHED_FIFO, &param);
	
	if (re_sched == -1) {
		printk(KERN_INFO "SET_RSV : Fail to set a RT scheduler.\n");
		return (long *)(-1);
	}

	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "SET_RSV : Set a task as a real-time task successfully.\n");
	#endif
	pid_task_ptr->reserved = 1;
	pid_task_ptr->stats.num_repeats = set_var->num_repeats;

	// Set drop mode
	pid_task_ptr->stats.job_mode = set_var->job_mode;
	JOB_MODE = set_var->job_mode;
	#if(MK_DEBUG_MSG)
    printk(KERN_INFO "SET_RSV : Set JOB_MODE is %d\n", JOB_MODE);
    #endif

	// Initializing hrtimer
	hrtimer_init(&pid_task_ptr->p_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_PINNED);
	pid_task_ptr->p_timer.task_ptr = pid_task_ptr;
	pid_task_ptr->p_timer.function = &periodic_timer;
	
	// Set timer to start all tasks start at critical instant
    if (ktime_equal(c_start, ns_to_ktime(0))) {
    	c_start = ktime_add_safe(ktime_get(), ns_to_ktime(CRITICAL_INSTANT));
    }
	//printk(KERN_INFO "SET_RSV : PID of HRTIMER is %d.\n", pid_task_ptr->p_timer.task_ptr->pid);
    hrtimer_start(&pid_task_ptr->p_timer, c_start, HRTIMER_MODE_ABS_PINNED);
    pid_task_ptr->time_stamp = ktime_to_timespec(ktime_get());    

    #if(MK_DEBUG_MSG)
    struct timespec c_start_tspec = ktime_to_timespec(c_start);
    printk(KERN_INFO "SET_RSV : Current time of PID %d is : %ld.%09ld sec\n", (int)tmp_pid, pid_task_ptr->time_stamp.tv_sec, pid_task_ptr->time_stamp.tv_nsec);
	printk(KERN_INFO "SET_RSV : Set initial time of PID %d is : %ld.%09ld sec\n", (int)tmp_pid, c_start_tspec.tv_sec, c_start_tspec.tv_nsec);
	#endif

	return (long *)tmp_pid;
}

