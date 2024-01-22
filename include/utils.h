void Debug_reginfo(pid_t *pid)
int lookupPid(char *processName, pid_t *pid)
int lookupModuleAddr(char *moduleName, pid_t *pid, unsigned long *baseAddr)
unsigned long lookupFuncAddr(void *funcName, char *moduleName, pid_t *pid)