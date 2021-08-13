#include "mk.h"

DEFINE_RWLOCK(mr_rwlock);			// Write lock

// Stamp face trace in & out for DROP_MODE 1
void stamp_fake_trace(struct task_struct *p, bool flag) {
	struct timespec now = ktime_to_timespec(ktime_get());
	write_lock(&mr_rwlock);
	if (flag == 1) {
		p->stamp_fake[p->stats.fake_idx] = now;	
	} else if (flag == 0) {
		p->stamp_fake[p->stats.fake_idx].tv_sec = -now.tv_sec;
		p->stamp_fake[p->stats.fake_idx].tv_nsec = now.tv_nsec;	
	}
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "STAMP_FAKE_TIME : Job ID %d, %ld.%09ld sec\n", p->pid, p->stamp_fake[p->stats.fake_idx].tv_sec, p->stamp_fake[p->stats.fake_idx].tv_nsec);
	#endif

	p->stats.fake_idx++;
	write_unlock(&mr_rwlock);
}

// Stamp complete time in wait_until_next_period
void stamp_complete_time(struct task_struct *p) {
	struct timespec now = ktime_to_timespec(ktime_get());
	write_lock(&mr_rwlock);
	p->stamp_complete[p->stats.complete_idx] = now;
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "STAMP_COMPLETE_TIME : Job ID %d, %ld.%09ld sec\n", p->pid, p->stamp_complete[p->stats.complete_idx].tv_sec, p->stamp_complete[p->stats.complete_idx].tv_nsec);
	#endif

	p->stats.complete_idx++;
	p->stats.flag_job_complete = 1;
	p->stats.meet++;
	write_unlock(&mr_rwlock);
}


// Stamp release time at every period
void stamp_release_time(struct task_struct *p) {
	struct timespec now = ktime_to_timespec(ktime_get());
	write_lock(&mr_rwlock);
	p->stamp_release[p->stats.release_idx] = now;
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "STAMP_RELEASE_TIME : Job ID %d, %ld.%09ld sec\n", p->pid, p->stamp_release[p->stats.release_idx].tv_sec, p->stamp_release[p->stats.release_idx].tv_nsec);
	#endif
	
	p->stats.release_idx++;
	p->stats.job_idx++;
	p->stats.flag_job_complete = 0;
	// Save current real-time priority
	write_unlock(&mr_rwlock);
}

