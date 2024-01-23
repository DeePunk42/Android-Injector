#include<android/log.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/mman.h>

#include"../inc/config.h"

#define LOG_TAG "INJECT"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))


int lookupPid(char *processName, pid_t *pid){
	FILE *fp;
	char buf[0x100], user[0x30];
	if(!(fp = popen(PS_A, "r")))
		return -1;
	while(fgets(buf, sizeof(buf), fp))
		if(strstr(buf, processName)){
			sscanf(buf, "%s%d", user, pid); 
			return 0;
		}
	return -1;
}

int lookupModuleAddr(char *moduleName, pid_t *pid, unsigned long *baseAddr){
	FILE *fp;
	char cmd[0x20], buf[0x180], user[0x30];
	sprintf(cmd, "cat /proc/%d/maps", *pid);
	if(!(fp = popen(cmd, "r")))
		return -1;
	while(fgets(buf, sizeof(buf), fp))
		if(strstr(buf, moduleName)){
			sscanf(buf, "%lx", baseAddr); 
			return 0;
		}
	return -1;
}

void _init(void){
    LOGD("Injected\n");
    pid_t pid;
    unsigned long base, target, hook;
    if(lookupPid(TARGET_PROCESS, &pid)<0){
      LOGD("lookup pid failed\n");
      return;
    }
    if(lookupModuleAddr(LIBCRACKME, &pid, &base)<0){
      LOGD("get module addr failed\n");
      return;
    }
    LOGD("module base:0x%lx", base);

    target = base + 0x10B4;
    if(mprotect((void *)(target & ~(getpagesize() - 1)), getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC) < 0){
      LOGD("mprotect failed\n");
      return;
    }
    hook = 0xa0e1;
    memcpy((void *)target, &hook, sizeof(long));

    target = base + 0x1234;
    if(mprotect((void *)(target & ~(getpagesize() - 1)), getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC) < 0){
      LOGD("mprotect failed\n");
      return;
    }
    hook = 0xa0e1;
    memcpy((void *)target, &hook, sizeof(long));
}
