#include "jcls.h"

//unsigned int *num_tasks;
int num_classes;

// Sort ascending order of tasks by period
int sort_ascend(const void *a, const void *b) {
	struct task_ascend *a1 = (struct task_ascend *)a;
	struct task_ascend *b1 = (struct task_ascend *)b;

	if ((*a1).period > (*b1).period) 
		return 1;
	else if ((*a1).period < (*b1).period)
		return -1;
	else
		return 0;
}

int sort_descend(const void *a, const void *b) {
	struct priority_descend *a1 = (struct priority_descend *)a;
	struct priority_descend *b1 = (struct priority_descend *)b;

	if ((*a1).priority > (*b1).priority) 
		return -1;
	else if ((*a1).priority < (*b1).priority)
		return 1;
	else
		return 0;
}

int sort_descend_bin(const void *a, const void *b) {
	struct array_sort_float *a1 = (struct array_sort_float *)a;
	struct array_sort_float *b1 = (struct array_sort_float *)b;

	if ((*a1).value > (*b1).value)
		return -1;
	else if ((*a1).value < (*b1).value)
		return 1;
	else
		return 0;
	/*		
	float aa = *(float*)a;
	float bb = *(float*)b;

	if (aa > bb) return -1;
	if (aa < bb) return 1;

	return 0;
	*/
}

void sort_descend_min_util(struct task *task, unsigned int num_tasks) {
	int i, j;
	struct task temp;

	for (i = 0; i < num_tasks-1; i++) {
		for (j = 0; j < (num_tasks-1-i); j++) {
			if (task[j].min_utilization < task[j+1].min_utilization) {
				temp = task[j];
				task[j] = task[j+1];
				task[j+1] = temp;
			}
		}
	}
}

void sort_descend_util(struct task *task, unsigned int num_tasks) {
	int i, j;
	struct task temp;

	for (i = 0; i < num_tasks-1; i++) {
		for (j = 0; j < (num_tasks-1-i); j++) {
			if (task[j].utilization < task[j+1].utilization) {
				temp = task[j];
				task[j] = task[j+1];
				task[j+1] = temp;
			}
		}
	}
}

void sort_ascend_period(struct task *task, unsigned int num_tasks) {
	int i, j;
	struct task temp;

	for (i = 0; i < num_tasks-1; i++) {
		for (j = 0; j < (num_tasks-1-i); j++) {
			if (task[j].T > task[j+1].T) {
				temp = task[j];
				task[j] = task[j+1];
				task[j+1] = temp;
			}
		}
	}
}



// Sort ascending order of tasks by miss threshold
int sort_miss_ascend(const void *a, const void *b) {
	struct miss_ascend *a1 = (struct miss_ascend *)a;
	struct miss_ascend *b1 = (struct miss_ascend *)b;

	if ((*a1).miss_thres > (*b1).miss_thres) 
		return 1;
	else if ((*a1).miss_thres < (*b1).miss_thres)
		return -1;
	else
		return 0;
}


// Sort dscending order of job class priority
void sort_job_priority_des (unsigned int *arr, unsigned int *index) {
	int i, j, temp, temp_idx;

	for (i = 0; i < num_classes; i++) {
        for (j = i + 1; j < num_classes; j++) {
            if (arr[i] < arr[j]) {
                temp = arr[i];
                temp_idx = index[i];

                arr[i] = arr[j];
                index[i] = index[j];

                arr[j] = temp;
                index[j] = temp_idx;
            }
        }
    }
}

