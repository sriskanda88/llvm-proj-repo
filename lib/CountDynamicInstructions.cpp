#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include <string>
#include <map>

using namespace llvm;
using namespace std;

map<string, int> InstCountMap;

namespace {
    struct DICount : public ModulePass {
        static char ID;

        Function *printFunction; // Prints Statistics

        Module *Mod;
        StringRef functionName; // The name of the function being currently analyzed

        DICount() : ModulePass(ID) {}

        virtual bool runOnModule(Module &M){
            Mod = &M;

            //Pointers to functions
            printFunction = cast<function> (M.getOrInsertFunction("printOutput",
                                            Type::getVoidTy(M.getContext()),
                                            (Type*)0));


            if(!printFunction){
                errs() << "function not in symbol table\n";
                exit(1);
            }

            for (Module::iterator F = M.begin(), FE = M.end(); F != FE; F++){
                Function* func = &*F;

                for (Function::iterator BB = func->begin(), BBE = func->end(); BB != BBE; BB++){

                    BasicBlock *bblock = &*BB;
                    InstCountMap.clear();
                    Instruction* inst;
                    /*
                       for (BasicBlock::iterator I = bblock->begin(), IE = bblock->end(); I != IE; I++){
                       inst = &*I;
                       string opname = inst->getOpcodeName();
                       InstCountMap[opname]++;
                       }

                       map <string, int>::const_iterator iter;
                       for (iter = InstCountMap.begin(); iter != InstCountMap.end(); iter++){
                       params.push_back((Type * const&)iter->first);
                       params.push_back((Type * const&)iter->second);
                       }

                       llvm::LLVMContext& context = llvm::getGlobalContext();
                       llvm::Module* module = new llvm::Module("top", context);
                       llvm::IRBuilder<> builder(context);
                       builder.CreateCall(myFunc, (Value*)"test", (Value *)1, (const Twine &)"tmp");
                       */

                    IRBuilder<> Builder(BB);
                    BasicBlock::iterator bbIter = bblock->end();
                    bbIter--;
                    builder.SetInsertPoint(bbIter);
                    builder.CreateCall(printFunction, "");
                }
            }
            return false;
        }
    };
}

char DICount::ID = 0;
static RegisterPass<DICount> X("dynamic_icount", "Dynamic Instruction Count", false, false);