// Copy monitoring data to user space
unsigned long copy_monitor_data(struct task_struct *p, struct task_stats *buff_stats, struct timespec *buff_trace, struct timespec *buff_release, 
								struct timespec *buff_complete, struct timespec *buff_fake) {
	unsigned long len1, ret1;
	unsigned long len2, ret2;
	unsigned long len3, ret3;
	unsigned long len4, ret4;
	unsigned long len5, ret5;
	len1 = sizeof(struct task_stats)*1;
	ret1 = copy_to_user(buff_stats, &p->stats, len1);
	if (ret1 != 0) {
		printk(KERN_INFO "COPY_MONITOR_DATA : stats has not copied %d bytes.\n", ret1);
	}
	
	switch (MONITOR_MODE) {
		case 1 :
			// Copy trace
			len2 = sizeof(struct timespec)*TASK_TRACE_BUFF;
			ret2 = copy_to_user(buff_trace, p->stamp_trace, len2);
			if (ret2 != 0) {
				printk(KERN_INFO "COPY_MONITOR_DATA : Trace has not copied %d bytes.\n", ret2);
			}
			// Copy release
			len3 = sizeof(struct timespec)*TASK_REL_COM_BUFF;
			ret3 = copy_to_user(buff_release, p->stamp_release, len3);
			if (ret3 != 0) {
				printk(KERN_INFO "COPY_MONITOR_DATA : Release has not copied %d bytes.\n", ret3);
			}
			// Copy fake
			len5 = sizeof(struct timespec)*TASK_TRACE_BUFF;
			ret5 = copy_to_user(buff_fake, p->stamp_fake, len5);
			if (ret5 != 0) {
				printk(KERN_INFO "COPY_MONITOR_DATA : Fake has not copied %d bytes.\n", ret5);
			}
			// Copy complete
			len4 = sizeof(struct timespec)*TASK_REL_COM_BUFF;
			ret4 = copy_to_user(buff_complete, p->stamp_complete, len4);
			if (ret4 != 0) {
				printk(KERN_INFO "COPY_MONITOR_DATA : Complete has not copied %d bytes.\n", ret4);
			}		
		break;

	case 2 :
			// Copy ovhd1
			len2 = sizeof(struct timespec)*TASK_TRACE_BUFF;
			ret2 = copy_to_user(buff_trace, p->ovhd1, len2);
			if (ret2 != 0) {
				printk(KERN_INFO "COPY_MONITOR_DATA : Ovhd1 has not copied %d bytes.\n", ret2);
			}
			// Copy ovhd2
			len3 = sizeof(struct timespec)*TASK_TRACE_BUFF;
			ret3 = copy_to_user(buff_release, p->ovhd2, len3);
			if (ret3 != 0) {
				printk(KERN_INFO "COPY_MONITOR_DATA : Ovhd2 has not copied %d bytes.\n", ret3);
			}

			// Copy ovhd3
			len5 = sizeof(struct timespec)*TASK_TRACE_BUFF;
			ret5 = copy_to_user(buff_fake, p->ovhd3, len5);
			if (ret5 != 0) {
				printk(KERN_INFO "COPY_MONITOR_DATA : Ovhd3 has not copied %d bytes.\n", ret5);
			}

			len4 = sizeof(struct timespec)*TASK_TRACE_BUFF;
			ret4 = copy_to_user(buff_complete, p->ovhd4, len4);
			if (ret4 != 0) {
				printk(KERN_INFO "COPY_MONITOR_DATA : Ovhd4 has not copied %d bytes.\n", ret4);
			}
		break;

	default:
		break;
	}

	

	return ret1+ret2+ret3+ret4+ret5;

/*
	// Copy ovhd1
	unsigned long len6, ret6;
	len6 = sizeof(struct timespec)*TASK_TRACE_BUFF;
	ret6 = copy_to_user(buff_ovhd1, p->ovhd1, len6);
	if (ret6 != 0) {
		printk(KERN_INFO "COPY_MONITOR_DATA : Ovhd1 has not copied %d bytes.\n", ret6);
	}

	// Copy ovhd2
	unsigned long len7, ret7;
	len7 = sizeof(struct timespec)*TASK_TRACE_BUFF;
	ret7 = copy_to_user(buff_ovhd2, p->ovhd2, len7);
	if (ret7 != 0) {
		printk(KERN_INFO "COPY_MONITOR_DATA : Ovhd2 has not copied %d bytes.\n", ret7);
	}

	// Copy ovhd3
	unsigned long len8, ret8;
	len8 = sizeof(struct timespec)*TASK_TRACE_BUFF;
	ret8 = copy_to_user(buff_ovhd3, p->ovhd3, len8);
	if (ret8 != 0) {
		printk(KERN_INFO "COPY_MONITOR_DATA : Ovhd3 has not copied %d bytes.\n", ret8);
	}
*/

}

// Time difference of two ktime and save it to TCB for overhead of JCLS (ovhd1)
void timediff_for_ovhd(ktime_t lhs, ktime_t rhs, struct task_struct *p, int ovhd_flag) {
	ktime_t diff;
	struct timespec t_spec_diff;

	diff = ktime_sub(lhs, rhs);
	t_spec_diff = ktime_to_timespec(diff);

	switch(ovhd_flag) {
		case 1:
			p->ovhd1[p->stats.ovhd1_idx] = t_spec_diff;
			p->stats.ovhd1_idx++;
			break;
		case 2:
			p->ovhd2[p->stats.ovhd2_idx] = t_spec_diff;
			p->stats.ovhd2_idx++;
			break;
		case 3:
			p->ovhd3[p->stats.ovhd3_idx] = t_spec_diff;
			p->stats.ovhd3_idx++;
			break;
		case 4:
			p->ovhd4[p->stats.ovhd4_idx] = t_spec_diff;
			p->stats.ovhd4_idx++;
			break;
		default:
			break;
	}
	//p->stats.ovhd1.tv_sec += t_spec_diff.tv_sec;
	//p->stats.ovhd1.tv_nsec += t_spec_diff.tv_nsec;
}