// Assign heuristic job-class priority
int assign_hst_priority(struct task *tasks, unsigned int num_tasks) {
	int re, i, j, c_priority = 0, flag, hst_flag = 0;
	int *c_index;
	struct task_rspt *resp_time;
	resp_time = malloc(sizeof(struct task_rspt)*num_tasks);
	c_index = malloc(sizeof(int)*num_tasks);

	struct task_ascend *task_period;
	task_period = malloc(sizeof(struct task_ascend)*num_tasks);

	for (i = 0; i < num_tasks; i++) {
		task_period[i].period = tasks[i].T;
		task_period[i].index = i;

		c_priority = c_priority + (tasks[i].m + 1);
		c_index[i] = 0;
	}

	// Sort tasks by period
	qsort(task_period, num_tasks, sizeof(task_period[0]), sort_ascend);

	// Check RM schedulability of tasks
	WCRT_RM(tasks, resp_time, num_tasks);

	for (i = 0; i < num_tasks; i++) {
		if (resp_time[i].sched == 0) {
			hst_flag = 1;
		}
	}

	if (hst_flag) {
		// Heuristic priority assignment
		flag = 1;
		while (flag) {
			for (i = 0; i < num_tasks; i++) {
				if (c_index[task_period[i].index] < (tasks[task_period[i].index].m+1)) {
					tasks[task_period[i].index].priorities[c_index[task_period[i].index]] = c_priority;
					c_priority -= 1;
					c_index[task_period[i].index] += 1;
				}
			}
			if (c_priority < 1)
				flag = 0;
		}
	} else {
		// RM priority assignment
		for (i = 0; i < num_tasks; i++) {
			for (j = 0; j < (tasks[task_period[i].index].m+1); j++) {
				tasks[task_period[i].index].priorities[j] = c_priority;
				c_priority -= 1;
			}
		}
	}

	#if(DEBUG_PRIORITY)
		for (i = 0; i < num_tasks; i++) {
			for (j = 0; j < tasks[i].m+1; j++) {
				printf("Priority of job %d of task %d is %d\n", j, i, tasks[i].priorities[j]); 
			}
		}
	#endif

	free(resp_time);
	free(c_index);
	free(task_period);
	return 0;
}

// Assign heuristic job-class priority
int assign_hst_priority_w(struct task *tasks, unsigned int num_tasks) {
	int re, i, j, m, q, c_priority = 0, flag, hst_flag = 0;
	int *c_index;
	struct task_rspt *resp_time;
	resp_time = malloc(sizeof(struct task_rspt)*num_tasks);
	c_index = malloc(sizeof(int)*num_tasks);
	
	struct task_ascend *task_period;
	task_period = malloc(sizeof(struct task_ascend)*num_tasks);

	struct miss_ascend *m_threshold;
	m_threshold = malloc(sizeof(struct miss_ascend)*num_tasks);

	for (i = 0; i < num_tasks; i++) {
		task_period[i].period = tasks[i].T;
		task_period[i].index = i;

		c_priority = c_priority + (tasks[i].m + 1);
		c_index[i] = 0;

		// Assign miss threshold
		m_threshold[i].miss_thres = tasks[i].w;
		m_threshold[i].index = i;
	}

	// Sort tasks by period
	qsort(task_period, num_tasks, sizeof(task_period[0]), sort_ascend);
	
	// Sort tasks by miss threshold window
	qsort(m_threshold, num_tasks, sizeof(m_threshold[0]), sort_miss_ascend);
	/*for (i = 0; i < num_tasks; i++) {
		printf("m_threshold (sorted) : %d\n", m_threshold[i].index);
	}*/


	// Check RM schedulability of tasks
	WCRT_RM(tasks, resp_time, num_tasks);

	for (i = 0; i < num_tasks; i++) {
		if (resp_time[i].sched == 0) {
			hst_flag = 1;
		}
	}

	if (hst_flag) {
		// Heuristic priority assignment
		flag = 1;
		q = 0;
		while (flag) {
			if (q == 0) {
				// For the lowest index of job-class
				for (i = 0; i < num_tasks; i++) {
					if (c_index[task_period[i].index] < (tasks[task_period[i].index].m+1)) {
						tasks[task_period[i].index].priorities[c_index[task_period[i].index]] = c_priority;
						c_priority -= 1;
						c_index[task_period[i].index] += 1;
					}
				}
			} else {
			
				for (i = 0; i < num_tasks; i++) {
					if (c_index[m_threshold[i].index] < (tasks[m_threshold[i].index].m+1)) {
						tasks[m_threshold[i].index].priorities[c_index[m_threshold[i].index]] = c_priority;
						c_priority -= 1;
						c_index[m_threshold[i].index] += 1;
					}
				}
			}
			
			if (c_priority < 1)
				flag = 0;

			q++;
		}
	} else {
		// RM priority assignment
		for (i = 0; i < num_tasks; i++) {
			for (j = 0; j < (tasks[task_period[i].index].m+1); j++) {
				tasks[task_period[i].index].priorities[j] = c_priority;
				c_priority -= 1;
			}
		}
	}

	#if(DEBUG_PRIORITY)
		for (i = 0; i < num_tasks; i++) {
			for (j = 0; j < tasks[i].m+1; j++) {
				printf("Priority of job %d of task %d is %d\n", j, i, tasks[i].priorities[j]); 
			}
		}
	#endif

	free(resp_time);
	free(c_index);
	free(task_period);
	return 0;
}

