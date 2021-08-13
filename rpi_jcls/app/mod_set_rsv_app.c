#include <string.h>
#include <linux/types.h>
//#include <pthread.h>
//#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

// Include local headers
#include "init_task.h"
#include "signals.h"

// System call number * Do not change!
#define RSV_DEV 402
#define CANCEL_RSV_DEV 398
#define WAIT_UNTIL_NEXT_PERIOD 399
#define COPY_DATA_TO_USER 400


// For dummy task
#define MATRIX_SIZE 50
#define DISPLAY_MSG 0
#define DATA_RQT_PERIOD 40
#define MONITOR_MODE 2

static pthread_mutex_t mutex;

// Declaration of functions
void save_data(struct task_stats*, struct timespec*, struct timespec*, struct timespec*, struct timespec*, struct timespec*, int, int);
//void save_data(struct task_stats*, struct timespec*, struct timespec*, struct timespec*, struct timespec*, int);
void matrix_multiplicatiion(int);
int request_data_to_kernel(int, struct timespec*, int);
//void request_data_to_kernel(int);
int cancel_rsv_task();
void *set_rsv(void *input);

// Main function
int main(int argc, char *argv[]) {

	struct set_task_param *set_var;
	pthread_t *thread;
	long double tmp_p, max_p = 0.0;
	unsigned int repeat, num_job_class;
	job_mode = 1;

	if (argc < 5 || (argc%2 == 0)) {
		printf("Usage: ./mod_set_rsv_app C1 T1 m1 K1 C2 T2 m2 K2 ...\n");
		printf("C: Reserved computation in milliseconds\n");
		printf("T: Reserved Peroid in milliseconds\n");
		printf("m: Weakly-hard constraint (# of misses)\n");
		printf("K: Weakly-hard constraint\n");
		return -1;
	}

	num_tasks = (argc-1)/4;
	cancel_flag = malloc(sizeof(int)*num_tasks);
	cancel_task_id = malloc(sizeof(int)*num_tasks);
	set_var = malloc(sizeof(struct set_task_param)*num_tasks);
	for (int i = 0; i < num_tasks; i++) {
		//printf("m=%d, K=%d.\n", atoi(argv[(i+1)*4-1]), atoi(argv[(i+1)*4]));
		num_job_class = atoi(argv[(i+1)*4]) - atoi(argv[(i+1)*4-1]) + 1;
		printf("The number of job classes is %d\n", num_job_class);
		set_var[i].rt_pri = malloc(sizeof(unsigned int)*10);
	}
	thread = malloc(sizeof(pthread_t)*num_tasks);
	
	if (job_mode == 1) {
		restart_p = malloc(sizeof(jmp_buf)*num_tasks);
		jmp_buf_task_id = malloc(sizeof(int)*num_tasks);
	}
	
	// Initialize parameters of tasks
	init_tasksets(set_var, argv);


	// Set up SIGUSR1 for data copy
	struct sigaction sig1;
	memset(&sig1, 0, sizeof(struct sigaction));
	sigemptyset(&sig1.sa_mask);
	sigaddset(&sig1.sa_mask, SIGUSR1);
	sig1.sa_sigaction = ISR_SIG_cancel_task;
	sig1.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sig1, NULL);

	// Set up SIGUSR2 for dropping a job
	if (job_mode == 1) {
		struct sigaction sig;
		memset(&sig, 0, sizeof(struct sigaction));
		sigemptyset(&sig.sa_mask);
		sigaddset(&sig.sa_mask, SIGUSR2);
		sig.sa_sigaction = ISR_SIG_jump_task;
		sig.sa_flags = SA_SIGINFO;
		sigaction(SIGUSR2, &sig, NULL);
	}

	// Create threads
	for (int i = 0; i < num_tasks; i++) {
		if (pthread_create(&thread[i], NULL, set_rsv, (void *)&set_var[i])){
			printf("Error creating a thread.\n");
			return 1;
		} else {
			printf("A thread is created.\n");
		}
	}

	sleep(5);
	
	// Terminate threads
	for (int j = 0; j < num_tasks; j++) {
		if (pthread_join(thread[j], NULL)) {
			printf("Error joining a thread.\n");
			//return 2;
		} else {
			printf("A thread is joined.\n");
		}	
	}

	// Free allocated memory
	for (int i = 0; i < num_tasks; i++) {
		free(set_var[i].rt_pri);
	}
	free(set_var);
	free(thread);
	
	if (job_mode == 1) {
		free(restart_p);
		free(jmp_buf_task_id);
	}

	return 0;
}

