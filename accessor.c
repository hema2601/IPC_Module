#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_LINE 256
#define MAX_NAME 30
#define MAX_EXP_LEN 256

#include <argp.h>
#include <stdbool.h>

const char *argp_program_version = "IPC_Module Accessor 0.0.1";
const char *argp_program_bug_address = "<your@email.address>";
static char doc[] = "Your program description.";
static char args_doc[] = "experiment_name";
static struct argp_option options[] = { 
    { "cores", 'c', "CORES", 0, "Define a single core or a range of cores to capture."},
    { "time", 't', "TIME", 0, "Set how often the proc file is read consecutively."},
    { 0 } 
};

struct arguments {
	long long unsigned time;    
	int cores[2];
	char exp_name[MAX_EXP_LEN];
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
	char *token;
    switch (key) {
    case 'c':
		//[TODO] Check that value is actually integer and within valid cpu range
		token = strtok(arg, "-");
		
		if(token == NULL)
			argp_usage( state );

		arguments->cores[0] = atoi(token);

		token = strtok(NULL, "-");

 		arguments->cores[1] = (token == NULL) ? arguments->cores[0]
											  : atoi(token);

		break;
    case 't': arguments->time = atoi(arg); break;
    case ARGP_KEY_ARG: 
		memcpy(arguments->exp_name, arg, MAX_EXP_LEN);
		arguments->exp_name[MAX_EXP_LEN-1] = '\0';
		return 0;
    case ARGP_KEY_END:
		if(state->arg_num < 1){
			argp_usage (state);
		}
		break;
	default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };


void get_func_names(char *line, char **names, int count){


	char *token = strtok(line, " ");
	int idx = 0;
	char *tab;

	while(idx < count && token != NULL){
		tab = strchr(token, '\t');
		memcpy(names[idx], token, MAX_NAME);
		names[idx][tab-token] = '\0';
		token = strtok(NULL, " ");
		idx++;
	}

}

void print_cpu_ipc(char *line, char **names, int count, long long unsigned t, long long unsigned ***prev, int *cpu, FILE *json_fp){

	int id;
	long long unsigned int inst, cyc;
	sscanf(line, "CPU %d", &id);

	char *token = strtok(line, "\t");

	int idx = 0;

	while(idx < count && token != NULL){
		
		//Instructions
		token = strtok(NULL, "\t");
		inst = atoi(token);
		

		//Cycles
		token = strtok(NULL, "\t");
		cyc = atoi(token);
		
		if(inst != prev[idx][id - cpu[0]][0] || cyc != prev[idx][id-cpu[0]][1]){

			fprintf(json_fp, "{ \"t\" : %llu, \"CPU\" : %d, \"Symbol\" : \"%s\", \"Inst\" : %llu, \"Cyc\" : %llu},\n", t, id, names[idx], inst, cyc);
			prev[idx][id-cpu[0]][0] = inst;
			prev[idx][id-cpu[0]][1] = cyc;
		}
		idx++;
	}

}

int get_fp_offset(FILE *fp, int *cpu){
	FILE *tmp = fp;
	int offset = 0;
	char line[MAX_LINE];

	fseek(tmp, 0, SEEK_SET);
		
	fgets(line, MAX_LINE, tmp);
	offset += strlen(line);
	fgets(line, MAX_LINE, tmp);
	offset += strlen(line);
	fgets(line, MAX_LINE, tmp);
	offset += strlen(line);

	for(int i = 0; i < cpu[0]; i++){
		fgets(line, MAX_LINE, tmp);
		offset += strlen(line);
	}


	return offset;
}

int createFiles(struct arguments *args, FILE **json){

	int ret;
	char meta_name[MAX_EXP_LEN + 10], data_name[MAX_EXP_LEN + 10];
	char *path = "./data/";
	char tmp[MAX_EXP_LEN + 10 + 7 + 30];

	//Create Directory

	memcpy(tmp, path, 8);
	strncat(tmp, args->exp_name, MAX_EXP_LEN);

	ret = mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
	
	if(ret < 0){
		//[TODO] Add option to overwrite an existing directory
		perror("Could not create directory");
		return ret;
	}

	//Create Metadata File

	
	memcpy(tmp, path, 8);
	strncat(tmp, args->exp_name, MAX_EXP_LEN);
	strncat(tmp, "/meta.json", 11);

	printf("Creating meta file called: %s\n", tmp);

	FILE *fp = fopen(tmp, "w");

	fprintf(fp, "[{ \"Name\": \"%s\", \"Time\" : %llu, \"CPUs\" : [%d, %d]}]", args->exp_name, args->time, args->cores[0], args->cores[1]);
	
	fclose(fp);


	//Create Json File
	memcpy(tmp, path, 8);
	strncat(tmp, args->exp_name, MAX_EXP_LEN);
	strncat(tmp, "/data.json", 11);

	printf("Creating data file called: %s\n", tmp);

	*json = fopen(tmp, "w");

	return 0;

}


int main(int argc, char *argv[]){

	//Options
	struct arguments arguments;

	//defaults
	arguments.time = 1000;
	arguments.cores[0] = 0;
	arguments.cores[1] = 5;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);
	
