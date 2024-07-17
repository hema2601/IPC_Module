#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define MAX_LINE 256
#define MAX_NAME 30



void get_func_names(char *line, char **names, int count){


	char *token = strtok(line, " ");
	int idx = 0;
	char *tab;

	while(idx < count && token != NULL){
		tab = strchr(token, '\t');
		//printf("Token: %s %ld\n", token, tab-token);
		
		//tab = strchr(token, '\t');
		memcpy(names[idx], token, MAX_NAME);
		names[idx][tab-token] = '\0';
		//printf("%s|\n", names[idx]);
		token = strtok(NULL, " ");
		idx++;
	}

}

void print_cpu_ipc(char *line, char **names, int count, long long unsigned t, long long unsigned ***prev, int *cpu){

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

			printf("{ \"t\" : %llu, \"CPU\" : %d, \"Symbol\" : \"%s\", \"Inst\" : %llu, \"Cyc\" : %llu},\n", t, id, names[idx], inst, cyc);
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


int main(int argc, char *argv[]){

	//open proc file
	FILE *fp = fopen("/proc/IPC_module", "r");

	if(!fp){
		printf("Failed to open /proc/IPC_module. Is the module loaded?");
		exit(1);
	}

	long long unsigned t = 0;
	char line[MAX_LINE];
	int count;
	char ** names;
	int cpu[2];
	long long unsigned ***prev;

	//SET CPUs TO MONITOR
	cpu[0] = 0;
	cpu[1] = 5;
	
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


	printf("[");

	while(t < 100000){
		
		fseek(fp, offset, SEEK_SET);

		//read proc file
		while(cpu_count < cpu[1] - cpu[0] + 1 && fgets(line, MAX_LINE, fp) != NULL){
			print_cpu_ipc(line, names, count, t, prev, cpu);
			cpu_count++;
		}
		t++;
		cpu_count = 0;

		//Close and reopen proc file to regenerate it
		fclose(fp);
		fp = fopen("/proc/IPC_module", "r");

	}
	
	printf("]");

	//deallocate
	for(int i = 0; i < count; i++){
		free(names[i]);
		for(int j = 0; j < cpu[1] - cpu[0] + 1; j++)
			free(prev[i][j]);
		free(prev[i]);
	}
	free(names);
	free(prev);


}
