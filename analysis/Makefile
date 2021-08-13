CC=gcc
#CROSS_COMPILE=arm-linux-gnueabihf-

all:
	$(CC) -pthread -D_GNU_SOURCE -std=c99 main.c init.c wcrt.c sched.c -lm -o check_sched
clean:
	rm -rf check_sched
