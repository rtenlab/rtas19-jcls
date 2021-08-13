#ifndef _signals_h
#define _signals_h

#include "init_task.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>

#define UPDATE_PRIORITY 401

//DEFINE_SPINLOCK(sched_pri_lock);
jmp_buf *restart_p;
int *jmp_buf_task_id;
int *cancel_flag;
int *cancel_task_id;
static pthread_mutex_t sig_mutex;

void ISR_SIG_cancel_task(int sig_num, siginfo_t *info, void *unused);
void ISR_SIG_jump_task(int sig_num, siginfo_t *info, void *unused);


#endif