// Main thread function
void *set_rsv(void *input) {
	// variables
	pthread_t thread = pthread_self();
	struct set_task_param *set_var = (struct set_task_param *)input;
	struct timespec start, end;
	struct timespec *setjmp_time;
	int re, ret, pid_rsv, i, restart_p_idx, cancel_idx, timediff_idx = 0;
	int counter = 0;

	setjmp_time = malloc(sizeof(struct timespec)*TASK_TRACE_BUFF);


	// Set CPU to run
	cpu_set_t set_cpu;
	CPU_ZERO(&set_cpu);
	CPU_SET(2, &set_cpu);
	int set_result = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &set_cpu);
	if (set_result != 0 ) {
		printf("Set pthread error.\n");
	}

	printf("C is %ld.%9ld sec\n", set_var->C.tv_sec, set_var->C.tv_nsec);
	printf("T is %ld.%9ld sec\n", set_var->T.tv_sec, set_var->T.tv_nsec);

	// System call for reservation	
	pid_rsv = syscall(RSV_DEV, (void *)set_var);
	printf("RSV_DEV result is %d\n", pid_rsv);
	if (pid_rsv < 0) {
		#if(DISPLAY_MSG)
		printf("Syscall RSV_DEV has an error.\n");
		printf("Error description is %s\n", strerror(errno));
		#endif
		return NULL;
	} 
	#if(DISPLAY_MSG)
	printf("Syscall RSV_DEV has done successfully.\n");
	#endif

	// For drop mode, set jump buffers
	if (job_mode == 1) {
		pthread_mutex_lock(&mutex);
		restart_p_idx = pid_rsv - getpid() - 1;
		jmp_buf_task_id[restart_p_idx] = pid_rsv;
		printf("PID is %d\n", jmp_buf_task_id[restart_p_idx]);	
		pthread_mutex_unlock(&mutex);
	}

	pthread_mutex_lock(&mutex);
	cancel_idx = pid_rsv - getpid() - 1;
	cancel_task_id[cancel_idx] = pid_rsv;
	cancel_flag[cancel_idx] = 0;
	pthread_mutex_unlock(&mutex);

