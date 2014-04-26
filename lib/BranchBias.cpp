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
        Function *currentFunction; // Prints Statistics
        BasicBlock *lastBlock;

        StringRef functionName; // The name of the function being currently analyzed

        BranchBias() : ModulePass (ID) {}

        virtual bool runOnModule (Module &M) {
            lastBlock = NULL;

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
                currentFunction = F;
                if (F->getName() == "main") lastBlock = &(F->back());
                
                // Loop over all Basic Blocks
                for(Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
                {
                    BranchBias::runOnBasicBlock(BB);
                }
            }
            return false;
        }

        virtual bool runOnBasicBlock (Function::iterator &BB) {
            
            Value *myStr;
            bool isFirst = true;

            for(BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI)
            {
                //If the function is main and we reached the return instruction
                //add a call to the print statistics function
                if ((isa<ReturnInst>(BI)) && (currentFunction->getName() == "main")) {
                    BranchBias::addPrintCall(BB);
                    
                }

                //Only operate on Branch Instructions
                if(isa<BranchInst>(&(*BI)) ) {
                    BranchInst *CI = dyn_cast<BranchInst>(BI);
                    // We only care about conditional branches
                    if (CI->isUnconditional()) {
                        continue;
                    }

                    // It's either Taken or Not so it must have 
                    // exactly two successor blocks
                    if (CI->getNumSuccessors() != 2) {
                        continue;
                    }

                    // INSTRUMENTATION

                    // Create a call to branchTaken when the branch is taken
                    // Successor 0 is the "Taken" basic block

                    // Get the "Taken" Basic Block and move to its first instruction
                    BasicBlock *takenBlock = CI->getSuccessor(0);
                    
                    //Create a new intermediate block
                    BasicBlock *newBlock = BasicBlock::Create(getGlobalContext(), "newBlock", currentFunction);

                    // Start building the new block
                    IRBuilder<> builder(newBlock);
                    
                    // Create a globally visible string with the function name and 
                    // call the branchTaken function with it

                    // Only create the string on the first block
                    if (isFirst) {
                        myStr = builder.CreateGlobalStringPtr(functionName, "myStr");
                        isFirst = false;
                    }
                    builder.CreateCall (branchTaken, myStr);
                    // Add unconditional jump to the "taken" block
                    builder.CreateBr (takenBlock);

                    // Fix the PHI node if it has one
                    CI->setSuccessor(0, newBlock); 
                    Instruction *takenIns = takenBlock->begin();
                    if (isa<PHINode>(takenIns)) {
                        PHINode *phi = cast<PHINode>(takenIns);
                        int bb_index = phi->getBasicBlockIndex(BB);
                        phi->setIncomingBlock (bb_index, newBlock);
                    }

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
            BasicBlock::iterator lastIns = BB->back();
            builder.SetInsertPoint(lastIns);

            // Create a call to the printFunction with no arguments
            builder.CreateCall (printFunction, "");
        }
    };
}

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("BranchBias", "Test Branch Bias per function", false, false);
