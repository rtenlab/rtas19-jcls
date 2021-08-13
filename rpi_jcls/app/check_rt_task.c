#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// System call number * Do not change!
#define CHECK_RT_TASK 291

// Main function
int main(int argc, char *argv[]) {
	int *dummy;

	// System call for reservation	
	int re = syscall(CHECK_RT_TASK, (void *)dummy);
	printf("CHECK_RT_TASK result is %d\n", re);
	if (re < 0) {
		printf("Syscall CHECK_RT_TASK has an error.\n");
	} 
	return 0;
}