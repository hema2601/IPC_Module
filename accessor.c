#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>






int main(int argc, char *argv[]){

	//open proc file
	FILE *fp = fopen("/proc/IPC_module", "r");

	//LOOP
	while(1){
		
		char line[256];

	
		//read proc file
		fgets(line, 256, fp);

		//parse proc file

		//output JSON
		printf("%s\n", line);

		sleep(1);

	}


}
