#include "signals.h"

// ISR of SIGUSR1 - set cancel_flag in user-space
void ISR_SIG_cancel_task(int sig_num, siginfo_t *info, void *unused) {
	int tid;

	tid = syscall(SYS_gettid);
	printf("Cancel signal of tid is %d\n", tid);

	// find index of restart_p
	for (int i = 0; i < num_tasks; i++) {
		if (cancel_task_id[i] == tid) {
			printf("Cancel signal from kernel PID %d\n", tid);
			cancel_flag[i] = 1;
		}
	}	
}

// ISR of SIGUSR2 - request drop a job : jump to setjmp point
void ISR_SIG_jump_task(int sig_num, siginfo_t *info, void *unused) {
	int index, re, tid;
	unsigned long flags;

	tid = info->si_pid;
	printf("Restart signal of tid is %d\n", tid);


	pthread_mutex_lock(&sig_mutex);
	// find index of restart_p
	for (int i = 0; i < num_tasks; i++) {
		if (jmp_buf_task_id[i] == tid)
			index = i;
	}
	pthread_mutex_unlock(&sig_mutex);

	//#if(DISPLAY_MSG)
	printf("Task ID is %d.\n", jmp_buf_task_id[index]);
	//#endif

	if (tid == jmp_buf_task_id[index]) {
		longjmp(restart_p[index], tid);
	} else {
		#if(DISPLAY_MSG)
		printf("tid1 and task id of jmpbuf is different.\n");
		#endif
	}

}