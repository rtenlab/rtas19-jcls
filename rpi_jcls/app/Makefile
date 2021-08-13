CC=gcc
CROSS_COMPILE=arm-linux-gnueabihf-

all:
	$(CROSS_COMPILE)$(CC) -pthread -D_GNU_SOURCE -std=c99 -lm -lrt mod_set_rsv_app.c init_task.c signals.c -o run_sched
clean:
	rm -rf run_sched
