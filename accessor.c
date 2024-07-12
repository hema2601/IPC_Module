#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>






int main(int argc, char *argv[]){

	//open proc file
	FILE *fp = fopen("/proc/IPC_module", "r");

	long long unsigned t = 0;

	//LOOP
	while(t < 10000){
		
		fseek(fp, 0, SEEK_SET);
		
		char line[256];

		fgets(line, 256, fp);
		fgets(line, 256, fp);
	
		//read proc file
		while(fgets(line, 256, fp) != NULL){
			int id;
			long long unsigned int inst, cyc;
			sscanf(line, "CPU %d:%llu\t\t%llu\n", &id, &inst, &cyc);

			printf("{ \"t\" : %llu, \"CPU\" : %d, \"Inst\" : %llu, \"Cyc\" : %llu},\n", t, id, inst, cyc);
			
		}
		t++;
		//parse proc file

		//output JSON
		//printf("%s\n", line);

		//sleep(1);

	}


}
