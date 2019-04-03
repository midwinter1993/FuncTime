a.out: opt.bc runtime.bc
	clang++ -o $@ $^

opt.bc: pass.so hello.bc
	@echo '==================================='
	opt -load ./pass.so -instrument < hello.bc > $@
	@echo '==================================='

hello.bc: hello.c
	clang -emit-llvm $^ -O0 -o hello.bc -c

pass.so: pass.cpp
	clang++ -std=c++0x -shared `llvm-config --cxxflags` `llvm-config --ldflags` -lLLVM-3.4 -o $@ $^

runtime.bc: runtime.cpp
	clang++ -std=c++0x -emit-llvm -c -o $@ $^

.PHONY: clean
clean:
	-rm a.out *.bc pass.so *.ll
