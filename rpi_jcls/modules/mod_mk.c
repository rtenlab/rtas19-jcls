#include "mk.h"

void **p_sys_call_table;	
asmlinkage long (*original_flex_bridge1)(void*); // SET_DEV
asmlinkage long (*original_flex_bridge2)(void*); // CANCEL_DEV
asmlinkage long (*original_flex_bridge3)(void*); // WAIT_UNTIL_NEXT_PERIOD
asmlinkage long (*original_flex_bridge4)(void*, void*, void*, void*, void*); // COPY_DATA_TO_USER

/* Since kernel 4.1-rc1, set_memory_rX() functions are no longer exported,
 so we need this hack to get them. */
static int prepare_set_memory_rx_funcs(void);
static int (*do_set_memory_ro)(unsigned long addr, int numpages) = NULL;
static int (*do_set_memory_rw)(unsigned long addr, int numpages) = NULL;

static int __init mod_set_rsv(void) {
	int replace_set = prepare_set_memory_rx_funcs();
	
	p_sys_call_table = (void *) kallsyms_lookup_name("sys_call_table");
	do_set_memory_rw((unsigned long)p_sys_call_table, 1);

	original_flex_bridge1 = p_sys_call_table[__NR_flex_bridge6];
	original_flex_bridge2 = p_sys_call_table[__NR_flex_bridge2];
	original_flex_bridge3 = p_sys_call_table[__NR_flex_bridge3];
	original_flex_bridge4 = p_sys_call_table[__NR_flex_bridge4];
	p_sys_call_table[__NR_flex_bridge6] = set_rsv;
	p_sys_call_table[__NR_flex_bridge2] = cancel_rsv;
	p_sys_call_table[__NR_flex_bridge3] = wait_until_next_period;
	p_sys_call_table[__NR_flex_bridge4] = copy_data_to_user;

	do_set_memory_ro((unsigned long)p_sys_call_table, 1);

	ktask_reserve_init();

	printk(KERN_INFO "MOD_SET_RSV : Successfully loaded.\n");
	return 0;
}

static void __exit mod_set_rsv_exit(void) {
	if (p_sys_call_table[__NR_flex_bridge6] != set_rsv) {
		printk(KERN_ALERT "Flex bridge system call is used elsewhere.\n");
	}

	// Make sys_call_table writable
	do_set_memory_rw((unsigned long)p_sys_call_table, 1);

	p_sys_call_table[__NR_flex_bridge6] = original_flex_bridge1;	
	p_sys_call_table[__NR_flex_bridge2] = original_flex_bridge2;	
	p_sys_call_table[__NR_flex_bridge3] = original_flex_bridge3;	
	p_sys_call_table[__NR_flex_bridge4] = original_flex_bridge4;

	do_set_memory_ro((unsigned long)p_sys_call_table, 1);
	
	ktask_reserve_cleanup();

	printk(KERN_INFO "MOD_SET_RSV : Module unloaded.\n");
}

module_init(mod_set_rsv);
module_exit(mod_set_rsv_exit);

static int prepare_set_memory_rx_funcs(void) {
	do_set_memory_ro = (void *)kallsyms_lookup_name("set_memory_ro");
	if (do_set_memory_ro == NULL) {
		return -EINVAL;
	}

	do_set_memory_rw = (void *)kallsyms_lookup_name("set_memory_rw");
	if (do_set_memory_rw == NULL) {
		return -EINVAL;
	}

	return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HYUNJONG");
MODULE_DESCRIPTION("This module is for resource reservation of tasks.");

