#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/bpf.h>


#define NUM_FUNCS 1

#define MAX_NAME_LEN 50
#define CPU_CORES 36

#define FIXED_CTR_INST 0x309U
#define FIXED_CTR_CYC  0x30AU

struct target {

	char fn_name[50];
	uint64_t inst[CPU_CORES];
	uint64_t cyc[CPU_CORES];
	double IPC[CPU_CORES];

};

struct priv_data{

	int id;
	uint64_t tmp_inst;
	uint64_t tmp_cyc;

};

static struct target targets[NUM_FUNCS] = {
		{
			.fn_name = "net_rx_action"
		}
};


static struct kprobe kp[NUM_FUNCS];
static struct kretprobe krp[NUM_FUNCS];

static uint64_t read_msr(uint32_t addr){

        uint64_t ret = 0xff;


        asm volatile( "mov %1, %%ecx\n\t"
                                "rdmsr\n\t"
                                "shl $32, %%rdx\n\t"
                                "or %%rdx, %0"
                                : "=a"(ret)
                                : "r"(addr)
                                : "rdx"
                                );

        //printk("MSR 0x%x = %llx\n", addr, ret);

        return ret;


} 

static void get_counters(uint64_t *values){

	uint64_t cycles, inst;


	//rdmsr for cycles
	cycles = read_msr(FIXED_CTR_CYC);
	

	//Note: instructions count will include instructions for measuring the cycles above. 
	//If this level of accuracy is needed, maybe it is possible to deduct the instructions required 
	//for rdmsr from the instruction count

	//rdmsr for instrtuctions
	inst = read_msr(FIXED_CTR_INST);


	//write to proc file
//	printk("Cyc:%llu\tInst:%llu\n", cycles, inst);

	values[0] = cycles;
	values[1] = inst;

}

static int handler_pre (struct kretprobe_instance *ri, struct pt_regs *regs){

	uint64_t values[2];
	struct priv_data *tmps = (struct priv_data*)ri->data;
	int cpu_id;

	//get cpu number
	cpu_id = smp_processor_id();
	tmps->id = cpu_id;


	//get counters
	get_counters(values);

	//safe them for later use
	tmps->tmp_cyc = values[0];
	tmps->tmp_inst = values[1];

	return 0;
}


static int handler_ret (struct kretprobe_instance *ri, struct pt_regs *regs){

	uint64_t values[2];
	int cpu_id = smp_processor_id();
	struct priv_data *tmps = (struct priv_data*)ri->data;

	//printk("CPU %d: ", cpu_id);

	//get counters
	get_counters(values);

	if(tmps->id != cpu_id){

		printk("Different CPU (%d != %d)\n", tmps->id, cpu_id);
		return 0;
	
	}

	//calculate difference
	values[0] -= tmps->tmp_cyc;	
	values[1] -= tmps->tmp_inst;	

	//update proc
	printk("CPU %d: Cyc:%llu\tInst:%llu\n", cpu_id, values[0], values[1]);
	
	return 0;

}

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

	int i, ret;

	for(i = 0; i < NUM_FUNCS; i++){

		//kprobe
		kp[i].symbol_name = targets[i].fn_name;
		kp[i].offset = 0;

		//kretprobe
		krp[i].kp = kp[i];
		krp[i].handler = handler_ret;
		krp[i].entry_handler = handler_pre;
		krp[i].maxactive = CPU_CORES;
		krp[i].data_size = sizeof(struct priv_data);

		ret = register_kretprobe(&krp[i]);
		if(ret < 0)	return ret;
		printk("Inserting kretprobe for symbol %s\n", targets[i].fn_name);
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

	int i;

	//unregister kprobes
	for(i = 0; i < NUM_FUNCS; i++){
		unregister_kretprobe(&krp[i]);
	}
	
	//deactivate hw counters

}

module_init(init_ipc_mod)
module_exit(exit_ipc_mod)
MODULE_LICENSE("GPL");


