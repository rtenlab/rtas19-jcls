//#include <linux/kernel.h>
//#include <linux/init.h>
//#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
//#include <asm/syscall.h>
//#include <linux/mutex.h>
//#include <linux/uaccess.h>
//#include <linux/pid.h>
//#include <linux/hrtimer.h>
//#include <uapi/linux/sched/types.h>

SYSCALL_DEFINE2(flex_bridge1, void*, arg, struct task_stats*, stats) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 1 SYSCALL is called.\n");
	return (long)arg;
}


SYSCALL_DEFINE1(flex_bridge2, 
		void*, arg) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 2 SYSCALL is called.\n");
	return (long)arg;
}

SYSCALL_DEFINE1(flex_bridge3, 
		void*, arg) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 3 SYSCALL is called.\n");
	return (long)arg;
}

SYSCALL_DEFINE5(flex_bridge4, 
		void*, arg1, void*, arg2, void*, arg3, void*, arg4, void*, arg5) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 4 SYSCALL is called.\n");
	return (long)arg1;
}

SYSCALL_DEFINE1(flex_bridge5, 
		void*, arg) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 5 SYSCALL is called.\n");
	return (long)arg;
}

SYSCALL_DEFINE1(flex_bridge6, 
		void*, arg) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 6 SYSCALL is called.\n");
	return (long)arg;
}

SYSCALL_DEFINE1(flex_bridge7, 
		void*, arg) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 7 SYSCALL is called.\n");
	return (long)arg;
}

SYSCALL_DEFINE1(flex_bridge8, 
		void*, arg) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 8 SYSCALL is called.\n");
	return (long)arg;
}

SYSCALL_DEFINE1(flex_bridge9, 
		void*, arg) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 9 SYSCALL is called.\n");
	return (long)arg;
}

SYSCALL_DEFINE1(flex_bridge10, 
		void*, arg) {
	printk(KERN_INFO "Syscall is called: Flexible bridge 10 SYSCALL is called.\n");
	return (long)arg;
}