	printf("Experiment Name: %s\n", arguments.exp_name);
	printf("Running for %llu cycles\n", arguments.time);
	printf("Monitoring Cores %d to %d\n", arguments.cores[0], arguments.cores[1]);

	//open proc file
	FILE *fp = fopen("/proc/IPC_module", "r");

	if(!fp){
		printf("Failed to open /proc/IPC_module. Is the module loaded?");
		exit(1);
	}


	//Create Experiment Folder

	FILE *json_fp;

	if(createFiles(&arguments, &json_fp) < 0){
		printf("Could not create all files\n");
		exit(2);
	}
	



	long long unsigned t = 0;
	char line[MAX_LINE];
	int count;
	char ** names;
	int cpu[2];
	long long unsigned ***prev;

	//SET CPUs TO MONITOR
	cpu[0] = arguments.cores[0];
	cpu[1] = arguments.cores[1];
	
	//Get number of functions
	fscanf(fp, "Count: %d\n", &count);

	//Allocate relevant data structures
	names = (char **)malloc(sizeof(char*) * count);
	prev = (long long unsigned ***)malloc(sizeof(long long unsigned**) * count);
	for(int i = 0; i < count; i++){
		names[i] = (char*)malloc(sizeof(char) * MAX_NAME);
		prev[i] = (long long unsigned **)calloc(sizeof(long long unsigned*), cpu[1]-cpu[0]+1);
		for(int j = 0; j < cpu[1]-cpu[0]+1; j++){
			prev[i][j] = (long long unsigned *)calloc(sizeof(long long unsigned), 2);
		}
	}
	
	//get function names
	fgets(line, MAX_LINE, fp);
	get_func_names(line, names, count);	


	//Find offset
	int offset = get_fp_offset(fp, cpu);
	int cpu_count = 0;	


	fprintf(json_fp, "[");

	while(t < arguments.time){
		
		fseek(fp, offset, SEEK_SET);

		//read proc file
		while(cpu_count < cpu[1] - cpu[0] + 1 && fgets(line, MAX_LINE, fp) != NULL){
			print_cpu_ipc(line, names, count, t, prev, cpu, json_fp);
			cpu_count++;
		}
		t++;
		cpu_count = 0;

		//Close and reopen proc file to regenerate it
		fclose(fp);
		fp = fopen("/proc/IPC_module", "r");

	}
	
	fprintf(json_fp, "]");

	//deallocate
	for(int i = 0; i < count; i++){
		free(names[i]);
		for(int j = 0; j < cpu[1] - cpu[0] + 1; j++)
			free(prev[i][j]);
		free(prev[i]);
	}
	free(names);
	free(prev);

	fclose(json_fp);


}
