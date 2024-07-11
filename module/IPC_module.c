#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/bpf.h>


#define NUM_FUNCS 3

static char *funcs[] = {
		"net_rx_action",
		"napi_poll",
		"netif_skb_receive"
};


//[TODO]
static int check_availability(void){


	return 0;

}

//[TODO]
static int activate_hw_count(void){

	
	

	
	return 0;

}

//[TODO]
static int register_kp(void){

	int i;

	for(i = 0; i < NUM_FUNCS; i++){
		printk("Inserting kprobe for symbol %s\n", funcs[i]);
	}

	return 0;

}

static int __init init_ipc_mod(void){
	int ret;

	//sanity checks	
	ret = check_availability();
	if(ret < 0){
		printk(KERN_ERR "IPC_Module cannot be loaded on this system (%d)\n", ret);
		return ret;
	}

	//activate hw counters
	ret = activate_hw_count();
	if(ret < 0){
		printk(KERN_ERR "IPC_Module could not activate Hardware Counters (%d)\n", ret);
		return ret;
	}


	//setup all kprobes
	ret = register_kp();
	if(ret < 0){
		printk(KERN_ERR "IPC_Module failed to register kprobes (%d)\n", ret);
		return ret;
	}

	return 0;



}

static void __exit exit_ipc_mod(void){

	//unregister kprobes
	
	//deactivate hw counters

}

module_init(init_ipc_mod)
module_exit(exit_ipc_mod)
MODULE_LICENSE("GPL");


