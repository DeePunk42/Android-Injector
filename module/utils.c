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

void Debug_reginfo(pid_t *pid)
{
  struct pt_regs regs;
  if(ptrace(PTRACE_GETREGS, *pid, NULL, &regs)<0)
  {
    fprintf(stderr, "[ERROR]debug failed\n");
    return;
  }
  printf("[DEBUG]debug info:\n");
  for(int i=0;i<4;i++)  printf("arg[%d]:%lx\n", i, regs.uregs[i]);
  printf("pc:%lx\n", regs.ARM_pc);
  printf("lr:%lx\n", regs.ARM_lr);
}

int lookupPid(char *processName, pid_t *pid)
{
	FILE *fp;
	char buf[0x100], user[0x30];
	if(!(fp = popen(PS_A, "r")))
		return -1;
	while(fgets(buf, sizeof(buf), fp))
		if(strstr(buf, processName))
		{
			sscanf(buf, "%s%d", user, pid); 
			return 0;
		}
	return -1;
}

int lookupModuleAddr(char *moduleName, pid_t *pid, unsigned long *baseAddr)
{
	FILE *fp;
	char cmd[0x20], buf[0x180], user[0x30];
	sprintf(cmd, "cat /proc/%d/maps", *pid);
	if(!(fp = popen(cmd, "r")))
		return -1;
	while(fgets(buf, sizeof(buf), fp))
		if(strstr(buf, moduleName))
		{
			sscanf(buf, "%lx", baseAddr); 
			return 0;
		}
	return -1;
}

unsigned long lookupFuncAddr(void *funcName, char *moduleName, pid_t *pid)
{
	pid_t selfPid = getpid();
	unsigned long base, selfLibcAddr;
	if(lookupModuleAddr(moduleName, pid, &base) < 0){
		fprintf(stderr, "[ERROR]getting base address of %s failed", moduleName);
		return NULL;
	}
	if(lookupBaseAddr(LIBCSO, &selfPid, &selfLibcAddr) < 0){
		fprintf(stderr, "[ERROR]getting libc failed", moduleName);
		return NULL;
	}

	return (unsigned long)funcName - selfLibcAddr + base;
}