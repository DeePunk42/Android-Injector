#define REG_ARGS_NUM 4
#define WORD sizeof(unsigned long)
#define CPSR_T_MASK (1u<<5)

int ptraceWrite(pid_t *pid, unsigned long target, void *content, unsigned long length)
void ptraceRead(pid_t *pid, unsigned long target, char *buf, unsigned long length)
int ptraceCall(pid_t *pid, unsigned long target, unsigned long *args, unsigned long argsNum, struct pt_regs *regs)
int ptraceInject(pid_t *pid)