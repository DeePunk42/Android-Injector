#include<android/log.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include"../inc/utils.c"
#include"../inc/config.h"

#define LOG_TAG "INJECT"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

void _init(void){
    LOGD("Injected\n");

    pid_t pid;
    unsigned long base, target;
    lookupPid(TARGET_PROCESS, &pid);
    lookupModuleAddr(LIBCRACKME, &pid, &base);
    LOGD("module base:%p", base);
    target = base + 0x1234;
    memcpy((void *)target, 0xa0e1, sizeof(long));
}