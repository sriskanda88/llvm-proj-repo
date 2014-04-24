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

namespace {
    struct BranchBias : public ModulePass {
        static char ID;

        // Pointers to the helper functions
        Function *branchFound; //Keeps the total counter
        Function *branchTaken; // Keeps a counter of Taken branches
        Function *printFunction; // Prints Statistics

        Module *Mod;
        StringRef functionName; // The name of the function being currently analyzed

        BranchBias() : ModulePass (ID) {}

        virtual bool runOnModule (Module &M) {
            Mod = &M;

            // Initialize the pointers to the functions
            branchTaken = cast<Function> (M.getOrInsertFunction("_Z11branchTakenPc", 
                                          Type::getVoidTy(M.getContext()), 
                                          Type::getInt8PtrTy(M.getContext()), 
                                          (Type*)0));
            branchFound = cast<Function> (M.getOrInsertFunction("_Z11foundBranchPc", 
                                          Type::getVoidTy(M.getContext()), 
                                          Type::getInt8PtrTy(M.getContext()), 
                                          (Type*)0));
            printFunction = cast<Function> (M.getOrInsertFunction("_Z15printStatisticsv", 
                                            Type::getVoidTy(M.getContext()), 
                                            (Type*)0));

            // Not sure if these are necessary
            branchTaken->setCallingConv(CallingConv::C);
            branchFound->setCallingConv(CallingConv::C);

            for(Module::iterator F = M.begin(), E = M.end(); F!= E; ++F)
            {
                // Keep the function name in global variable
                functionName = (*F).getName();
                
                // Loop over all Basic Blocks
                for(Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
                {
                    BranchBias::runOnBasicBlock(BB);
                }

                // Add the print helper function before the program's end 
                // (last Basic Block of Main function)
                if ((*F).getName() == "main") {
                    Function::iterator lastBlock = F->end();
                    lastBlock--;
                    BranchBias::addPrintCall(lastBlock);
                }
            }
            return false;
        }

        virtual bool runOnBasicBlock (Function::iterator &BB) {
            
            Value *myStr;

            for(BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI)
            {
                //Only operate on Branch Instructions
                if(isa<BranchInst>(&(*BI)) ) {
                    BranchInst *CI = dyn_cast<BranchInst>(BI);
                    // We only care about conditional branches
                    if (CI->isUnconditional()) continue;

                    // It's either Taken or Not so it must have 
                    // exactly two successor blocks
                    if (CI->getNumSuccessors() != 2) continue;

                    // INSTRUMENTATION

                    // Create a call to branchTaken when the branch is taken
                    // Successor 0 is the "Taken" basic block

                    // Get the "Taken" Basic Block and move to its first instruction
                    BasicBlock *block = CI->getSuccessor(0);
                    BasicBlock::iterator takenIns = block->begin();
                    takenIns++;

                    // Create an IRBuilder instance and set the insert point 
                    // to the beginning of the "Taken" basic block
                    IRBuilder<> builder(block);
                    builder.SetInsertPoint(takenIns);

                    // Create a globally visible string with the function name and 
                    // call the branchTaken function with it

                    // Only create the string on the first block
                    if (BI == BB->begin()) {
                        myStr = builder.CreateGlobalStringPtr(functionName, "myStr");
                    }
                    builder.CreateCall (branchTaken, myStr);

                    /* CALL TO BRANCH TAKEN COMPLETED */

                    // Create a call to branchFound function to keep track of the 
                    // total number of branches

                    // Set the insert point at the current instruction and place 
                    // a call instruction
                    builder.SetInsertPoint(CI);
                    builder.CreateCall (branchFound, myStr);

                    /* INSTRUMENTATION COMPLETED */
                }
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

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("BranchBias", "Test Branch Bias per function", false, false);
