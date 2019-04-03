#include <map>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace {

class Instrument : public ModulePass {
public:
    bool runOnModule(Module &M) override;
    Instrument();

    GlobalVariable *make_global_string(Module &mod, StringRef s, uint64_t len);

    static char ID;

private:
    LLVMContext *ctx;
    std::map<std::string, GlobalVariable *> m_string_ptr;
};

Instrument::Instrument() : ModulePass(ID) {
    ctx = &getGlobalContext();
}

GlobalVariable *Instrument::make_global_string(Module &mod, StringRef s,
                                               uint64_t len) {
    auto key = s.str();
    auto item = m_string_ptr.find(key);
    if (item != m_string_ptr.end())
        return item->second;

    // Create the type of global string variable: ArrayType
    ArrayType *ArrayTy_0 =
        ArrayType::get(IntegerType::get(mod.getContext(), 8), len);
    // define the global string variable of
    GlobalVariable *g_ptr = new GlobalVariable(mod, ArrayTy_0, true,
                                               GlobalValue::InternalLinkage, 0);
    g_ptr->setAlignment(1);
    // initilize global string variable with a constant string
    g_ptr->setInitializer(ConstantDataArray::getString(*ctx, s));

    m_string_ptr[key] = g_ptr;
    return g_ptr;
}

bool Instrument::runOnModule(Module &M) {
    DEBUG(int cnt = 1;);

    for (auto f = M.begin(); f != M.end(); ++f) {
        if (f->getName().equals("main")) {
            Constant *init_func =
                M.getOrInsertFunction("init_func", Type::getVoidTy(*ctx), NULL);
            CallInst::Create(init_func, "", &*f->begin()->begin());
        }
        for (auto blk = f->begin(); blk != f->end(); ++blk) {
            for (auto ist = blk->begin(); ist != blk->end(); ++ist) {
                DEBUG(errs() << cnt++ << ": " << *ist << "\n";);

                if (CallInst *i = dyn_cast<CallInst>(&*ist)) {
                    Function *fun = i->getCalledFunction();
                    if (fun->getName().startswith("llvm"))
                        continue;
                    if (fun) {
                        // BitCastInst *cast_inst = new BitCastInst((Value*)fun,
                        // Type::getInt8PtrTy(*ctx));
                        GlobalVariable *g_value = make_global_string(
                            M, fun->getName(), fun->getName().size() + 1);
                        BitCastInst *cast_inst = new BitCastInst(
                            (Value *)g_value, Type::getInt8PtrTy(*ctx));

                        Constant *before_fun = M.getOrInsertFunction(
                            "before_call", Type::getVoidTy(*ctx),
                            Type::getInt8PtrTy(*ctx), NULL);
                        Constant *after_fun = M.getOrInsertFunction(
                            "after_call", Type::getVoidTy(*ctx),
                            Type::getInt8PtrTy(*ctx), NULL);

                        std::vector<Value *> args;
                        args.push_back(cast_inst);
                        CallInst *call_before =
                            CallInst::Create(before_fun, args);
                        CallInst *call_after =
                            CallInst::Create(after_fun, args);

#if (__clang_minor__ == 4)
                        blk->getInstList().insert(i, cast_inst);
                        blk->getInstList().insert(i, call_before);
                        blk->getInstList().insertAfter(i, call_after);
#elif (__clang_minor__ == 8)
                        blk->getInstList().insert(i->getIterator(), cast_inst);
                        blk->getInstList().insert(i->getIterator(), call_before);
                        blk->getInstList().insertAfter(i->getIterator(), call_after);
#endif
                        ++ist;

                        DEBUG(errs() << (unsigned)fun; errs() << "Call: ";
                              errs().write_escaped(fun->getName()) << "\n";);
                    }
                }
            }
        }
    }
    DEBUG(errs() << "----------------------------------\n"; cnt = 1;
          for (auto f = M.begin(); f != M.end(); ++f) {
              for (auto blk = f->begin(); blk != f->end(); ++blk) {
                  for (auto ist = blk->begin(); ist != blk->end(); ++ist) {
                      errs() << cnt++ << ": " << *ist << "\n";
                  }
              }
          });
    return false;
}
}

char Instrument::ID = 0;
static RegisterPass<Instrument> X("instrument", "Instrument Pass", false,
                                  false);
