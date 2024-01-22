#define REG_ARGS_NUM 4
#define WORD sizeof(unsigned long)

int ptraceWrite(pid_t *pid, unsigned long target, void *content, unsigned long length)
int ptraceCall(pid_t *pid, unsigned long target, unsigned long *args, unsigned long argsNum, struct pt_regs *regs)
int ptraceInject(pid_t *pid)