#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ptrace.h>

#include"./inc/config.h"
#include"./inc/utils.h"
#include"./inc/ptraceWrapper.h"

int main(){
	pid_t pid;
	if(lookupPid(TARGET_PROCESS, &pid)){
		fprintf(stderr, "[ERROR]Target process \"%s\" matching failed\n", TARGET_PROCESS);	
		return 1;
	}
	printf("[INFO]Target pid found: %d\n", pid);

	if(ptraceInject(&pid)){
		fprintf(stderr, "[ERROR]Injection failed");
		return 1;
	}

	return 0;
}