//	printf("Process PID is %d\n", getpid());
	printf("Thread PID is is %d\n", pid_rsv);

	// Set wait_until_next_period mode (0 : initial wait)
	set_var->wait_until_mode = 0;
	re = syscall(WAIT_UNTIL_NEXT_PERIOD, (void *)set_var);
	if (re != 0) {
		#if(DISPLAY_MSG)
		printf("Syscall WAIT_UNTIL_NEXT_PERIOD(Initial) has an error.\n");
		#endif
	}

	printf("Job ID %d, Start a thread's work.\n", pid_rsv);
	// Wrap real task
	while(1) {

		// For drop mode, save a setjmp point(start of a task)
		if (job_mode == 1) {
			//gettimeofday(&start, NULL);
			clock_gettime(CLOCK_REALTIME, &start);
			re = sigsetjmp(restart_p[restart_p_idx], SIGUSR2);		
			
			if (re == 0) {
				//gettimeofday(&end, NULL);
				clock_gettime(CLOCK_REALTIME, &end);
				setjmp_time[timediff_idx].tv_sec = end.tv_sec - start.tv_sec;
				setjmp_time[timediff_idx].tv_nsec = end.tv_nsec - start.tv_nsec;
				//printf("SETJMP time overhead is %ld.%09ld\n", setjmp_time[timediff_idx].tv_sec, setjmp_time[timediff_idx].tv_nsec);
				timediff_idx++;

				#if(DISPLAY_MSG)
				printf("Job ID %d, SETJMP is set for restart.\n", pid_rsv);	
				#endif
			} else {
				// Increment job counter (when a job is dropped.)
				counter++;

				#if(DISPLAY_MSG)
				printf("Job ID %d, Restart is activated.\n", re);
				//printf("Cancel_flag is %d\n", cancel_flag[cancel_idx]);
				#endif
			}	
		}

		// Request data transfer from Kernel
		if ((counter != 0 && (counter % DATA_RQT_PERIOD) == 0) || cancel_flag[cancel_idx] != 0) {
			timediff_idx = request_data_to_kernel(pid_rsv, setjmp_time, timediff_idx);
			//request_data_to_kernel(pid_rsv);
		}

		// If cancel flag is set, go to cancel procedure
		if (cancel_flag[cancel_idx] != 0) {
			// Request data transfer from Kernel
			//request_data_to_kernel();
			if (cancel_rsv_task() < 0) {
				#if(DISPLAY_MSG)
				printf("Cancel_rsv_task returns an error.\n");
				#endif			
			} else {
				#if(DISPLAY_MSG)
				printf("Terminate a process.\n");
				#endif
				free(setjmp_time);
				return NULL;
			}
		}

		// Start of a task function
		matrix_multiplicatiion(pid_rsv);
		// End of a task function
		
		// Set wait_until_next_period mode (1 : wait after finishing execution)
		set_var->wait_until_mode = 1;
		re = syscall(WAIT_UNTIL_NEXT_PERIOD, (void *)set_var);
		if (re < 0) {
			#if(DISPLAY_MSG)
			printf("Fail to run 'wait_until_next_period' SYSCALL.\n");
			#endif
		} else {
			// Increment job counter (when a job is completed.)
			counter++;
			#if(DISPLAY_MSG)
			printf("Job ID %d, Thread is sleeping by 'wait_until_next_period' SYSCALL.\n", pid_rsv);
			#endif
		}
	}
	free(setjmp_time);
	return NULL;
}

