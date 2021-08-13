#ifndef _jcls_h_
#define _jcls_h_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// Node constant for schedulability test
#define MAX_CLS_SIZE 3000			// Maximum number of classes of a tree
#define	MAX_SEQUENCE 30				// Size of Node.seq


// DEBUG SETTING
#define DEBUG_MAIN 0
#define DEBUG_WCRT 0
#define DEBUG_SCHED 0
#define DEBUG_PRIORITY 0

int num_tasksets;
unsigned int *num_tasks;
int num_classes;

struct task {
	unsigned int	m;
	unsigned int	K;
	long double		C;
	long double		T;
	unsigned int	w;
	unsigned int	v;
	unsigned int	*priorities;
	long double		*WCRT;
	long double		jitter;
	unsigned int	*cpu;
	double			min_utilization;
	double			utilization;
	double		 	*eta;
};

struct task_rspt {
	// For job-class-level
	long double		job_rspt;
	long double		job_rspt_prev;

	// For task-level analysis
	long double		rspt;
	long double 	rspt_prev;
	int 			sched;			// 0: unschedulable, 1: schedulable
};

struct tree_node {
	unsigned int 	*W;
	unsigned int 	*cls;
	unsigned int 	*miss;
	char 			**seq;

	unsigned int  	W_idx;
	unsigned int  	cls_idx;
	unsigned int  	miss_idx;
	unsigned int  	seq_idx;
};

struct task_ascend {
	long double		period;
	unsigned int 	index;
};

struct priority_descend {
	unsigned int 	priority;
	unsigned int 	index;
};

struct miss_ascend {
	unsigned int	miss_thres;
	unsigned int 	index;
};

struct array_sort_float {
	float value;
	unsigned int index;
};

// init.c
int load_file(char*, long double**);
int load_data(FILE *, long double **, unsigned int);
void initialize_task(struct task*, long double, long double, unsigned int, unsigned int, long double);
void init_node(struct tree_node*, unsigned int, unsigned int); 

// cal_wcrt.c
int assign_hst_priority(struct task*, unsigned int);
int assign_hst_priority_w(struct task*, unsigned int);
int assign_hst_priority_v(struct task*, unsigned int);
int WFD_allocation(struct task *, unsigned int, unsigned int);
int WFD_allocation_U(struct task *, unsigned int, unsigned int);
int Job_allocation(struct task *, unsigned int, unsigned int);

void WCRT_RM(struct task*, struct task_rspt*, unsigned int);
void WCRT(struct task*, unsigned int);
void WCRT_CLASS(struct task*, unsigned int, unsigned int, unsigned int);

// sched.c
int schedulability (struct task*, unsigned int);


#endif 