// Assign heuristic job-class priority
int assign_hst_priority_v(struct task *tasks, unsigned int num_tasks) {
	int re, i, j, m, q, c_priority = 0, flag, hst_flag = 0;
	int *c_index;
	struct task_rspt *resp_time;
	resp_time = malloc(sizeof(struct task_rspt)*num_tasks);
	c_index = malloc(sizeof(int)*num_tasks);
	
	struct task_ascend *task_period;
	task_period = malloc(sizeof(struct task_ascend)*num_tasks);

	struct miss_ascend *m_threshold;
	m_threshold = malloc(sizeof(struct miss_ascend)*num_tasks);

	for (i = 0; i < num_tasks; i++) {
		task_period[i].period = tasks[i].T;
		task_period[i].index = i;

		c_priority = c_priority + (tasks[i].m + 1);
		c_index[i] = 0;

		// Assign miss threshold
		m_threshold[i].miss_thres = tasks[i].w;
		m_threshold[i].index = i;
	}

	// Sort tasks by period
	qsort(task_period, num_tasks, sizeof(task_period[0]), sort_ascend);
	
	// Sort tasks by miss threshold window
	//qsort(m_threshold, num_tasks, sizeof(m_threshold[0]), sort_miss_ascend);
	/*for (i = 0; i < num_tasks; i++) {
		printf("m_threshold (sorted) : %d\n", m_threshold[i].index);
	}*/


	// Check RM schedulability of tasks
	WCRT_RM(tasks, resp_time, num_tasks);

	for (i = 0; i < num_tasks; i++) {
		if (resp_time[i].sched == 0) {
			hst_flag = 1;
		}
	}

	//printf("%d\n", c_priority);
	if (hst_flag) {
		// Heuristic priority assignment
		flag = 1;
		while (flag) {
			// For the lowest index of job-class
			for (i = 0; i < num_tasks; i++) {
				if (c_index[i] < (tasks[i].m+1)) {
					tasks[i].priorities[c_index[i]] = c_priority;
					c_priority -= 1;
					c_index[i] += 1;
				}
			}
			if (c_priority < 1)
				flag = 0;
		}
		
		// Initialize c_index, again
		for (i = 0; i < num_tasks; i++) {
			c_index[i] = 0;
		}
		
		flag = 1;
		while (flag) {			
			for (i = 0; i < num_tasks; i++) {
				int pri_flag = 0;
				for (j = 0; j < tasks[i].v; j++) {
					if (c_index[i] < (tasks[i].m+1)) {
						//tasks[i].priorities[c_index[i]] = c_priority;
						if (j != 0) {
							tasks[i].priorities[c_index[i]] = tasks[i].priorities[c_index[i]-1];
						}
						c_index[i] += 1;
						//pri_flag = 1;
					}
				}

				//if (pri_flag == 1)
				//c_priority -= 1;
			}

			flag = 0;
			for (i = 0; i < num_tasks; i++) {
				if (c_index[i] < (tasks[i].m+1)) {
					flag = 1;
				}
			}
		}
	} else {
		// RM priority assignment
		for (i = 0; i < num_tasks; i++) {
			for (j = 0; j < (tasks[task_period[i].index].m+1); j++) {
				tasks[task_period[i].index].priorities[j] = c_priority;
				c_priority -= 1;
			}
		}
	}

	free(resp_time);
	free(c_index);
	free(task_period);
	return 0;
}

