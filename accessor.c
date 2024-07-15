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

void print_cpu_ipc(char *line, char **names, int count, long long unsigned t){

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
		
		printf("{ \"t\" : %llu, \"CPU\" : %d, \"Symbol\" : %s, \"Inst\" : %llu, \"Cyc\" : %llu},\n", t, id, names[idx], inst, cyc);

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


	//SET CPUs TO MONITOR
	cpu[0] = 1;
	cpu[1] = 7;
	

	fscanf(fp, "Count: %d\n", &count);

	printf("Monitoring %d functions\n", count);


	names = (char **)malloc(sizeof(char*) * count);
	for(int i = 0; i < count; i++){
		names[i] = (char*)malloc(sizeof(char) * MAX_NAME);
	}

	
	//get function names
	fgets(line, MAX_LINE, fp);
	get_func_names(line, names, count);	


	//Find offset
	int offset = get_fp_offset(fp, cpu);
	int cpu_count = 0;	


	//LOOP
	while(t < 1){
		
		fseek(fp, offset, SEEK_SET);
		
		//read proc file
		while(cpu_count < cpu[1] - cpu[0] + 1 && fgets(line, MAX_LINE, fp) != NULL){
			print_cpu_ipc(line, names, count, t);
			cpu_count++;
		}
		t++;

	}
	

	//deallocate
	for(int i = 0; i < count; i++){
		free(names[i]);
	}
	free(names);


}
