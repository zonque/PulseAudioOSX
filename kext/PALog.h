#ifdef PA_DEBUG
#define debugIOLog(x...)    IOLog(x)
#else
#define debugIOLog(x...)    do {} while(0)
#endif

#define debugFunctionEnter() debugIOLog("%s(%p)::%s\n", getName(), this, __func__)