int WFD_allocation(struct task *tasks, unsigned int num_tsks, unsigned int max_bin) {
	struct array_sort_float bin[max_bin];
	int assigned;
	int unsched = 0;

	// Sort taskset by descending order of minimum utilization
	sort_descend_min_util(tasks, num_tsks);

	/*	
	for (int i = 0; i < num_tsks; i++) {
		printf("Task period is %Lf: min_utilization is %f\n", tasks[i].T, tasks[i].min_utilization);
	}
	*/

	for (int i = 0; i < max_bin; i++) {
		bin[i].value = 1.0;
		bin[i].index = i;
	}

	for (int i = 0; i < num_tsks; i++) {
		assigned = 0;
		if (i == 0) {
			bin[i].value = bin[i].value - tasks[i].min_utilization;
			for (int k = 0; k < tasks[i].m+1; k++)
				tasks[i].cpu[k] = 1;
			
			assigned = 1;
		} else {
			qsort(bin, max_bin, sizeof(bin[0]), sort_descend_bin);
			/*
			for (int k = 0; k < max_bin; k++) {
				printf("Bin is %f, index %d\n", bin[k].value, bin[k].index);
			}*/

			for (int j = 0; j < max_bin; j++) {
				unsigned int index = bin[j].index;
				if (bin[j].value - tasks[i].min_utilization >= 0) {
					bin[j].value = bin[j].value - tasks[i].min_utilization;
					for (int k = 0; k < tasks[i].m+1; k++)
						tasks[i].cpu[k] = index+1;
					
					assigned = 1;
					break;
				}
			}

			if (assigned != 1) {
				unsched = 1;
				/*
				unsigned int index = bin[max_bin-1].index;
				bin[max_bin-1].value = bin[max_bin-1].value - tasks[i].min_utilization;
				for (int k = 0; k < tasks[i].m+1; k++)
					tasks[i].cpu[k] = index+1;
			
				assigned = 1;
				*/
			}
		}


	}
	
	sort_ascend_period(tasks, num_tsks);
	/*
	for (int i = 0; i < num_tsks; i++) {
		printf("Task period is %Lf: min_utilization is %f: CPU is %d\n", tasks[i].T, tasks[i].min_utilization, tasks[i].cpu[0]);
	}*/
	return unsched;
	
}

