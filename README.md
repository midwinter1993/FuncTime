### FuncTime(C/C++)

A simple instrumentation tool to capture the cost time of function call.

It supports C well but not have a nice effect for C++, because the function name is confusing. To solve such problem, you should put functions in *Extern "C"*.

It can't get the time of functions in libraries. Actually, I made it just to do some proactice in using LLVM.

Powered by: LLVM-3.4
