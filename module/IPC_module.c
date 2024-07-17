#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/bpf.h>
#include<linux/proc_fs.h>
#include<linux/seq_file.h>

#define NUM_FUNCS 3

#define MAX_NAME_LEN 30
#define CPU_CORES 36

#define FIXED_CTR_INST 0x309U
#define FIXED_CTR_CYC  0x30AU

struct target {

	char fn_name[50];
	uint64_t inst[CPU_CORES];
	uint64_t cyc[CPU_CORES];

};

struct priv_data{

	int id;
	uint64_t tmp_inst;
	uint64_t tmp_cyc;

};

static struct target targets[NUM_FUNCS] = {
		{
			.fn_name = "net_rx_action"
		},
		{
			.fn_name = "__netif_receive_skb_core"
		},
		{
			.fn_name = "napi_gro_receive"
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
	int64_t idx = (ri->rph->rp - krp); 

	//get counters
	get_counters(values);

	if(tmps->id != cpu_id){

		printk("Different CPU (%d != %d)\n", tmps->id, cpu_id);
		return 0;
	
	}
 
	//calculate difference + Overflow Consideration
	if(likely(values[0] > tmps->tmp_cyc))	values[0] -= tmps->tmp_cyc;
	else									values[0] += 0xFFFFFFFFFFFFFFFFU - tmps->tmp_cyc;							
	if(likely(values[1] > tmps->tmp_inst))	values[1] -= tmps->tmp_inst;	
	else									values[1] += 0xFFFFFFFFFFFFFFFFU - tmps->tmp_inst;


	//Update Values	
	targets[idx].cyc[cpu_id] = values[0];
	targets[idx].inst[cpu_id] = values[1];
	
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

static int my_proc_show(struct seq_file *m,void *v){

        int i, j;

		

        seq_printf(m, "Count: %d\n", NUM_FUNCS );

        for(j = 0; j < NUM_FUNCS; j++)
                seq_printf(m, "%30s\t\t", targets[j].fn_name );

        seq_printf(m, "\n");

        seq_printf(m, "CPU\t" );
        for(j = 0; j < NUM_FUNCS; j++)
                seq_printf(m,"Instructions\tCycles\t\t\t");
        seq_printf(m, "\n");

        for(i = 0; i < CPU_CORES; i++){
                seq_printf(m, "CPU %d:\t", i);

                for(j = 0; j < NUM_FUNCS; j++)
                        seq_printf(m, "%llu\t\t%llu\t\t\t", targets[j].inst[i], targets[j].cyc[i]);
                seq_printf(m, "\n");

        }

        return 0;
}




static int my_proc_open(struct inode *inode,struct file *file){
	return single_open(file,my_proc_show,NULL);
}

static struct proc_ops my_fops={
	//.owner = THIS_MODULE,
	.proc_open = my_proc_open,
	.proc_release = single_release,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek
};


static int create_proc(void){

	struct proc_dir_entry *entry;
	
	entry = proc_create("IPC_module",0777,NULL,&my_fops);
	if(!entry){
		return -1;	
	}else{
		printk(KERN_INFO "create proc file successfully\n");
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

	//create proc
	ret = create_proc();
	if(ret < 0){
		printk(KERN_ERR "IPC_Module could not setup proc (%d)\n", ret);
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

	//remove proc
	remove_proc_entry("IPC_module", NULL);
	
}

module_init(init_ipc_mod)
module_exit(exit_ipc_mod)
MODULE_LICENSE("GPL");