int WFD_allocation_U(struct task *tasks, unsigned int num_tsks, unsigned int max_bin) {
	struct array_sort_float bin[max_bin];
	int assigned;
	int unsched = 0;
	
	// Sort taskset by descending order of minimum utilization
	sort_descend_util(tasks, num_tsks);

	/*	
	for (int i = 0; i < num_tsks; i++) {
		printf("Task period is %Lf: min_utilization is %f\n", tasks[i].T, tasks[i].min_utilization);
	}
	*/

	for (int i = 0; i < max_bin; i++) {
		bin[i].value = 1.0;
		bin[i].index = i;
	}

	for (int i = 0; i < num_tsks; i++) {
		assigned = 0;
		if (i == 0) {
			bin[i].value = bin[i].value - tasks[i].utilization;
			for (int k = 0; k < tasks[i].m+1; k++)
				tasks[i].cpu[k] = 1;
			
			assigned = 1;
		} else {
			qsort(bin, max_bin, sizeof(bin[0]), sort_descend_bin);
			/*
			for (int k = 0; k < max_bin; k++) {
				printf("Bin is %f, index %d\n", bin[k].value, bin[k].index);
			}*/

			for (int j = 0; j < max_bin; j++) {
				unsigned int index = bin[j].index;
				if (bin[j].value - tasks[i].utilization >= 0) {
					bin[j].value = bin[j].value - tasks[i].utilization;
					for (int k = 0; k < tasks[i].m+1; k++)
						tasks[i].cpu[k] = index+1;
					
					assigned = 1;
					break;
				}
			}

			if (assigned != 1) {
				unsched = 1;
				/*
				unsigned int index = bin[max_bin-1].index;
				bin[max_bin-1].value = bin[max_bin-1].value - tasks[i].utilization;
				for (int k = 0; k < tasks[i].m+1; k++)
					tasks[i].cpu[k] = index+1;
			
				assigned = 1;
				*/
			}
		}


	}
	
	sort_ascend_period(tasks, num_tsks);
	/*
	for (int i = 0; i < num_tsks; i++) {
		printf("Task period is %Lf: min_utilization is %f: CPU is %d\n", tasks[i].T, tasks[i].min_utilization, tasks[i].cpu[0]);
	}*/
	return unsched;
}
/*
int Job_allocation(struct task *tasks, unsigned int num_tsks, unsigned int max_bin) {
	int i, j, tmp_idx = 0, max_j = 0, max_c = 0;
	unsigned int *job_cls_tid, *job_cls_idx;
	struct priority_descend *job_priority;
	int assigned, task_id, job_class_id, prev_priority, prev_cpu, unsched = 0;
	int **cpu;

	job_cls_tid = malloc(sizeof(unsigned int)*num_classes);
	job_cls_idx = malloc(sizeof(unsigned int)*num_classes);
	job_priority = malloc(sizeof(struct priority_descend)*num_classes);
	cpu = (int **)malloc(sizeof(int)*max_bin);
	for(i = 0; i < max_bin; i++) {
		cpu[i] = (int *)malloc(sizeof(int)*num_tsks);
		for (int j = 0; j < num_tsks; j++)
			cpu[i][j] = 0;
	}


	for (i = 0; i < num_tsks; i++) {
		if ((tasks[i].m+1) > max_j)
			max_j = tasks[i].m + 1;

		for (j = 0; j < tasks[i].m+1; j++) {
			job_priority[tmp_idx].priority = tasks[i].priorities[j];
			job_priority[tmp_idx].index = tmp_idx;
			job_cls_tid[tmp_idx] = i;
			job_cls_idx[tmp_idx] = j;
			tmp_idx++;
		}
	}

	//sort_job_priority_des(job_cls_priority, order_idx);
	qsort(job_priority, num_classes, sizeof(job_priority[0]), sort_descend);

	
	float bin[max_bin];
	for (int i = 0; i < max_bin; i++) {
		bin[i] = 1.0;
	}

	//prev_priority = 0; prev_cpu = 0;
	for (i = 0; i < num_classes; i++) {
		assigned = 0;
		task_id = job_cls_tid[job_priority[i].index];
		job_class_id = job_cls_idx[job_priority[i].index];


		if (assigned == 0) {
			for (int j = 0; j < max_bin; j++) {
				tasks[task_id].cpu[job_class_id] = j+1;
				WCRT_CLASS(tasks, task_id, job_class_id, num_tsks);

				if (tasks[task_id].WCRT[job_class_id] <= tasks[task_id].T) {
					cpu[j][task_id] = 1;
					assigned = 1;
//					prev_cpu = j+1;
					break;
				}
				tasks[task_id].cpu[job_class_id] = 0;
				tasks[task_id].WCRT[job_class_id] = 0.0;
			}

			if (assigned == 0) {
				for (int j = 0; j < max_bin; j++) {
					
					float tmp_util = 0.0; bin[j] = 1.0;
					for (int k = 0; k < num_tsks; k++) {
						for (int n = 0; n < tasks[k].m+1; n++) {
							if (tasks[k].cpu[n] == j+1) {
								bin[j] -= tasks[k].min_utilization;
								break;
							}								
						}
					}
					

					if (cpu[j][task_id] == 1 || (cpu[j][task_id] == 0 && (bin[j]-tasks[task_id].min_utilization >= 0))) {
						cpu[j][task_id] = 1;
						tasks[task_id].cpu[job_class_id] = j+1;
						assigned = 1;
//						prev_cpu = j+1;
						break;
					}
				}
			}

			if (assigned == 0) {
				float tmp_bin = -1.0;
				int tmp_cpu = 0;
				for (int k = 0; k < max_bin; k++) {
					if (bin[k] > tmp_bin) {
						tmp_bin = bin[k];
						tmp_cpu = k + 1;
					}						
				}
				tasks[task_id].cpu[job_class_id] = tmp_cpu;
				//unsched = 1;
				printf("Unschedulable taskset!!!\n");
			}
		}
//		prev_priority = tasks[task_id].priorities[job_class_id];
		//printf("CPU of job-class %d of task %d is %d\n", job_class_id, task_id, tasks[task_id].cpu[job_class_id]);
	}

	for (int i = 0; i < max_bin; i++)
		free(cpu[i]);
	free(cpu);
	free(job_cls_tid);
	free(job_cls_idx);
	free(job_priority);

	return unsched;
}
*/
int Job_allocation(struct task *tasks, unsigned int num_tsks, unsigned int max_bin) {
	int i, j, tmp_idx = 0, max_j = 0, max_c = 0;
	unsigned int *job_cls_tid, *job_cls_idx;
	struct priority_descend *job_priority;
	int assigned, task_id, job_class_id, prev_priority, prev_cpu, unsched = 0;
	//int **cpu;

	job_cls_tid = malloc(sizeof(unsigned int)*num_classes);
	job_cls_idx = malloc(sizeof(unsigned int)*num_classes);
	job_priority = malloc(sizeof(struct priority_descend)*num_classes);
	/*
	cpu = (int **)malloc(sizeof(int)*max_bin);
	for(i = 0; i < max_bin; i++) {
		cpu[i] = (int *)malloc(sizeof(int)*num_tsks);
		for (int j = 0; j < num_tsks; j++)
			cpu[i][j] = 0;
	}
	*/

	for (i = 0; i < num_tsks; i++) {
		if ((tasks[i].m+1) > max_j)
			max_j = tasks[i].m + 1;

		for (j = 0; j < tasks[i].m+1; j++) {
			job_priority[tmp_idx].priority = tasks[i].priorities[j];
			job_priority[tmp_idx].index = tmp_idx;
			job_cls_tid[tmp_idx] = i;
			job_cls_idx[tmp_idx] = j;
			tmp_idx++;
		}
	}

	//sort_job_priority_des(job_cls_priority, order_idx);
	qsort(job_priority, num_classes, sizeof(job_priority[0]), sort_descend);

	
	float U_p[max_bin];

	//prev_priority = 0; prev_cpu = 0;
	for (i = 0; i < num_classes; i++) {
		assigned = 0;
		task_id = job_cls_tid[job_priority[i].index];
		job_class_id = job_cls_idx[job_priority[i].index];

		if (assigned == 0) {
			for (int j = 0; j < max_bin; j++) {
				tasks[task_id].cpu[job_class_id] = j+1;
				WCRT_CLASS(tasks, task_id, job_class_id, num_tsks);

				if (tasks[task_id].WCRT[job_class_id] <= tasks[task_id].T) {
					if (job_class_id == 0)
						tasks[task_id].eta[job_class_id] = (tasks[task_id].w + 1)*tasks[task_id].T;
					else if (job_class_id > 0 && job_class_id < tasks[task_id].m)
						tasks[task_id].eta[job_class_id] = (job_class_id + 2)*tasks[task_id].T;
					else
						tasks[task_id].eta[job_class_id] = tasks[task_id].T;


					//cpu[j][task_id] = 1;
					assigned = 1;
					break;
				}
				tasks[task_id].cpu[job_class_id] = 0;
				tasks[task_id].WCRT[job_class_id] = 0.0;
			}

			if (assigned == 0) {
				float U_min = INFINITY;
				for (int j = 0; j < max_bin; j++) {					
					// Calculate U_p	
					U_p[j] = 0.0;
					for (int k = 0; k < num_tsks; k++) {
						for (int n = 0; n < tasks[k].m+1; n++) {
							if (tasks[k].cpu[n] == j+1) {
								U_p[j] += tasks[k].C/tasks[k].eta[n];
							}								
						}
					}

					if (U_p[j] < U_min) {
						U_min = U_p[j];
						tasks[task_id].cpu[job_class_id] = j+1;
					}				
				}
			}

			
		}
//		prev_priority = tasks[task_id].priorities[job_class_id];
		//printf("CPU of job-class %d of task %d is %d\n", job_class_id, task_id, tasks[task_id].cpu[job_class_id]);
	}
/*
	for (int i = 0; i < max_bin; i++)
		free(cpu[i]);
	free(cpu);
*/
	free(job_cls_tid);
	free(job_cls_idx);
	free(job_priority);

	return unsched;
}


