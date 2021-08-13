#CONFIG_MODULE_SIG=n

#obj-m := mod_mk.o
#mod_mk-objs := mod_set_rsv.o rsv_kthread.o
obj-m += run_mk.o
run_mk-objs += mod_mk.o mk_signals.o mk_monitor.o mk_syscalls.o mk_rsv_kthread.o mk_sched.o

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ../.. M=$(PWD) modules
clean:
	make -C ../.. M=$(PWD) clean
