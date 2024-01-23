#include<stdio.h>
#include<stdlib.h>
#include<string.h>
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

#include<../include/ptraceWrapper.h>
#include<../include/config.h>
#include<../include/utils.h>

int ptraceWrite(pid_t *pid, unsigned long target, char *content, unsigned long length){
  unsigned long cnt = length % WORD ? length / WORD + 1 : length / WORD;
  for(int i = 0; i < cnt; i++)
    if(ptrace(PTRACE_POKETEXT, *pid, target + i * WORD, *((unsigned long *)content + i)) < 0){
      return -1;
    }

  return 0;
}

void ptraceRead(pid_t *pid, unsigned long target, char *buf, unsigned long length){
  unsigned long cnt = length % WORD ? length / WORD + 1 : length / WORD;
  unsigned long tmp;
  for(int i = 0; i < cnt; i++){
    tmp = ptrace(PTRACE_PEEKTEXT, *pid, target + i * WORD, NULL);
    memcpy((unsigned long *)buf + i, &tmp, sizeof(tmp));
  }

  return;
}

int ptraceCall(pid_t *pid, unsigned long target, unsigned long *args, unsigned long argsNum, struct pt_regs *regs){
  /* set args */
  if(argsNum > REG_ARGS_NUM){
    regs->ARM_sp -= (argsNum - REG_ARGS_NUM) * WORD;
    if(ptraceWrite(
          pid,
          (unsigned long)regs->ARM_sp,
          (char *)(args + REG_ARGS_NUM),
          (argsNum - REG_ARGS_NUM) * WORD) < 0){
      fprintf(stderr, "[ERROR]write args failed");
      return -1;
    }
  }

  for(int i = 0; i < REG_ARGS_NUM; i++)  regs->uregs[i] = args[i];

  /* judge thumb or arm */
  regs->ARM_pc = target;
  if(regs->ARM_pc & 1){
    regs->ARM_pc &= (~1u);
    regs->ARM_cpsr |= CPSR_T_MASK;
  }
  else{
    regs->ARM_cpsr &= ~CPSR_T_MASK;
  }

  /* trigger signal */
  regs->ARM_lr = 0;

  /* set regs */
  if(ptrace(PTRACE_SETREGS, *pid, NULL, regs) < 0){
    fprintf(stderr, "[ERROR]Set registers failed\n");
    return -1;
  }

  /* continue */
  int stat = 0;
  while(stat != 0xb7f){
    if(ptrace(PTRACE_CONT, *pid, NULL, NULL) < 0){
      fprintf(stderr, "[ERROR]Continue failed\n");
      return -1;
    }
    waitpid(*pid, &stat, WUNTRACED);
  }

  /* return value saved*/
  if(ptrace(PTRACE_GETREGS, *pid, NULL, regs) < 0){
    fprintf(stderr, "[ERROR]Get return value failed\n");
    return -1;
  }

  return 0;
}

int ptraceInject(pid_t *pid){	
  /* get function address */
  unsigned long dlopenAddr = lookupFuncAddr(dlopen, LIBDLSO, pid);
  unsigned long mmapAddr = lookupFuncAddr(mmap, LIBCSO, pid);
	printf("[INFO]Functions address got: %lx, %lx\n", dlopenAddr, mmapAddr);
 
  /* attach */
	if(ptrace(PTRACE_ATTACH, *pid, NULL, NULL) < 0){
    fprintf(stderr, "[ERROR]Failed to attach target process\n");
  }
  waitpid(*pid, NULL, WUNTRACED);

  struct pt_regs savedRegs, regs;
  memset(&savedRegs, 0, sizeof(struct user_regs));
  memset(&regs, 0, sizeof(struct user_regs));

  if(ptrace(PTRACE_GETREGS, *pid, NULL, &savedRegs) < 0){
    fprintf(stderr, "[ERROR]Failed to Get registers information\n");
    return -1;
  }
  memcpy(&regs, &savedRegs, sizeof(struct pt_regs));

  /* call mmap*/
  unsigned long args[0x10], ret;
  args[0] = 0;
  args[1] = 0x100;
  args[2] = PROT_READ|PROT_WRITE|PROT_EXEC;
  args[3] = MAP_ANONYMOUS|MAP_PRIVATE;
  args[4] = 0;
  args[5] = 0;
  
  if(ptraceCall(pid, mmapAddr, args, 6, &regs) < 0){
    fprintf(stderr, "[ERROR]Failed to call mmap\n");
    return -1;
  }
  ret = regs.ARM_r0;
  printf("[INFO]Mmap successfully at %lx\n", ret);

  /* write lib path */
  if(ptraceWrite(pid, ret, HIJACKLIBCPATH, strlen(HIJACKLIBCPATH)) < 0){
      fprintf(stderr, "[ERROR]Failed to write libc path to memory\n");
      return -1;
    }

  char buf[0x30];
  ptraceRead(pid, ret, buf, strlen(HIJACKLIBCPATH));
  printf("[INFO]write successfully:\n%s\n", buf);
  
 /*call dlopen */
  args[0] = (long)ret;
  args[1] = RTLD_NOW| RTLD_GLOBAL;
  
  if(ptraceCall(pid, dlopenAddr, args, 2, &regs) < 0){
    fprintf(stderr, "[ERROR]Failed to call dlopen\n");
    return -1;
  }

  ret = regs.ARM_r0;
  printf("[INFO]Dlopen successfully at %lx\n", ret);

/* recover regs */
  if(ptrace(PTRACE_SETREGS, *pid, NULL, &savedRegs) < 0){
    fprintf(stderr, "[ERROR]Failed to recover regs\n");
    return -1;
  }

  ptrace(PTRACE_DETACH, *pid, NULL, NULL);

  return 0;
}