void WCRT_RM(struct task *tasks, struct task_rspt *resp_time, unsigned int num_tasks) {
	#if(DEBUG_WCRT)
		printf("Start of WCRT_RM\n");
	#endif

	int i, j, flag = 1;
	long double w_total;

	for (i = 0; i < num_tasks; i++) {
		resp_time[i].rspt = tasks[i].C;
		resp_time[i].rspt_prev = tasks[i].C;
		
		while (flag) {
			w_total = 0;

			for (j = 0; j < num_tasks; j++) {
				//if (tasks[j].T < tasks[i].T) {
				if (j < i) {
					w_total = w_total + (double)(ceil(resp_time[i].rspt/tasks[j].T))*tasks[j].C;
				}
			}

			if ((resp_time[i].rspt <= tasks[i].T) && ((tasks[i].C + w_total) > resp_time[i].rspt)) {
				resp_time[i].rspt = tasks[i].C + w_total;
			} else if ((resp_time[i].rspt <= tasks[i].T) && ((tasks[i].C + w_total) <= resp_time[i].rspt)) {
				resp_time[i].rspt = tasks[i].C + w_total;
				resp_time[i].sched = 1;
				break;
			} else if (resp_time[i].rspt > tasks[i].T) {
				resp_time[i].sched = 0;
				break;
			}
			
		}
		#if(DEBUG_WCRT)
			printf("WCRT_RM response time of task %d is %Lf\n", i, resp_time[i].rspt);
		#endif
	}

	#if(DEBUG_WCRT)
		printf("End of RM\n");
	#endif
}

