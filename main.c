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


#define CPSR_T_MASK (1u<<5)

long libcrackmeAddr, libcAddr, selfLibcAddr, libdlAddr, selfLibdlAddr;


int main()
{
	pid_t pid;
	if(lookupPid(TARGET_PROCESS, &pid))
	{
		fprintf(stderr, "[ERROR]Target process \"%s\" matching failed\n", TARGET_PROCESS);	
		return 1;
	}

	printf("[INFO]Target pid found: %d\n", pid);

  pid_t selfPid = getpid();
	if(lookupBaseAddr(LIBCRACKME, &pid, &libcrackmeAddr))
		fprintf(stderr, "[ERROR]Target module \"%s\" locating failed\n", LIBCRACKME);
	if(lookupBaseAddr(LIBCSO, &pid, &libcAddr))
		fprintf(stderr, "[ERROR]Target module \"%s\" locating failed\n", LIBCSO);
  if(lookupBaseAddr(LIBCSO, &selfPid, &selfLibcAddr))
		fprintf(stderr, "[ERROR]Target module \"%s\" of current process locating failed\n", LIBCSO);
	lookupBaseAddr(LIBDLSO, &pid, &libdlAddr);
  lookupBaseAddr(LIBDLSO, &selfPid, &selfLibdlAddr);
  

	printf("[INFO]Base address of \"%s\" found: %lx\n", LIBCRACKME, libcrackmeAddr);
	printf("[INFO]Base address of \"%s\" found: %lx\n", LIBCSO, libcAddr);
	printf("[INFO]Base address of \"%s\" of current process found: %lx\n", LIBCSO, selfLibcAddr);

	if(ptraceInject(&pid))
	{
		fprintf(stderr, "[ERROR]Injection failed");
		return 1;
	}
	return 0;
}
