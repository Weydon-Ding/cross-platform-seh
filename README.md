# 跨平台结构化异常处理（SEH）

> [!NOTE]
>
> 本文档和代码是由 AI 协作完成的，旨在提供一个跨平台的结构化异常处理（SEH）解决方案。
>
> 请注意，AI 生成的内容可能包含错误或不准确之处，作者已经基本审查过。如有遗漏或错误，请在 GitHub 上提交 Issue 或 Pull Request 进行修正。

## 项目概述

该项目实现了一个跨平台的结构化异常处理（SEH）机制，原本只在 Windows 平台上支持，现在扩展到支持 POSIX 系统（如 Linux、macOS 等）。该 SEH 机制提供了一种可靠的方式来处理异常和信号，使得您的应用能够以一致的方式捕获和处理错误、异常或信号，并且跨平台兼容。

> [!IMPORTANT]
>
> 大部分代码转载自知乎文章: [一种通用跨平台实现SEH的解决方案](https://zhuanlan.zhihu.com/p/1920929429057673090)，原作者[areslee](https://www.zhihu.com/people/aresleejian-chun)。
>
> 我按原作者的思路，将Windows 和 POSIX 系统的异常处理机制进行了整合，提供了一个统一的接口来处理异常。

### 主要特性
- **跨平台支持**：同时支持 Windows 和 POSIX 系统（Linux、macOS 等）。
- **信号处理**：使用 POSIX 的 `sigaction` 和 Windows 的异常处理机制来进行信号和异常的管理。
- **线程安全**：该 SEH 机制通过线程局部存储（`__thread`）实现，每个线程有独立的异常处理状态。
- **自定义异常过滤器**：允许通过函数指针自定义异常过滤和处理逻辑。

### 待办事项

- [ ] 支持从异常监视区中退出循环，即break指令。
- [ ] 支持从异常监视区中退出函数，即return指令。
- [ ] 增加更多的测试用例，覆盖更多的异常场景。
- [ ] 验证移动端（Android和iOS）的支持。
- [ ] 完善CMake构建脚本。

## 使用方法

### Windows

在 Windows 系统上，SEH 机制使用 Windows 特定的异常处理 API，但对于 POSIX 系统，它使用 `sigaction` 提供类似的功能。

### POSIX（Linux/macOS）

对于 POSIX 系统，代码使用 `sigaction` 来注册自定义的信号处理程序，用于捕获异常信号（如 `SIGSEGV`）。异常处理机制通过系统的信号处理 API 集成。

### 示例代码

```c
#include "cross_platform_seh.h"


int main()
{
    TRY_START
    {
        TRY_START
        {
            // 可能引发异常的代码
            int* p = NULL;
            *p = 42;  // 这将导致段错误
        }
        TRY_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            // 捕获异常并执行处理程序
        }
        TRY_END
    }
    TRY_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        // 捕获异常并执行处理程序
    }
    TRY_END

    return 0;
}
```

### 解释：

* `TRY_START`、`TRY_EXCEPT` 和 `TRY_END` 宏用于包装可能引发异常的代码。
* 异常处理流程会调用自定义的过滤函数 `my_filter` 来决定如何处理异常。

### 线程安全：

* `__thread` 关键字确保每个线程都有自己的 `s_Maintain` 变量副本，这使得异常处理机制对多线程环境是安全的。

## 贡献

欢迎大家为该项目做出贡献！如果您发现任何问题或有改进建议，请随时进行 Fork、创建分支并提交 Pull Request。

### 贡献步骤：

1. Fork 该仓库。
2. 创建新的功能分支。
3. 修改代码并提交清晰的提交信息。
4. 将修改推送到您的 Fork，并打开 Pull Request。

## 许可证

此项目采用 MIT 许可证 - 详见 [LICENSE](./LICENSE.txt) 文件。