void WCRT(struct task *tasks, unsigned int num_tasks) {
	#if(DEBUG_WCRT)
		printf("Start of WCRT\n");
	#endif

	int i, j, tmp_idx = 0, max_j = 0, max_c = 0;
	unsigned int *job_cls_tid, *job_cls_idx;
	struct priority_descend *job_priority;

	job_cls_tid = malloc(sizeof(unsigned int)*num_classes);
	job_cls_idx = malloc(sizeof(unsigned int)*num_classes);
	job_priority = malloc(sizeof(struct priority_descend)*num_classes);

	for (i = 0; i < num_tasks; i++) {
		if ((tasks[i].m+1) > max_j)
			max_j = tasks[i].m + 1;

		for (j = 0; j < tasks[i].m+1; j++) {
			job_priority[tmp_idx].priority = tasks[i].priorities[j];
			job_priority[tmp_idx].index = tmp_idx;
			job_cls_tid[tmp_idx] = i;
			job_cls_idx[tmp_idx] = j;
			tmp_idx++;
		}
	}

	//sort_job_priority_des(job_cls_priority, order_idx);
	qsort(job_priority, num_classes, sizeof(job_priority[0]), sort_descend);

	for (i = 0; i < num_classes; i++) {
		WCRT_CLASS(tasks, job_cls_tid[job_priority[i].index], job_cls_idx[job_priority[i].index], num_tasks);
	}

	#if(DEBUG_WCRT)
		for (i=0; i<num_classes; i++) {
			printf("Ordered job class priority is %d\n", job_cls_priority[i]);
			printf("Ordered class index is %d\n", order_idx[i]);
		}
	#endif

	free(job_cls_tid);
	free(job_cls_idx);
	free(job_priority);
}

void WCRT_CLASS(struct task *tasks, unsigned int tid, unsigned int idx, unsigned int num_tasks) {
	#if(DEBUG_WCRT)
		printf("Start of WCRT CLASS\n");
	#endif

	int i, j, invo_period, flag = 1;
	long double W, w, W_t;
	struct task_rspt resp_time;
	
	resp_time.job_rspt = tasks[tid].C;
	resp_time.job_rspt_prev = tasks[tid].C;
	while (flag) {
		W = 0.0;
		for (i = 0; i < num_tasks; i++) {
			w = 0.0;
			W_t = 0.0;
			if (i != tid) {
				for (j = 0; j < (tasks[i].m+1); j++) {
					if (tasks[i].priorities[j] > tasks[tid].priorities[idx] && tasks[i].cpu[j] == tasks[tid].cpu[idx]) {
						if (tasks[i].WCRT[j] <= tasks[i].T-tasks[i].jitter) {
							if (j == 0) {
								invo_period = tasks[i].w + 1;
							} else {
								invo_period = j + 2;
							}
						} else {
							if (tasks[i].w == 1) {
								invo_period = j + 1;
							} else {
								invo_period = 1;
							}
						}


						if (j == tasks[i].m) {
							invo_period = 1;
						}

						// Modified, need to be checked later
						double offset = j*tasks[i].T*tasks[i].v;

						w = w + (double)ceil((resp_time.job_rspt + tasks[i].jitter)/((long double)(invo_period)*tasks[i].T))*tasks[i].C;
					}
				}
				W_t = MIN(w, (double)ceil((resp_time.job_rspt + tasks[i].jitter)/tasks[i].T)*tasks[i].C);
			}
			W = W + W_t;
		}

		//resp_time.job_rspt_prev = resp_time.job_rspt;

		resp_time.job_rspt = tasks[tid].C + W;

		if (resp_time.job_rspt <= resp_time.job_rspt_prev) {
			resp_time.job_rspt = resp_time.job_rspt_prev;
			tasks[tid].WCRT[idx] = resp_time.job_rspt;
			flag = 0;
		}

		if (resp_time.job_rspt > tasks[tid].T*2) {
			tasks[tid].WCRT[idx] = resp_time.job_rspt;
			flag = 0;
		}

		resp_time.job_rspt_prev = resp_time.job_rspt;

	}
}
