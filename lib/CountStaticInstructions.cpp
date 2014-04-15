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
  struct SICount : public ModulePass {
    static char ID;
		map <string, int> InstCountMap; 
 
    SICount() : ModulePass(ID) {
		}

    virtual bool runOnModule(Module &M) {

		  for (Module::iterator F = M.begin(), FE = M.end(); F != FE; F++){  		  
				Function* func = &*F;

  	    for (inst_iterator I = inst_begin(func), IE = inst_end(func); I != IE; I++){
					Instruction* inst = &*I;
				  string opname = inst->getOpcodeName();
					InstCountMap[opname]++;
				}
		  }
			return false;
    }

		void print(raw_ostream&	O, const Module* M) const {
	    map <string, int>::const_iterator iter;

		  O << "Static Instruction Counts:\n";
		  for (iter = InstCountMap.begin(); iter != InstCountMap.end(); iter++){
				O << iter->first << " : " << iter->second << "\n";
		  }
    }
  };	
}	

char SICount::ID = 0;

static RegisterPass<SICount> X("static_icount", "Static Instruction Count", false, false);
