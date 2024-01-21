#include<android/log.h>
#define LOG_TAG "INJECT"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG, __VA_ARGS__))
void _init(void) 
{
  LOGD("Injected\n");  
}

