#include "mk.h"

struct kwork_queue* cpu_kwork_queue;
extern struct task_struct* worker;

/* Create a kernel thread for update_priority in HRTIMER ISR */
void ktask_update_task_priority(int type, struct task_struct *task, int cpunum, int prio) {
	int dest_cpu = 0;
	
	unsigned long flags;
	raw_spin_lock_irqsave(&per_cpu(cpu_kwork_queue, dest_cpu)->lock, flags);

	ktime_t start, finish;
	start = ktime_get();
	
	struct sched_param par;

	// Change task's priority
	par.sched_priority = prio;
	if (task->rt_priority != par.sched_priority) {
		printk(KERN_INFO "PREPARE_TASK_FOR_CPURSV: Task %d priority is different. before %d, after %d.\n", task->pid, task->rt_priority, prio);
		if (sched_setscheduler_nocheck(task, SCHED_FIFO, &par) < 0) {
			printk(KERN_INFO "PREPARE_TASK_FOR_CPURSV: Cannot change task's priority.\n");
		}
	} else {
		printk(KERN_INFO "PREPARE_TASK_FOR_CPURSV: Task %d priority is same. before %d, after %d.\n", task->pid, task->rt_priority, prio);
	}

	finish = ktime_get();
	timediff_for_ovhd(finish, start, task, 3);

	raw_spin_unlock_irqrestore(&per_cpu(cpu_kwork_queue, dest_cpu)->lock, flags);
}

int push_to_kwork_queue(int dest_cpu, int type, void* arg1, void* arg2, void* arg3) {
	unsigned long flags;
	int ret = -1;
	struct kwork_queue *queue;

	raw_spin_lock_irqsave(&per_cpu(cpu_kwork_queue, dest_cpu)->lock, flags);

	queue = per_cpu(cpu_kwork_queue, dest_cpu);
	if (queue->cur_size >= KWORK_MAX) {
		printk(KERN_INFO "push_to_workqueue: queue is full.\n");
		goto error;
	}

	queue->work[queue->cur_pos].type = type;
	queue->work[queue->cur_pos].args[0] = arg1;
	queue->work[queue->cur_pos].args[1] = arg2;
	queue->work[queue->cur_pos].args[2] = arg3;

	queue->cur_size++;
	queue->cur_pos = (queue->cur_pos + 1) % KWORK_MAX;
	ret = 0;

error:
	raw_spin_unlock_irqrestore(&per_cpu(cpu_kwork_queue, dest_cpu)->lock, flags);
	return ret;
}

static int pop_from_kwork_queue(int dest_cpu, struct kwork_info *output) {
	unsigned long flags;
	int index, ret = -1;
	struct kwork_queue *queue;

	raw_spin_lock_irqsave(&per_cpu(cpu_kwork_queue, dest_cpu)->lock, flags);

	queue = per_cpu(cpu_kwork_queue, dest_cpu);
	if (queue->cur_size <= 0) goto error;

	index = ((queue->cur_pos + KWORK_MAX) - queue->cur_size)%KWORK_MAX;
	*output = queue->work[index];

	queue->cur_size--;
	ret = 0;

error:
	raw_spin_unlock_irqrestore(&per_cpu(cpu_kwork_queue, dest_cpu)->lock, flags);
	return ret;
}

static int worker_thread(void *data) {
	int thread_id = (long)data;
	#if(MK_DEBUG_MSG)
	printk(KERN_INFO "Worker_thread : %s (%d)\n", current->comm, thread_id);
	#endif

	while (!kthread_should_stop()) {
		struct kwork_info work = {0,};
		#if(MK_DEBUG_MSG)
		printk(KERN_INFO "kthread should stop while loop.\n");
		#endif

		while (pop_from_kwork_queue(thread_id, &work) == 0) {
			ktask_update_task_priority(work.type, work.args[0], (long)work.args[1], (long)work.args[2]);
			#if(MK_DEBUG_MSG)
			printk(KERN_INFO "Execute worker_thread.\n");
			#endif
		}

		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
	}
	return 0;
}

void ktask_reserve_init(void) {
	int cpunum = 0;
	struct sched_param par;

	par.sched_priority = 99;

	// Worker thread
	per_cpu(cpu_kwork_queue, cpunum) = kmalloc(sizeof(struct kwork_queue), GFP_ATOMIC);
	memset(per_cpu(cpu_kwork_queue, cpunum), 0, sizeof(struct kwork_queue));

	per_cpu(worker, cpunum) = kthread_create(&worker_thread, (void*)(long)cpunum, "worker/0");

	if (IS_ERR(per_cpu(worker, cpunum))) {
		printk(KERN_INFO "worker/0: ERROR\n");
	}

	sched_setscheduler(per_cpu(worker, cpunum), SCHED_FIFO, &par);
	wake_up_process(per_cpu(worker, cpunum));
}

void ktask_reserve_cleanup(void) {
	int cpunum = 0;
	kfree(per_cpu(cpu_kwork_queue, cpunum));
	kthread_stop(per_cpu(worker, cpunum));
}