// Save monitoring data from kernel
void save_data(struct task_stats *stats, struct timespec *trace, struct timespec *release, 
								struct timespec *complete, struct timespec *fake, struct timespec *ovhd5, int idx, int tid) {
//void save_data(struct task_stats *stats, struct timespec *trace, struct timespec *release, 
//								struct timespec *complete, struct timespec *fake, int tid) {



	//pthread_mutex_lock(&mutex);
	FILE *fp;
	char pid[10], fname[15];

	sprintf(pid, "%d", tid);
	strcat(pid,".txt");
	
	fp = fopen(pid, "a+");

	if (fp == NULL) {
		printf("Cannot open file!\n");		
	} else {
		if (MONITOR_MODE == 1) {
			for (int i = 0; i < stats->trace_idx; i++) {
			fprintf(fp, "trace: %ld.%09ld\n", trace[i].tv_sec, trace[i].tv_nsec);
			}
			for (int i = 0; i < stats->release_idx; i++) {
				fprintf(fp, "release: %ld.%09ld\n", release[i].tv_sec, release[i].tv_nsec);
			}	
			for (int i = 0; i < stats->fake_idx; i++) {
				fprintf(fp, "fake: %ld.%09ld\n", fake[i].tv_sec, fake[i].tv_nsec);
			}
			for (int i = 0; i < stats->complete_idx; i++) {
				fprintf(fp, "complete: %ld.%09ld\n", complete[i].tv_sec, complete[i].tv_nsec);
			}
		} else {

			for (int i = 0; i < TASK_TRACE_BUFF; i++) {
				if (i < stats->ovhd1_idx)
					fprintf(fp, "ovhd1: %ld.%09ld\n", trace[i].tv_sec, trace[i].tv_nsec);
					
				if (i < stats->ovhd2_idx)
					fprintf(fp, "ovhd2: %ld.%09ld\n", release[i].tv_sec, release[i].tv_nsec);

				if (i < stats->ovhd3_idx)
					fprintf(fp, "ovhd3: %ld.%09ld\n", fake[i].tv_sec, fake[i].tv_nsec);

				if (i < stats->ovhd4_idx)
					fprintf(fp, "ovhd4: %ld.%09ld\n", complete[i].tv_sec, complete[i].tv_nsec);
					
				if (i < idx - 1)
					fprintf(fp, "ovhd5: %ld.%09ld\n", ovhd5[i].tv_sec, ovhd5[i].tv_nsec);		

			}
			/*
			for (int i = 0; i < stats->ovhd1_idx; i++) {
				fprintf(fp, "ovhd1: %ld.%09ld\n", trace[i].tv_sec, trace[i].tv_nsec);
			}
			for (int i = 0; i < stats->ovhd2_idx; i++) {
				fprintf(fp, "ovhd2: %ld.%09ld\n", release[i].tv_sec, release[i].tv_nsec);
			}
			for (int i = 0; i < stats->ovhd3_idx; i++) {
				fprintf(fp, "ovhd3: %ld.%09ld\n", fake[i].tv_sec, fake[i].tv_nsec);
			}
			for (int i = 0; i < stats->ovhd4_idx; i++) {
				fprintf(fp, "ovhd4: %ld.%09ld\n", complete[i].tv_sec, complete[i].tv_nsec);
			}
			
			for (int i = 0; i < idx-1; i++) {
				fprintf(fp, "ovhd5: %ld.%09ld\n", ovhd5[i].tv_sec, ovhd5[i].tv_nsec);
			}*/
		}

		fprintf(fp, "job_idx: %d\n", stats->job_idx);
		fprintf(fp, "trace_idx: %d\n", stats->trace_idx);
		fprintf(fp, "release_idx: %d\n", stats->release_idx);
		fprintf(fp, "complete_idx: %d\n", stats->complete_idx);
		fprintf(fp, "fake_idx: %d\n", stats->fake_idx);
		fprintf(fp, "ovhd1_idx: %d\n", stats->ovhd1_idx);
		fprintf(fp, "ovhd2_idx: %d\n", stats->ovhd2_idx);
		fprintf(fp, "ovhd3_idx: %d\n", stats->ovhd3_idx);
		fprintf(fp, "ovhd4_idx: %d\n", stats->ovhd4_idx);
		fprintf(fp, "ovhd5_idx: %d\n", idx);

		fprintf(fp, "miss: %d\n", stats->miss);
		fprintf(fp, "meet: %d\n", stats->meet);
		fprintf(fp, "m: %d\n", stats->m);
		fprintf(fp, "K: %d\n", stats->K);
		fprintf(fp, "C: %ld.%09ld\n", stats->init_release.tv_sec, stats->init_release.tv_nsec);
		fprintf(fp, "T: %ld.%09ld\n", stats->cancel.tv_sec, stats->cancel.tv_nsec);
	}

	fclose(fp);
	printf("Data file is saved.\n");
	//pthread_mutex_unlock(&mutex);
}

// Sample task - matrix multiplication
void matrix_multiplicatiion(int pid) {
//	printf("Job ID %d, Matrix start.\n", pid);

	int m, n, p, q, c, d, k, sum = 0;
  	int first[MATRIX_SIZE][MATRIX_SIZE], second[MATRIX_SIZE][MATRIX_SIZE], multiply[MATRIX_SIZE][MATRIX_SIZE];
 
  	m = MATRIX_SIZE; n = MATRIX_SIZE;
  	p = MATRIX_SIZE; q = MATRIX_SIZE;

  	for (c = 0; c < m; c++)
    	for (d = 0; d < n; d++)
      		first[c][d] = 10;
 
    for (c = 0; c < p; c++)
    	for (d = 0; d < q; d++)
      		second[p][q] = 10;

  
    for (c = 0; c < m; c++) {
      for (d = 0; d < q; d++) {
        for (k = 0; k < p; k++) {
          sum = sum + first[c][k]*second[k][d];
        }
 
        multiply[c][d] = sum;
        sum = 0;
      }
    }
 
//  	printf("Job ID %d, Matrix finish.\n", pid);
}

