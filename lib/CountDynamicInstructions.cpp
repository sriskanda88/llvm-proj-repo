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


namespace {
    struct DICount : public ModulePass {
        static char ID;

        Function *printFunction; // Prints Statistics
        Function *basicBlockInstCountFunction; // Basic block instruction count

        DICount() : ModulePass(ID) {}

        virtual bool runOnModule(Module &M){
            //Pointers to functions
            basicBlockInstCountFunction = cast<Function> (M.getOrInsertFunction("_Z26basicBlockInstructionCountPci",
                        Type::getVoidTy(M.getContext()),
                        Type::getInt8PtrTy(M.getContext()),
                        Type::getInt32Ty(M.getContext()),
                        NULL));

            printFunction = cast<Function> (M.getOrInsertFunction("_Z31printDynamicInstCountStatisticsv",
                        Type::getVoidTy(M.getContext()),
                        NULL));

            basicBlockInstCountFunction->setCallingConv(CallingConv::C);

            for (Module::iterator F = M.begin(), FE = M.end(); F != FE; F++){
                // Loop over all Basic Blocks
                for(Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
                {
                    DICount::runOnBasicBlock(BB, M);
                }

                // Add the print helper function before the program's end
                // (last Basic Block of Main function)
                if ((*F).getName().compare("main") == 0) {
                    Function::iterator lastBlock = F->end();
                    lastBlock--;
                    DICount::addPrintCall(lastBlock);
                }
            }
            return false;
        }


        virtual bool runOnBasicBlock (Function::iterator &BB, Module &Mod) {

                map<string, int> InstCountMap;
                Instruction* inst;
                Value *myStr, *myInt;

                for (BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; I++){
                    inst = &*I;
                    string opname = inst->getOpcodeName();
                    if (InstCountMap.count(opname) == 0){
                        InstCountMap[opname] = 1;
                    }
                    else {
                        InstCountMap[opname]++;
                    }
                }

                IRBuilder<> builder(BB);
                BasicBlock::iterator bbIter = BB->end();
                bbIter--;
                builder.SetInsertPoint(bbIter);

                map <string, int>::const_iterator iter;
                for (iter = InstCountMap.begin(); iter != InstCountMap.end(); iter++){
                    myStr = builder.CreateGlobalStringPtr(iter->first);
                    myInt = ConstantInt::get(Type::getInt32Ty(Mod.getContext()), iter->second);

                    builder.CreateCall2(basicBlockInstCountFunction, myStr, myInt);
                }

            return true;
        }

        virtual void addPrintCall (Function::iterator BB) {
            // Create an IRBuilder instance and set the insert point
            // to the very last instruction of main
            IRBuilder<> builder(BB);
            BasicBlock::iterator lastIns = BB->end();
            lastIns--;
            builder.SetInsertPoint(lastIns);

            // Create a call to the printFunction with no arguments
            builder.CreateCall (printFunction, "");
        }
    };
}

char DICount::ID = 0;
static RegisterPass<DICount> X("dynamic_icount", "Dynamic Instruction Count", false, false);
