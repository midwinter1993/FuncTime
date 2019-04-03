### FuncTime (C/C++)

A simple instrumentation tool to capture the cost time of function call.
It supports C well but not works well on C++ because the function names are demangled.

Function calls in libraries are not captured--a common issue in instrumentation.

I coded it just to exercise using LLVM.

Powered by: LLVM-3.4
