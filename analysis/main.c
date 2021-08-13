#include "jcls.h"

int main(int argc, char *argv[]) {

	if (argc < 5) {
		printf("Usage: ./check_sched filename # of tasksets Util range \n");
		printf("Example: ./check_sched sample.txt 1000 11 sample_num.txt\n");
		return -1;
	}

	int i, j, k, u, re;
	unsigned int sched_cnt = 0;
	char rd_dir[64] = "../data/";
	char wr_dir[64] = "../data/";
	char num_dir[64] = "../data/";
	char *fname = argv[1];
	//num_tasks = atoi(argv[2]);
	num_tasksets = atoi(argv[2]);
	int util_range =atoi(argv[3]);
	char *fname_num = argv[4];
	unsigned int num_cpu = atoi(argv[5]);
	num_classes = 0;
	
	printf("File name is %s\n", fname_num);

	struct task *tasks;
	long double **data;
	num_tasks = malloc(sizeof(unsigned int)*num_tasksets);

	printf("Num of tasksets: %d\n", num_tasksets);
	FILE *fp, *fp_out, *fp_tasknum;
	strcat(rd_dir, fname);
	fp = fopen(rd_dir, "r");
	if (fp == NULL) {
		printf("Fail to open input data file.\n");
		return -1;
	}

	// Save the results on txt file
	char *name = strtok(fname, ".");
	strcat(wr_dir, name);
	strcat(wr_dir, "_out.txt");
	fp_out = fopen(wr_dir, "a+");
	if (fp_out == NULL) {
		printf("Fail to open output data file.\n");
		return -1;
	}
	
	strcat(num_dir, fname_num);
	fp_tasknum = fopen(num_dir, "r");
	if (fp == NULL) {
		printf("Fail to open input data file.\n");
		return -1;
	}
	
	
	// Time measurement
	struct timeval t1, t2;
	double elapsedTime, meanTime, maxTime = 0.0;
	int unsched = 0;
	for (u = 0; u < util_range; u++) {

		sched_cnt = 0;

		// Assign number of tasks in a taskset
		for (i = 0; i < num_tasksets; i++) {
			fscanf(fp_tasknum, "%d", &num_tasks[i]);
			//printf("Number of tasks is %d\n", num_tasks[i]);
		}

		for (k = 0; k < num_tasksets; k++) {
			
			// Allocate memory
			data = (long double **)malloc(sizeof(long double)*num_tasks[k]);
			for(i = 0; i < num_tasks[k]; i++) {
				data[i] = (long double *)malloc(sizeof(long double)*9);
			}
			
			tasks = malloc(sizeof(struct task)*num_tasks[k]);
			for (i = 0; i < num_tasks[k]; i++) {
				tasks[i].priorities = malloc(sizeof(unsigned int)*40);
				tasks[i].WCRT = malloc(sizeof(long double)*40);
			}

			// Load txt file to save data
		    re = load_data(fp, data, num_tasks[k]);
		    if (re < 0) {
		    	printf("Load text file error.\n");
		    	return -1;
		    }

		    // start timer for measuring
			gettimeofday(&t1, NULL);

		    // Initialize tasksets
		    for(i = 0; i < num_tasks[k]; i++) {
				initialize_task(&tasks[i], data[i][1], data[i][0], data[i][3], data[i][4], data[i][7]);
				#if(DEBUG_MAIN)
			    	printf("C: %Lf, T: %Lf, m: %d, K: %d\n", tasks[i].C, tasks[i].T, tasks[i].m, tasks[i].K);
		    	#endif
				//printf("C: %Lf, T: %Lf, m: %d, K: %d, v: %d\n", tasks[i].C, tasks[i].T, tasks[i].m, tasks[i].K, tasks[i].v);
				
		    }
		    
		    // Assign heuristic priorities
			re = assign_hst_priority_v(tasks, num_tasks[k]);
			#if(DEBUG_PRIORITY)
				for(i = 0; i < num_tasks[k]; i++) {
					for(j = 0; j < tasks[i].m+1; j++) {
						printf("Priority of job %d of task %d is %d\n", j, i, tasks[i].priorities[j]);
					}
				}
			#endif
		    if (re != 0) {
				printf("Priority assignment has an error.\n");
			}

			// Task(Job) allocation on CPU
			unsched = Job_allocation(tasks, num_tasks[k], num_cpu);

			if (!unsched) {
				// Calculate WCRT
				WCRT(tasks, num_tasks[k]);
				
				#if(DEBUG_MAIN)
					for (i = 0; i < num_tasks[k]; i++) {
						for (j = 0; j < tasks[i].m+1; j++) {
							printf("WCRT of job %d of task %d is %Lf\n", j, i, tasks[i].WCRT[j]);
						}
					}
				#endif
				
				if (schedulability(tasks, num_tasks[k])) {
					#if(DEBUG_MAIN)
						printf("Taskset %d is schedulable.\n", k);
					#endif
					sched_cnt++;
				} else {
					#if(DEBUG_MAIN)
						printf("Taskset %d is not schedulable.\n", k);
					#endif			
				}
			}

		    // stop timer
	    	gettimeofday(&t2, NULL);

	    	// compute and print the elapsed time in millisec
		    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
		    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
		    
			if (elapsedTime > maxTime) {
		    	maxTime = elapsedTime;
		    }

		    if (k == 0) {
		    	meanTime = elapsedTime;
		    } else {
		    	meanTime = (meanTime*k + elapsedTime)/(k+1);
		    }

		    num_classes = 0;

			// Free memory allocations
			for(i = 0; i < num_tasks[k]; i++) {
				free(data[i]);
				free(tasks[i].priorities);
				free(tasks[i].WCRT);
			}
		}
		
		printf("Util range: %d, Schedulability ratio: %lf \n", u, (double)(sched_cnt/(double)num_tasksets));
		fprintf(fp_out, "%lf \n", (double)(sched_cnt/(double)num_tasksets));
	}
	// Print running time
	printf("Running time (MAX) is %lf ms\n", maxTime);
	printf("Running time (MEAN) is %lf ms\n", meanTime);


    free(data);
    free(tasks);
    fclose(fp);
    fclose(fp_out);
	fclose(fp_tasknum);
 	return 0;
}