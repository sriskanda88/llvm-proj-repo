#include "llvm/Pass.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"

#include "llvm/IR/IRBuilder.h"

#include <iostream>
using namespace std;

using namespace llvm;

namespace{
    struct bishe_insert : public ModulePass{
        static char ID;   
        Function *branchFound;
        Function *branchTaken;
        Function *branchNotTaken;

        Function *testArgs;
        Function *testArgs2;
        Module *Mod;
        StringRef functionName;
        
        bishe_insert() : ModulePass(ID) {}

        virtual bool runOnModule(Module &M)
        {
            Mod = &M;
/*
            Constant *hookFunc;
            hookFunc = M.getOrInsertFunction("printBranch", Type::getVoidTy(M.getContext()), (Type*)0);

            branchFound = cast<Function>(hookFunc);

            Constant *TFunc;
            TFunc = M.getOrInsertFunction("printBranchTaken", Type::getVoidTy(M.getContext()), (Type*)0);
            branchTaken = cast<Function>(TFunc);


            // Try to pass arguments
            testArgs = cast<Function>(M.getOrInsertFunction("testPrint", Type::getVoidTy(M.getContext()), IntegerType::get(M.getContext(), 32), (Type*)0));
*/
            branchTaken = cast<Function>(M.getOrInsertFunction("_Z11branchTakenPc", Type::getVoidTy(M.getContext()), Type::getInt8PtrTy(M.getContext()), (Type*)0));
            branchNotTaken = cast<Function>(M.getOrInsertFunction("_Z11foundBranchPc", Type::getVoidTy(M.getContext()), Type::getInt8PtrTy(M.getContext()), (Type*)0));
            //testArgs2 = cast<Function>(M.getOrInsertFunction("testPrint2", Type::getVoidTy(M.getContext()), PointerType::getUnqual(Type::getInt8Ty(M.getContext())), (Type*)0));

            branchTaken->setCallingConv(CallingConv::C);
            branchNotTaken->setCallingConv(CallingConv::C);

            errs() << "Module: " << M.getModuleIdentifier() << "\n";


            for(Module::iterator F = M.begin(), E = M.end(); F!= E; ++F)
            {
                errs() <<"Function Name: " << (*F).getName()<<"\n";            
                functionName = (*F).getName();
                if ((*F).getName() == "_Z11foundBranchPc") {
                    //errs() << "Helper Function. Continuing...\n";
                    continue;   
                }
                if ((*F).getName() == "helperFunction") {
                    //errs() << "Helper Function. Continuing...\n";
                    continue;   
                } 
                if ((*F).getName() == "_Z11branchTakenPc") {
                    //errs() << "Helper Function. Continuing...\n";
                    continue;
                }
                for(Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
                {
                    bishe_insert::runOnBasicBlock(BB);
                }
                //errs()<<(*F);
            }

            return false;
        }
        virtual bool runOnBasicBlock(Function::iterator &BB)
        {
            for(BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI)           
            {
                    if(isa<BranchInst>(&(*BI)) )
                    {
                            BranchInst *CI = dyn_cast<BranchInst>(BI);
                            if (CI->isUnconditional()) continue;
                            //errs() << "Found branch\n";
                            if (CI->getNumSuccessors() <= 1) {
                                //errs() << "Continuing...\n";
                                continue;
                            }
                            //errs() << "Successors: "<< CI->getNumSuccessors()<<"\n";

            //BasicBlock* block = BB;
            Function *imgFunc = cast<Function>(Mod->getOrInsertFunction("helperFunction", Type::getVoidTy(Mod->getContext()), (Type*)0));
            BasicBlock* entry = BasicBlock::Create(Mod->getContext(), "entry", imgFunc);
            BasicBlock* cond_true = BasicBlock::Create(Mod->getContext(), "condTrue", imgFunc);
            BasicBlock* cond_false = BasicBlock::Create(Mod->getContext(), "condFalse", imgFunc);

            IRBuilder<> builder(entry);
            Value *myStr = builder.CreateGlobalStringPtr(functionName, "myStr");
            Value *condition = CI->getCondition();
            //errs() << "Condition: " << *condition<<"\n";
            builder.CreateCondBr (condition, cond_true, cond_false);

            builder.SetInsertPoint(cond_true);
            builder.CreateCall (branchTaken, myStr);
            builder.CreateRetVoid();

            builder.SetInsertPoint(cond_false);
            builder.CreateCall (branchNotTaken, myStr);
            builder.CreateRetVoid();

            builder.SetInsertPoint (BI);
            builder.CreateCall(imgFunc, "");
           
/*
            //Instruction* newInst = CallInst::Create(imgFunc, "");

            //errs() << *imgFunc << "\n";

            //BB->getInstList().insert(CI, newInst);
            //(CI->getSuccessor(0))->getInstList().push_front(newInst);


            IRBuilder<> builder(BB);
            Function* cur_func = BB->getParent();
            BasicBlock* cond_true = BasicBlock::Create(Mod->getContext(), "condTrue", cur_func);
            BasicBlock* cond_false = BasicBlock::Create(Mod->getContext(), "condFalse", cur_func);

            builder.SetInsertPoint (BI);
            Value *myStr = builder.CreateGlobalStringPtr(functionName, "myStr");
            Value *condition = CI->getCondition();
            builder.CreateCondBr (condition, cond_true, cond_false);

            builder.SetInsertPoint (cond_true);
            builder.CreateCall (branchTaken, myStr);
            builder.CreateRetVoid();

            builder.SetInsertPoint (cond_false);
            builder.CreateCall (branchNotTaken, myStr);
            builder.CreateRetVoid();
*/
                    }
                   
            }
            return true;
        }
    };
}
char bishe_insert::ID = 0;
static RegisterPass<bishe_insert> X("bishe_insert", "test function exist", false, false);

