#include "cross_platform_seh.h"
#include <stdio.h>
#include <iostream>


void do_something_bad(int testcase)
{
    // 这里可以放置一些可能引发异常的代码
    // 例如访问无效内存、除以零等
    switch (testcase)
    {
    case 1:
    {
        int* p = nullptr;
        *p = 42; // 这将引发访问违规异常
    }
    break;
    case 2:
    {
        int a = 1;
        int b = 0;
        int c = a / b; // 这将引发除以零异常
        (void)c; // 避免未使用变量警告
    }
    break;
    case 3:
    {
        char* p = (char*)0x12345678; // 假设这是一个无效地址
        char value = *p; // 这将引发访问违规异常
        (void)value; // 避免未使用变量警告
    }
    break;
    default:
        break;
    }
}

void do_clean_work(int testcase)
{
    switch (testcase)
    {
    case 1:
    {

    }
    default:
        break;
    }
}

void Test(void)
{
    {
        EXCEPTION_NODE __weLees_ExceptionNode;
        __weLees_ExceptionNode.Prev = __null;
        if (!s_Maintain.Initialized)
        {
            StartSEHService();
        }
        printf(" Test111: %p, %p\n", s_Maintain.SignalHandler.sa_sigaction, s_Maintain.OldHandler.sa_sigaction);
        __weLees_ExceptionNode.Prev = s_Maintain.Chain;
        s_Maintain.Chain = &__weLees_ExceptionNode;
        __weLees_ExceptionNode.RunStatus = __sigsetjmp(__weLees_ExceptionNode.SectionEntry, 1);
        if (2 == __weLees_ExceptionNode.RunStatus)
        {
            {
                int* p = nullptr;
                *p = 42; // 这将引发访问违规异常
                int a = 1;
                int b = 0;
                int c = a / b; // 这将引发除以零异常
                //(void)c; // 避免未使用变量警告
                do_something_bad(0);
            }
            s_Maintain.Chain = __weLees_ExceptionNode.Prev;
        }
        else if (!__weLees_ExceptionNode.RunStatus)
        {
            __weLees_ExceptionNode.FilterRoutine = exception_execute_handler_func;
            siglongjmp(__weLees_ExceptionNode.SectionEntry, 2);
        }
        else
        {
            {
                do_clean_work(0);
            }
            ForwardSignalToOldHandler();
        }
    }
}

void Test1(void)
{
    char sz[] = "Exception critical section & exception clean up section was run in THE SAME FUNCTION, I show the same information as proof.";
    //printf("--------------Virtual SEH Test1 : simple exception handling--------------\n");
    //printf("  +Enter critical section @ %s %d\n", __FILE__, __LINE__); //进入异常监视区

    TRY_START
    {
        //------------------------------- Exception monitor block start -------------------------------
        //printf("Test1 : Hello, we go into exception monitor section\n  This is the local string :'%s'\n",sz);
        //printf("Test1 :   Let's do some bad thing\n");
        
		do_something_bad(1);

        //------------------------------- Exception monitor block end -------------------------------
        //printf("  -Leave critical section @ %s %d\n",__FILE__,__LINE__); //这一行其实不会运行
    }
    TRY_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        //------------------------------- Exception clean block start -------------------------------
        //printf("Test1 :   Exception occur! do clean work\n  and we can access local data in the same environment of exception critical section:\n'%s'\n", sz);
        //------------------------------- Exception clean block end -------------------------------
    }
    TRY_END
}

void Test2(void)
{
    char sz[] = "Exception critical section & exception clean up section was run in THE SAME FUNCTION, I show the same information as proof.";
    //printf("\n--------------Virtual SEH Test2 : nested exception handling--------------\n");

    TRY_START
    {
        //------------------------------- Exception monitor block start -------------------------------
        //printf("Test2 : +Enter outer critical section @ %s %d\n",__FILE__,__LINE__); //进入异常监视区
        TRY_START
        {
           //------------------------------- Exception monitor block start -------------------------------
           //printf("Test2 : +Enter inner critical section @ %s %d\n",__FILE__,__LINE__); //进入异常监视区
           //printf("Test2 : Hello, we go into exception monitor section\n  This is the local string :'%s'\n",sz);
           //printf("Test2 :   Let's do some bad thing\n");

           do_something_bad(2);

           //------------------------------- Exception monitor block end -------------------------------
           //printf("  -Leave critical section @ %s %d\n",__FILE__,__LINE__); //这一行其实不会运行
        }
        TRY_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            //------------------------------- Exception clean block start -------------------------------
            //printf("Test2 :   Exception occur in INNER level! do clean work\n  and we can access local data in the same environment of exception critical section:\n'%s'\n",sz);
            //------------------------------- Exception clean block end -------------------------------
        }
        TRY_END
    }
    TRY_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        //------------------------------- Exception clean block start -------------------------------
        //printf("Test2 :   Exception occur in outer level! do clean work\n  and we can access local data in the same environment of exception critical section:\n'%s'\n", sz);
        //------------------------------- Exception clean block end -------------------------------
    }
    TRY_END
}

void Test3(void)
{
    char sz[] = "Exception critical section & exception clean up section was run in THE SAME FUNCTION, I show the same information as proof.";
    //printf("\n--------------Virtual SEH Test3 : nested exception handling-try outer exception handler --------------\n");

    TRY_START
    {
        //------------------------------- Exception monitor block start -------------------------------
        //printf("Test3 : +Enter outer critical section @ %s %d\n",__FILE__,__LINE__); //进入异常监视区
        TRY_START
        {
            //------------------------------- Exception monitor block start -------------------------------
            //printf("Test3 : +Enter inner critical section @ %s %d\n",__FILE__,__LINE__); //进入异常监视区
            //printf("Test3 : Hello, we go into exception monitor section\n  This is the local string :'%s'\n",sz);
            //printf("Test3 :   Let's do some bad thing\n");

            do_something_bad(3);

            //------------------------------- Exception monitor block end -------------------------------
            //printf("  -Leave critical section @ %s %d\n",__FILE__,__LINE__); //这一行其实不会运行
        }
        TRY_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            //------------------------------- Exception clean block start -------------------------------
            //printf("Test3 :   Exception occur in INNER level! do clean work\n  and we can access local data in the same environment of exception critical section:\n'%s'\n",sz);
            //------------------------------- Exception clean block end -------------------------------
        }
        TRY_END
    }
    TRY_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        //------------------------------- Exception clean block start -------------------------------
        //printf("Test3 :   Exception occur in outer level! do clean work\n  and we can access local data in the same environment of exception critical section:\n'%s'\n", sz);
        //------------------------------- Exception clean block end -------------------------------
    }
    TRY_END
}


int main(void)
{
    Test();
    Test1();
    do_something_bad(1);
    Test2();
    Test3();
    TerminateSEHService();
    return 0;
}