int request_data_to_kernel(int tid1, struct timespec *setjmp_time, int idx) {
//void request_data_to_kernel(int tid1) {
	struct task_stats *buff_stats;
	struct timespec *buff_trace, *buff_release, *buff_complete, *buff_fake;
	struct timespec *buff_ovhd1, *buff_ovhd2, *buff_ovhd3, *buff_ovhd4, *buff_ovhd5;
	int re, index, terminate_flag = 0;

	// Allocate memory for buff
	buff_stats = malloc(sizeof(struct task_stats));
	buff_trace = malloc(sizeof(struct timespec)*TASK_TRACE_BUFF);
	buff_release = malloc(sizeof(struct timespec)*TASK_REL_COM_BUFF);
	buff_complete = malloc(sizeof(struct timespec)*TASK_REL_COM_BUFF);
	buff_fake = malloc(sizeof(struct timespec)*TASK_TRACE_BUFF);
	buff_ovhd1 = malloc(sizeof(struct timespec)*TASK_TRACE_BUFF);
	buff_ovhd2 = malloc(sizeof(struct timespec)*TASK_TRACE_BUFF);
	buff_ovhd3 = malloc(sizeof(struct timespec)*TASK_TRACE_BUFF);
	buff_ovhd4 = malloc(sizeof(struct timespec)*TASK_TRACE_BUFF);
	//buff_ovhd5 = malloc(sizeof(struct timespec)*TASK_TRACE_BUFF);


	//int tid1 = syscall(SYS_gettid);
	printf("Copy data of tid is %d\n", tid1);
	
	// Request copy data using syscall
	if (MONITOR_MODE == 1) {
		re = syscall(COPY_DATA_TO_USER, (void *)buff_stats, (void *)buff_trace, 
									(void *)buff_release, (void *)buff_complete, 
									(void *)buff_fake);
	} else {
		re = syscall(COPY_DATA_TO_USER, (void *)buff_stats, (void *)buff_ovhd1, 
									(void *)buff_ovhd2, (void *)buff_ovhd4, 
									(void *)buff_ovhd3);
	}
	
	

	if (re == 0) {
		#if(DISPLAY_MSG)
		printf("Data is copied from kernel.\n");
		#endif
		// Save data into a txt file
		if (MONITOR_MODE == 1) {
			save_data((struct task_stats *)buff_stats, (struct timespec *)buff_trace, (struct timespec *)buff_release, 
					  (struct timespec *)buff_complete, (struct timespec *)buff_fake, setjmp_time, idx, tid1);	
			//save_data((struct task_stats *)buff_stats, (struct timespec *)buff_trace, (struct timespec *)buff_release, 
			//		  (struct timespec *)buff_complete, (struct timespec *)buff_fake, tid1);	
		} else {
			save_data((struct task_stats *)buff_stats, (struct timespec *)buff_ovhd1, (struct timespec *)buff_ovhd2, 
				  (struct timespec *)buff_ovhd4, (struct timespec *)buff_ovhd3, setjmp_time, idx, tid1);	
			//save_data((struct task_stats *)buff_stats, (struct timespec *)buff_ovhd1, (struct timespec *)buff_ovhd2, 
			//	  (struct timespec *)buff_ovhd4, (struct timespec *)buff_ovhd3, tid1);	
		}
		
	} else {
		#if(DISPLAY_MSG)
		printf("Data is not copied correctly.\n");
		#endif
	}

	free(buff_stats);
	free(buff_trace);
	free(buff_release);
	free(buff_complete);
	free(buff_fake);
	free(buff_ovhd1);
	free(buff_ovhd2);
	free(buff_ovhd3);
	free(buff_ovhd4);
	//free(buff_ovhd5);
	return 0;
}

// Canceling a thread
int cancel_rsv_task() {
	struct timespec dummy;

	int ret = syscall(CANCEL_RSV_DEV, (void *)&dummy);
	if (ret < 0) {
		#if(DISPLAY_MSG)
		printf("Thread is not canceled correctly.\n");
		#endif
		return -1;
	}
	#if(DISPLAY_MSG)
	printf("Thread with PID of %d is canceled.\n", ret);
	#endif
	
	return 0;
}