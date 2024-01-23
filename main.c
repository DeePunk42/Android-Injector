#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<dirent.h>
#include<dlfcn.h>
#include<sys/types.h>
#include<sys/ptrace.h>
#include<sys/wait.h>
#include<sys/user.h>
#include<sys/reg.h>
#include<sys/mman.h>

#include<./include/config.h>
#include<./include/utils.h>
#include<./include/ptraceWrapper.h>

int main()
{
	pid_t pid;
	if(lookupPid(TARGET_PROCESS, &pid))
	{
		fprintf(stderr, "[ERROR]Target process \"%s\" matching failed\n", TARGET_PROCESS);	
		return 1;
	}
	printf("[INFO]Target pid found: %d\n", pid);

	if(ptraceInject(&pid))
	{
		fprintf(stderr, "[ERROR]Injection failed");
		return 1;
	}
	return 0;
}
