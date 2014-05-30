#ifndef _Analyzer_
#define _Analyzer_

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CFG.h"
#include "DFF.h"
#include <string>
#include <map>
#include <queue>
#include <list>
#include <set>

using namespace llvm;
using namespace std;

//enum to represent different instruction formats
enum InstType {X_Eq_C, X_Eq_C_Op_Z, X_Eq_Y_Op_C, X_Eq_C_Op_C, X_Eq_Y_Op_Z, X_Eq_Y, X_Eq_Addr_Y, X_Eq_Star_Y, Star_X_Eq_Y, PHI, UNKNOWN};

template <class T>
class DataFlowAnalyzer {

    protected:
        typedef map<string, set<T> > InstructionFact; //Instruction level data fact
        std::queue<BasicBlock *, std::list<BasicBlock *> > bbQueue; //Worklist queue
        DFF<T> dataFlowFactsMap; //DataFlowFact map for all instructions

        virtual bool flowFunction(Instruction* inst, InstType instType, InstructionFact* dff_in); //generic flow function for all instructions : returns true if fact was changed
        virtual InstructionFact* join (InstructionFact* inst_a, InstructionFact* inst_b); //join function that internally does what the lattice has defined
        virtual InstructionFact* setBottom(Instruction* inst); //set the instructionFact for instruction to bottom
        virtual InstructionFact* setTop(Instruction* inst); //set instructionfact for instruction to top

    protected:

        DataFlowAnalyzer(){
        }

        //returns instruction type for given instruction
        InstType getInstType(Instruction *inst){

            //PHI instruction - not sure what to do with it !
            if(PHINode *PN = dyn_cast<PHINode>(I)){
                return PHI;
            }

            //Will implement this when we extend to non-binary ops
            if (!inst.isBinaryOp())
                return UNKNOWN;

            //check if one or both of the operands are constants
            User::op_iterator i = inst->op_begin();
            Constant *op1 = dyn_cast<Constant>(*i++);
            Constant *op2 = dyn_cast<Constant>(*i);

            if (!op1 && !op2){
                return X_Eq_Y_Op_Z;
            }
            else if (!op2){
                return X_Eq_Y_Op_C;
            }
            else if (!op1){
                return X_Eq_C_Op_Z;
            }
            else{
                return X_Eq_C_Op_C;
            }
        }

        //populate the worklist queue with all basic blocks at start and inits them to bottom
        void populateBBQueue(Function &F){
            for (Function::iterator BB = F->begin(), E = F->end(); BB != E; BB++){
                bbQueue.push(&*BB);

                for(BasicBlock::iterator I = BB->begin(), IE = BB->end(); I !+ IE; I++){
                    Instruction *inst = &*I;
                    setBottom(inst);
                }
            }
        }

        //The worklist algorithm that runs the flow on the entire procedure
        void runWorkList(Function &F){

            populateBBQueue(F);

            while (!bbQueue.empty()){
                BasicBlock* BB = bbQueue.pop();
                BasicBlock* uniqPrev = NULL;
                InstructionFact* if_in;
                InstructionFact* if_tmp, if_out;
                Instruction* inst;
                bool isBBOutChanged;

                //check if BB has only one predecessor
                if ((uniqPrev = BB->getUniquePredecessor()) != NULL){
                    if_in = getBBInstructionFactOut(uniqPrev);
                }
                //if it has more than one predecessor then join them all
                else {
                    for (auto it = pred_begin(BB), et = pred_end(BB); it != et; ++it)
                    {
                        BasicBlock* prevBB = *it;
                        if_tmp = getBBInstructionFactOut(prevBB);
                        if_in = join (if_in, if_tmp);
                    }
                }

                //run the flow function for each instruction in the basic block
                for (BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; I++){
                    inst = &*I;
                    string opname = inst->getOpcodeName();
                    InstType instType = getInstType(I);
                    isBBOutChanged = flowFunction(I, instType, if_in);

                    if_in = if_out;
                }

                //if the OUT of the last instruction in the bb has changed then add all successors to the worklist
                if (isBBOutChanged){
                    for (auto it = succ_begin(BB), et = succ_end(BB); it != et; ++it)
                    {
                        BasicBlock* succBB = *it;
                        bbQueue.push(succBB);
                    }
                }
            }
        }

        //get the instructionFact for the last instruction in the given basic block
        InstructionFact* getBBInstructionFactOut(BasicBlock *BB){
            Instruction *I;
            I = &*BB->end();
            return dataFlowFactsMap.getInsFact(I);
        }
};

#endif
