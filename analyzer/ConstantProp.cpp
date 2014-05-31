#include "Analyzer.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <map>

using namespace llvm;
using namespace std;

namespace {
    struct ConstantProp : public FunctionPass, public DataFlowAnalyzer<int>
    {
        static char ID;
        
        ConstantProp() : FunctionPass(ID) {}
        
        virtual bool runOnFunction(Function &F) {

            //call the worklist algorithm
            this->runWorkList(F);

            return true;
        }

        // now we will override the print to simply display every entry (and it's mapping) in DFF
        void print(raw_ostream&	O, const Module* M) const {
            O << "TODO: for each entry in the DFF print it and its mapping     \n";
        }

        bool flowFunction(Instruction* inst, InstType instType, InstFact* dff_in)
        {   
            return false; 
        }   

        InstFact* join (InstFact* a, InstFact* b)
        {   
            return dataFlowFactsMap.intersectFacts(a,b);
        }   

        void setBottom(Instruction* inst)
        {   
            dataFlowFactsMap.setFactFullSet("tmp");
            //dataFlowFactsMap.setFactFullSet(inst);
        }   

        void setTop(Instruction* inst)
        {  
            dataFlowFactsMap.setFactEmptySet("tmp");
            //dataFlowFactsMap.setFactEmptySet(inst);
        }

    };
}

char ConstantProp::ID = 0;

static RegisterPass<ConstantProp> X("constant_prop", "Propigate constant values", false, false);
