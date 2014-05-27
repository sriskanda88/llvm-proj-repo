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

enum InstType {X_Op_Y, X_Eq_Y, X_Eq_Addr_Y, X_Eq_Star_Y, Star_X_Eq_Y};

template <class T>
class DataFlowAnalyzer {

    protected:
        typedef map<string, set<T> > DataFlowFact;
        std::queue<BasicBlock *, std::list<BasicBlock *> > bbQueue;
        DFF<T> dataFlowFactsMap;

        virtual void initializeLattice();
        virtual DataFlowFact* flowFunction(Instruction* inst, InstType instType, DataFlowFact* dff_in);
        virtual DataFlowFact* join (DataFlowFact* a, DataFlowFact* b);
        virtual DataFlowFact* getBottom();
        virtual DataFlowFact* getTop();

    protected:

        DataFlowAnalyzer(){
        }

        InstType getInstType(Instruction *inst){
            return X_Op_Y;
        }


        void populateBBQueue(Module &M){
            for (Module::iterator F = M.begin(), FE = M.end(); F != FE; F++){
                for (Function::iterator BB = F->begin(), E = F->end(); BB != E; BB++){
                    bbQueue.push(&*BB);
                }
            }
        }

        void runWorkList(){

            while (!bbQueue.empty()){
                BasicBlock* BB = bbQueue.pop();
                BasicBlock* uniqPrev = NULL;
                DataFlowFact* dff_in = getBottom(); //TODO
                DataFlowFact* dff_tmp, dff_bb_in, dff_out;
                Instruction* inst;

                if ((uniqPrev = BB->getUniquePredecessor()) != NULL){
                    dff_in = getBBDataFlowFactOut(uniqPrev);
                }
                else {
                    for (auto it = pred_begin(BB), et = pred_end(BB); it != et; ++it)
                    {
                        BasicBlock* prevBB = *it;
                        dff_tmp = getBBDataFlowFactOut(prevBB);
                        dff_in = join (dff_in, prevBB);
                    }
                }

                dff_bb_in = dff_in;

                for (BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; I++){
                    inst = &*I;
                    string opname = inst->getOpcodeName();
                    InstType instType = getInstType(I);
                    dff_out = flowFunction(I, instType, dff_in);

                    dataFlowFactsMap.set(I, dff_out);
                    dff_in = dff_out;
                }

                if (!isDataFlowFactEqual(dff_out, dff_bb_in)){
                    for (auto it = succ_begin(BB), et = succ_end(BB); it != et; ++it)
                    {
                        BasicBlock* succBB = *it;
                        bbQueue.push(succBB);
                    }
                }
            }
        }

        DataFlowFact* getBBDataFlowFactOut(BasicBlock *BB){
            Instruction *I;
            I = &*BB->end();
            return dataFlowFactsMap.getInsFact(I);
        }
};

#endif
