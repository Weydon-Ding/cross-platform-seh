#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32


#ifndef _WIN32

// Defined values for the exception filter expression
enum {
    exception_execute_handler = 1,
    exception_continue_search = 0,
    exception_continue_execution = -1,
};

// Typedef for pointer returned by exception_info()
typedef struct _EXCEPTION_POINTERS {
    siginfo_t* ExceptionRecord;
    void* ContextRecord;
} EXCEPTION_POINTERS, * PEXCEPTION_POINTERS;

int exception_execute_handler_func(unsigned long code, PEXCEPTION_POINTERS info)
{
    return exception_execute_handler;
}

int exception_continue_search_func(unsigned long code, PEXCEPTION_POINTERS info)
{
    return exception_continue_search;
}

int exception_continue_execution_func(unsigned long code, PEXCEPTION_POINTERS info)
{
    return exception_continue_execution;
}

// 识别异常。
// 通过执行 __except 复合语句将控制权传输到异常处理程序，然后在块之后 __except 继续执行。
#define EXCEPTION_EXECUTE_HANDLER        exception_execute_handler_func

// 无法识别异常。
// 继续搜索处理程序的堆栈，首先搜索包含 try-except 语句，然后搜索具有下一个最高优先级的处理程序。
#define EXCEPTION_CONTINUE_SEARCH        exception_continue_search_func

// 异常将被消除。
// 在发生异常时继续执行。
#define EXCEPTION_CONTINUE_EXECUTION     exception_continue_execution_func


// 异常块相关信息
typedef struct _EXCEPTION_NODE {
    struct _EXCEPTION_NODE* Prev;
    int RunStatus;
    jmp_buf SectionEntry; // 保存异常监视区入口以及运行环境
    int (*FilterRoutine)(unsigned long code, PEXCEPTION_POINTERS info); // 异常过滤函数指针
} EXCEPTION_NODE, * PEXCEPTION_NODE;


// SEH管理块（线程安全）
static __thread struct _SEH_SET
{
    PEXCEPTION_NODE Chain; // 异常块链表头指针
    struct sigaction SignalHandler, OldHandler;
    int Initialized;
} s_Maintain = { 0 };


// 异常处理函数
// 直接跳转到异常监视区开始位置，并利用setjmp/longjmp函数的特性跳转到异常环境回收例程
static void _ExceptionHandler(int iSignal, siginfo_t* pSignalInfo, void* pContext)
{
    int iResult;
    PEXCEPTION_NODE pEntry, pPrev;
    EXCEPTION_POINTERS info;
    //printf("    Got SIGSEGV at address: %lXH, %p\n", (long)pSignalInfo->si_addr, pContext);

    pEntry = s_Maintain.Chain; // 准备遍历注册的异常处理块
    do
    {
        // 调当前异常块的异常过滤函数
        info.ExceptionRecord = pSignalInfo;
        info.ContextRecord = pContext;
        iResult = pEntry->FilterRoutine(iSignal, &info);

        switch (iResult)
        {
        case exception_execute_handler:          // 当前异常无法解决，执行善后清理工作
            s_Maintain.Chain = pEntry->Prev;     // 注销当前异常块
            siglongjmp(pEntry->SectionEntry, 1); // 跳转到注册异常块的善后清理入口
            break;
        case exception_continue_search:          // 注册异常块无法处理当前异常，尝试上一级异常处理
            pPrev = pEntry->Prev;
            s_Maintain.Chain = pEntry->Prev;     // 既然需要到上一级异常块处理，那么当前异常块已经无用了，注销当前异常块
            pEntry = pPrev;                      // 转到上一级异常处理块
            break;
        case exception_continue_execution:       // 异常修复完成，返回异常发生处继续运行
            return;
            break;
        default: // 异常过滤程序返回了错误的值
            //printf("Bad exception filter result %d\n", iResult);
            break;
        }
    } while (pEntry);

    //printf("    No more handler\n");
    sigaction(SIGSEGV, &s_Maintain.OldHandler, NULL);
}
#endif


//异常处理初始化函数，应该程序启动之后运行
int StartSEHService(void)
{
#ifndef _WIN32
    s_Maintain.SignalHandler.sa_sigaction = _ExceptionHandler;
    //注册自己的异常处理函数
    if (-1 == sigaction(SIGSEGV, &s_Maintain.SignalHandler, &s_Maintain.OldHandler))
    {
        perror("Register sigaction fail");
        return 0;
    }
    s_Maintain.Initialized = 1;
#endif // _WIN32
    return 1;
}

int TerminateSEHService(void)
{
#ifndef _WIN32
    sigaction(SIGSEGV, &s_Maintain.OldHandler, &s_Maintain.OldHandler); //恢复原有异常处理函数
#endif
    return 1;
}

#ifdef _WIN32

#define TRY_START __try
#define TRY_EXCEPT(filter) __except(filter/*(GetExceptionCode(), GetExceptionInformation())*/)
#define TRY_END

#else

#define TRY_START \
{ \
    EXCEPTION_NODE __weLees_ExceptionNode; \
    __weLees_ExceptionNode.Prev=NULL; \
    if(!s_Maintain.Initialized) \
    { \
        StartSEHService(); \
    } \
    __weLees_ExceptionNode.Prev=s_Maintain.Chain; \
    s_Maintain.Chain=&__weLees_ExceptionNode; \
    __weLees_ExceptionNode.RunStatus=sigsetjmp(__weLees_ExceptionNode.SectionEntry,1);/*保存当前运行位置以及环境信息*/ \
    if(2==__weLees_ExceptionNode.RunStatus) \
    {

#define TRY_EXCEPT(filter) \
        s_Maintain.Chain=__weLees_ExceptionNode.Prev; \
    } \
    else if(!__weLees_ExceptionNode.RunStatus)/*setjmp返回0表示刚刚完成了环境保存工作，此时我们需要注册异常过滤例程*/ \
    { \
        /*printf("  *Register exception filter @ %s %d\n",__FILE__,__LINE__);*/ \
        __weLees_ExceptionNode.FilterRoutine=filter;/*注册异常过滤函数*/ \
        siglongjmp(__weLees_ExceptionNode.SectionEntry,2);/*跳转到异常监视块开始处*/ \
    } \
    else {/*非0/2表示是从系统异常处理程序中的longjmp函数跳转过来，在本例中表明发生了异常并无法恢复运行，处理善后工作*/

#define TRY_END }}

#endif
