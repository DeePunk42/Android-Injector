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

#define PS_A "ps -A"
#define PS_AT "ps -AT"
#define TARGET_PROCESS "crackme1"
#define TARGET_WCHAN "compat_SyS_nanosleep"
#define LIBCRACKME "libcrackme1.so"
#define LIBCSO "libc.so"
#define LIBDLSO "libdl.so"
#define LIBCPATH "/apex/com.android.runtime/lib/bionic/libc.so"
//#define HIJACKLIBCPATH "/data/app/com.example.crackme1-3yTR1T6Or9SL_kDjCEEfYg==/lib/arm/evil.so"
#define HIJACKLIBCPATH "/data/local/tmp/evil.so"
#define CPSR_T_MASK (1u<<5)

long libcrackmeAddr, libcAddr, selfLibcAddr, libdlAddr, selfLibdlAddr;

void debug(pid_t *pid)
{
  struct pt_regs regs;
  if(ptrace(PTRACE_GETREGS, *pid, NULL, &regs)<0)
  {
    printf("debug failed\n");
    return;
  }
  printf("debug info:\n");
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

int lookupBaseAddr(char *moduleName, pid_t *pid, long *baseAddr)
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



int ptraceCall(pid_t *pid, long target, long *args, long argsNum, struct pt_regs *regs)
{
  if(argsNum > 4){
    regs->ARM_sp -= (argsNum - 4) * sizeof(long);
    for(int i = 0; i < argsNum - 4; i++)
      ptrace(PTRACE_POKETEXT, *pid, (long)regs->ARM_sp + i * sizeof(long), args[4 + i]); 
  }
  for(int i = 0; i < 4; i++)  regs->uregs[i] = args[i];

  regs->ARM_pc = target;
  if(regs->ARM_pc & 1)
  {
    regs->ARM_pc &= (~1u);
    regs->ARM_cpsr |= CPSR_T_MASK;
  }
  else
  {
    regs->ARM_cpsr &= ~CPSR_T_MASK;
  }

  regs->ARM_lr = 0;
  if(ptrace(PTRACE_SETREGS, *pid, NULL, regs) < 0)
  {
    fprintf(stderr, "[ERROR]Set registers failed\n");
    return -1;
  }
  debug(pid);
  int stat = 0;
  while(stat != 0xb7f)
  {
    if(ptrace(PTRACE_CONT, *pid, NULL, NULL) < 0)
    {
      fprintf(stderr, "[ERROR]Continue failed\n");
      return -1;
    }
    waitpid(*pid, &stat, WUNTRACED);
    printf("status: %x\n", stat);
  }

  if(ptrace(PTRACE_GETREGS, *pid, NULL, regs) < 0)
  {
    fprintf(stderr, "[ERROR]Get return value failed\n");
    return -1;
  }
  return 0;
}

int ptraceInject(pid_t *pid)
{	
  unsigned long dlopenAddr =  (unsigned long)(void *)dlopen - selfLibdlAddr + libdlAddr;
  long mmapAddr = (long)(void *)mmap - selfLibcAddr + libcAddr;
	long freeAddr =  (long)(void *)free - selfLibcAddr + libcAddr;
	printf("[INFO]Functions address got: %lx, %lx\n", dlopenAddr, mmapAddr);
 
	if(ptrace(PTRACE_ATTACH, *pid, NULL, NULL) < 0)
  {
    fprintf(stderr, "[ERROR]Failed to attach target process\n");
  }
  waitpid(*pid, NULL, WUNTRACED);

  struct pt_regs savedRegs, regs;
  memset(&savedRegs, 0, sizeof(struct user_regs));
  memset(&regs, 0, sizeof(struct user_regs));

  if(ptrace(PTRACE_GETREGS, *pid, NULL, &savedRegs) < 0)
  {
    fprintf(stderr, "[ERROR]Failed to Get registers information\n");
    return -1;
  }
  memcpy(&regs, &savedRegs, sizeof(struct pt_regs));

/* call mmap*/
  long args[0x10], ret;
  args[0] = 0;
  args[1] = 0x100;
  args[2] = PROT_READ|PROT_WRITE|PROT_EXEC;
  args[3] = MAP_ANONYMOUS|MAP_PRIVATE;
  args[4] = 0;
  args[5] = 0;
  
  if(ptraceCall(pid, mmapAddr, args, 6, &regs) < 0)
  {
    fprintf(stderr, "[ERROR]Failed to call mmap\n");
    return -1;
  }
  ret = regs.ARM_r0;
  printf("[INFO]Mmap successfully at %lx\n", ret);

/* write lib path */

  char hijack[] = HIJACKLIBCPATH;
  int i=0;
  for(int i = 0; i < ((strlen(hijack)+1) / sizeof(long)) + 1; i++)
    if(ptrace(PTRACE_POKETEXT, *pid, ret + i*sizeof(long), *(long *)(hijack + i*sizeof(long))) < 0)
    {
      fprintf(stderr, "[ERROR]Failed to write libc path to memory\n");
      return -1;
    }
  long chr;
  char buf[4];
  printf("[INFO]write successfully:\n");
  for(int i = 0; i < (strlen(hijack) / sizeof(long)) + 1; i++)
  {
    chr = ptrace(PTRACE_PEEKTEXT, *pid, ret + i*sizeof(long), NULL);
    memcpy(buf, &chr, 4);
    write(1, buf, 4);
  }
  printf("\n");
  
 /*call dlopen */
  memset(args, 0, sizeof(args));
  args[0] = (long)ret;
  args[1] = RTLD_NOW| RTLD_GLOBAL;
  
  if(ptraceCall(pid, dlopenAddr, args, 2, &regs) < 0)
  {
    fprintf(stderr, "[ERROR]Failed to call dlopen\n");
    return -1;
  }

  ret = regs.ARM_r0;
  printf("[INFO]Dlopen successfully at %lx\n", ret);
  debug(pid);

/* recover regs */
  if(ptrace(PTRACE_SETREGS, *pid, NULL, &savedRegs) < 0)
  {
    fprintf(stderr, "[ERROR]Failed to recover regs\n");
    return -1;
  }

  ptrace(PTRACE_DETACH, *pid, NULL, NULL);
  return 0;
}

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
