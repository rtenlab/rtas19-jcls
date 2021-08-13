#ifndef _init_task_h
#define _init_task_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include "../../include/linux/reserve.h"

#define MAX(a,b) (((a)>(b))?(a):(b))
#define RT_PRIORITY_MAX 90
#define EXP_TIME 600		// Experiment time (unit: sec)

int num_tasks;
int job_mode;

int compare(const void *, const void *);
void init_tasksets(struct set_task_param *set_var, char **argv);

#endif