#include "init_task.h"

int compare (const void *pa, const void *pb) {
    const int *a = pa;
    const int *b = pb;
    if(a[0] == b[0])
        return a[1] - b[1];
    else
        return a[0] - b[0];
}

void init_tasksets(struct set_task_param *set_var, char **argv) {
	long double tmp_p, max_p = 0.0;
	int num_cls[num_tasks];
	int i, j, cnt = 0, max_num_cls = 0;

	// Save periods of tasks
	int periods[num_tasks][2];
	for (i = 0; i < num_tasks; i++) {
		set_var[i].C.tv_sec = atol(argv[4*i+1])/1000;
		set_var[i].C.tv_nsec = (atol(argv[4*i+1])%1000)*1000000;
		set_var[i].T.tv_sec = atol(argv[4*i+2])/1000;
		set_var[i].T.tv_nsec = (atol(argv[4*i+2])%1000)*1000000;

		set_var[i].m = atoi(argv[4*i+3]);
		set_var[i].K = atoi(argv[4*(i+1)]);

		set_var[i].w = MAX((int)(floor(set_var[i].K/(set_var[i].K-set_var[i].m)) - 1), 1);

		tmp_p = set_var[i].T.tv_sec + set_var[i].T.tv_nsec/1000000000.0;
		periods[i][0] = (int)(tmp_p*1000);
		periods[i][1] = i;
		if (tmp_p > max_p) {
			max_p = tmp_p;
		}
		//printf("Periods of index %d task is %ld.%09ld sec\n", i, set_var[i].T.tv_sec, set_var[i].T.tv_nsec);
	}

	
	// Sort tasks by periods (ascending order) and assign RT priority to tasks
	qsort(periods, num_tasks, sizeof(periods[0]), compare);
	for (i = 0; i < num_tasks; i++) {
		num_cls[i] = set_var[i].K - set_var[i].m + 1;
		set_var[i].num_job_cls = num_cls[i];
		printf("Number of classes of task %d is %d\n", i, set_var[i].num_job_cls);
		if (num_cls[i] > max_num_cls)
			max_num_cls = num_cls[i];
	}

	// Assign job-class priorities
	for (i = 0; i < 10; i++) {
		for (j = 0;  j < num_tasks; j++) {
			if (i < num_cls[periods[j][1]]) {
				set_var[periods[j][1]].rt_pri[i]= RT_PRIORITY_MAX - cnt;
				//printf("Priority of job-class index %d of tasks %d is %d\n", i, periods[j][1], set_var[periods[j][1]].rt_pri[i]);
				cnt++;
			} else {
				set_var[periods[j][1]].rt_pri[i]= 0;
			}
			//printf("Priority of job-class index %d of tasks %d is %d\n", i, periods[j][1], set_var[periods[j][1]].rt_pri[i]);
		}
	}
	
	// Determine the number repetitions for eash threads
	for (int i = 0; i < num_tasks; i++) {
		//set_var[i].num_repeats = (unsigned int)(max_p*10/(set_var[i].T.tv_sec + set_var[i].T.tv_nsec/1000000000.0));
		set_var[i].num_repeats = (unsigned int)(EXP_TIME/(set_var[i].T.tv_sec + set_var[i].T.tv_nsec/1000000000.0));
		set_var[i].job_mode = job_mode;
		printf("Repeatiton of task %d is  %d\n", i, set_var[i].num_repeats);
	}
}