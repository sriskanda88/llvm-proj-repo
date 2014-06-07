#ifndef _DFF_
#define _DFF_

#include "llvm/Pass.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CFG.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include <string>
#include <map>
#include <queue>
#include <list>
#include <set>

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FEnv.h"
#include "llvm/Support/MathExtras.h"


#include <map>
#include <set>
#include <iostream>
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
using namespace std;
using namespace llvm;

template <class T>
class DFF {
    public:
    typedef map<string, set<T> > InstFact;
    typedef map <Instruction*, InstFact* > DATA_FLOW_FACT;
    typedef typename map<string, set<T> >::iterator InstFact_IT;
    typedef typename set<T>::iterator SET_IT;

    enum setType { FULL, EMPTY, REGULAR };

    // The data flow fact map structure
    DATA_FLOW_FACT dffMap;
    map <Instruction*, setType> typeDescription;

    bool isFirstElement;

    // Prints a set in the form X->5, X->10, ...
    void printSet(string var_id, set<T> temp_set) {
        for (SET_IT it=temp_set.begin(); it!=temp_set.end(); ++it) {
            if (!isFirstElement)
            {
                cout <<", ";
                errs() << ", ";
            } else {
                isFirstElement = false;
            }
            cout << var_id << "->" << *it;
            errs() << var_id << "->" << *it;
        }
    }

    // Checks if an instruction exists in the structure
    bool exists(Instruction* key) {
        return dffMap.count(key) != 0;
    }

    // Returns true if the two maps are identical
    bool compareMaps (InstFact* map1, InstFact* map2) {
        if (map1->size() != map2->size()) 
            return false;
        InstFact_IT it1, it2;
        for (it1=map1->begin(), it2=map2->begin(); it1!=map1->end(); ++it1, ++it2) {
            if (*it1 != *it2) 
                return false;
        }
        return true;
    } 

    public:
    DFF() {
        isFirstElement = true;
    };

    InstFact* getInsFact(Instruction* ins_id) {
        if(!exists(ins_id))
            dffMap[ins_id] = new InstFact;

        return dffMap[ins_id];
    }

    // check to see if variable exists within the given instruction fact passed in
    static bool exists(InstFact* if_in, string key)
    {
        return if_in->count(key) != 0;
    }



    // Given an instruction (ins_id), a variable name (var_id) and the 
    // "fact", it adds it in the map structure
    void setVarFact (Instruction* ins_id, string var_id, T value) {

        typeDescription[ins_id] = REGULAR;

        InstFact* temp_fact;
        if (exists(ins_id)) {
            temp_fact = getInsFact(ins_id);
        } else {
            temp_fact = new InstFact;
            dffMap[ins_id] = temp_fact;
        }

        (*temp_fact)[var_id].insert(value);
    }

    // same as above, but just for a temp instruction fact, not a full DFF
    static void setVarFact(InstFact* in_if, string var_id, T value)
    {
        if( (*in_if).count(var_id) == 0)
        {
            errs() << "Doesn't exist... we need to add it in!\n";
            std::set<T> tempSet;
            tempSet.insert(value);

            errs() << "created temp set..\n";

            (*in_if).insert(std::pair<string,set<T> >(var_id, tempSet));
            errs() << "Done inserting into map.\n";
        } else {
            (*in_if)[var_id].insert(value);
        }
        //TODO: handle empty/full sets...
    }


    // Given an instruction, it prints its facts in the form { X->10, Y->5, ... }
    void printInsFact (Instruction* ins_id) {
        if (typeDescription[ins_id] == FULL) {
            cout <<ins_id << ": FULL SET"<<endl;
            errs() << "fullset\n";
            return;
        }
        
        if (typeDescription[ins_id] == EMPTY) {
           cout <<ins_id << ": EMPTY SET"<<endl;
           errs() << "empty set\n";
           return;
        }
        
        InstFact* fact = getInsFact(ins_id); 
        errs() << ins_id << ": { ";
        cout <<ins_id << ": { ";
        isFirstElement = true;
        for (InstFact_IT it=fact->begin(); it!=fact->end(); ++it) {
            printSet (it->first, it->second);
        }
        cout <<" }"<<endl;
        errs() << " }\n";
    }

    // Given an instruction and a variable, it performs in - { VAR -> * }
    void removeVarFacts (Instruction* ins_id, string var_id) {
        if (!exists(ins_id)) 
            return;
        InstFact* fact = getInsFact(ins_id);
        fact->erase(var_id);
        
        if (fact->size() == 0) 
            typeDescription[ins_id] = EMPTY;
    }

    // same as above, just for a temp instruction fact, not a full DFF
    static void removeVarFacts(InstFact* in_if, string var_id)
    {
        in_if->erase(var_id);
        //TODO: handle empty/full sets...
    }

    // Given an instruction (ins_id) and its predecessor instruction (previous_id)
    // it clones the facts of the previous instruction into the current one
    void clonePrevIns (Instruction* ins_id, string previous_id) {
            
        typeDescription[ins_id] = typeDescription[previous_id];

        InstFact* prev_fact = getInsFact(previous_id);
        InstFact* current_fact;
        
        if (exists(ins_id)) 
            current_fact = getInsFact(ins_id);
        else {
            current_fact = new InstFact;
            dffMap[ins_id] = current_fact;
        }
       
        for (InstFact_IT it=prev_fact->begin(); it!=prev_fact->end(); ++it) {
            string var = it->first;
            set<T> t_set = it->second;
            (*current_fact)[var] = t_set;
        }
    }

    // This function sets facts at the instruction level.
    // It also compares the previous value with the new one
    // so we know if anything changed. (True means something 
    // changed)
    bool setInsFact (Instruction* ins_id, InstFact* new_fact) 
    {
        typeDescription[ins_id] = REGULAR;
        InstFact* prev_fact;
        if (exists(ins_id)) {
            prev_fact = getInsFact(ins_id);
            dffMap[ins_id] = new_fact;
            return (!(compareMaps(prev_fact, new_fact)));
        } else {
            dffMap[ins_id] = new_fact;
            return true;
        }    
    }

    // Clones the previous instruction into a placeholder and returns it. 
    // Does not modify the map structure. We can modify the returned map 
    // and then set it using the setInsFact function
    InstFact* getTempFact (Instruction* prev_id) {
        InstFact* prev_fact = getInsFact(prev_id);
        InstFact* temp_fact = new InstFact;
        for (InstFact_IT it=prev_fact->begin(); it!=prev_fact->end(); ++it) {
            string var = it->first;
            set<T> t_set = it->second;
            (*temp_fact)[var] = t_set;
        }
        return temp_fact;
    }

    // Sets a fact to the full set (WARNING: Deletes the set's contents)
    void setFactFullSet (Instruction* ins_id) {
        if (exists(ins_id)) dffMap[ins_id]->clear();
        else {
            InstFact* temp_fact = new InstFact;
            dffMap[ins_id] = temp_fact;
        }
        typeDescription[ins_id] = FULL;
    }

    // Sets a fact to the empty set (WARNING: Deletes the set's contents)
    void setFactEmptySet (Instruction* ins_id) {
        if (exists(ins_id)) dffMap[ins_id]->clear();
        else {
            InstFact* temp_fact = new InstFact;
            dffMap[ins_id] = temp_fact;
        }
        typeDescription[ins_id] = EMPTY;
    }

    // Returns a fact's type. Normally, we shouldn't need to ever call this function.
    int getFactType (Instruction* ins_id) {
        return typeDescription[ins_id];
    }


    // Gets two instruction identifiers and unions them.
    // I cannot check if a set is full. The caller needs to check and abort 
    // if at least one of the instruction facts are the full set.
    InstFact* unionFacts (InstFact* map1, InstFact* map2) {
        //get all the keys from both maps into a set (Avoids duplicates)
        set<string> allKeys;
        for (InstFact_IT it=map1->begin(); it!=map1->end(); ++it) {
            string var = it->first;
            allKeys.insert(var);
        }
        for (InstFact_IT it=map2->begin(); it!=map2->end(); ++it) {
            string var = it->first;
            allKeys.insert(var);
        }

        //Begin merging
        InstFact* result = new InstFact;
        for (typename set<string>::iterator it=allKeys.begin(); it!=allKeys.end(); ++it) { 
            set<T> set1, set2;
            set1 = (*map1)[*it];
            set2 = (*map2)[*it];
            (*result)[*it].insert(set1.begin(), set1.end());
            (*result)[*it].insert(set2.begin(), set2.end());
        }

        return result;
    }     

    // Gets two instruction identifiers and intersects them.
    // I cannot check if a set is empty. The caller needs to check and abort
    // if at least one of the instruction facts are the full set.
    InstFact* intersectFacts (InstFact* map1, InstFact* map2) {
        //get all the keys from both maps into a set (Avoids duplicates)
        set<string> allKeys;
        for (InstFact_IT it=map1->begin(); it!=map1->end(); ++it) {
            string var = it->first;
            allKeys.insert(var);
        }
        for (InstFact_IT it=map2->begin(); it!=map2->end(); ++it) {
            string var = it->first;
            allKeys.insert(var);
        }
        
        InstFact* result = new InstFact;
        for (typename set<string>::iterator it=allKeys.begin(); it!=allKeys.end(); ++it) {
            set<T> set1, set2;
            set1 = (*map1)[*it];
            set2 = (*map2)[*it];
            set<T> resultSet;

            //Begin intersection
            for (typename set<T>::iterator l_it=set1.begin(); l_it!=set1.end(); ++l_it) {
                if (set2.count(*l_it) != 0) resultSet.insert(*l_it);
            }
            for (typename set<T>::iterator l_it=set2.begin(); l_it!=set2.end(); ++l_it) {
                if (set1.count(*l_it) != 0) resultSet.insert(*l_it);
            }
            (*result)[*it] = resultSet;
        }
        return result;
    }

    bool isFullSet(Instruction* inst) {
        return (typeDescription[inst] == FULL);
    }   

    bool isEmptySet(Instruction* inst) {
        return (typeDescription[inst] == EMPTY);
    }   

};

#endif

#include "llvm/Pass.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CFG.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include <string>
#include <map>
#include <queue>
#include <list>
#include <set>

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FEnv.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;
using namespace std;

//enum to represent different instruction formats
enum InstType {X_Eq_C, X_Eq_C_Op_Z, X_Eq_Y_Op_C, X_Eq_C_Op_C, X_Eq_Y_Op_Z, X_Eq_Y, X_Eq_Addr_Y, X_Eq_Star_Y, Star_X_Eq_Y, PHI, UNKNOWN};

namespace{
    struct ConstantProp : public FunctionPass
    {
        typedef map<string, set<int> > InstFact; //Instruction level data fact
//        std::queue<BasicBlock *, std::list<BasicBlock *> > bbQueue; //Worklist queue
        set<BasicBlock*> bbQueue;
        DFF<int> dataFlowFactsMap; //DataFlowFact map for all instructions

        static char ID;
        
        ConstantProp() : FunctionPass(ID) {}
        
        virtual bool runOnFunction(Function &F) {

            //call the worklist algorithm
            this->runWorkList(F);

            return true;
        }

        // now we will override the print to simply display every entry (and it's mapping) in DFF
        void print(raw_ostream&    O, const Module* M) const {
            O << "TODO: for each entry in the DFF print it and its mapping     \n";
        }

        // returns true if the flow function was changed
        bool flowFunction(Instruction* inst, InstFact* if_in)
        {  
            //this will be the value we will push
            Constant *C = NULL;
            // the first thing we need to do is check to see if it's a PHI node
            if(PHINode *PN = dyn_cast<PHINode>(inst))
            {
                for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
                    Value *Incoming = PN->getIncomingValue(i);
                    // If the incoming value is undef then skip it.  Note that while we could
                    // skip the value if it is equal to the phi node itself we choose not to
                    // because that would break the rule that constant folding only applies if
                    // all operands are constants.
                    if (isa<UndefValue>(Incoming))
                        continue;
                    // If the incoming value is not a constant, then give up.
                    Constant *C2 = dyn_cast<Constant>(Incoming);
                    if (!C)
                        return false;
                    // Fold the PHI's operands.
                    if (ConstantExpr *NewC = dyn_cast<ConstantExpr>(C2))
                    {
                        errs() << "######### TODO: Got a constant expression, the vallue is: " << NewC << ". Will we handle this case???\n";
                        //C = ConstantFoldConstantExpression(NewC, TD, TLI);
                    }

                    // If the incoming value is a different constant to
                    // the one we saw previously, then give up.
                    if (C2 && C != C2)
                        return false;
                    C = C2;
                }
            } else {

                // the first thing we need to check is to see if all operands are constants
                SmallVector<Constant*, 8> Ops;
                  
                for (User::op_iterator i = inst->op_begin(), e = inst->op_end(); i != e; ++i) 
                {
                    //errs() << "### @for loop.\n";
                    ConstantInt *Op = dyn_cast<ConstantInt>(*i);
                    if (!Op)
                    {
                        //errs() << "the op wasn't a constant..\n";
                        // this op isn't a constant, but we might have a mapping for it to a constant!
                        string opVarID = (string) (*i)->getName();
                        //errs() << " the name of this op is: " << opVarID << ".\n";
                        if(DFF<int>::exists(if_in, opVarID))
                        {
                            errs() << opVarID << " exists in the instruction fact, so let's check if it has a value.\n";
                            if( (*if_in)[opVarID].size() == 1 )
                            {
                                // get the integer value
                                DFF<int>::SET_IT it = (*if_in)[opVarID].begin();
                                int val = *it;

                                // cast it as an unsigned value
                                unsigned NumBits = 32;
                                uint64_t uVal = (uint64_t) val;

                                // get the type information needed for LLVM then cast it as a constant int
                                IntegerType* intType = IntegerType::get(getGlobalContext(), NumBits);
                                ConstantInt* newConst = ConstantInt::get(llvm::Type::getInt32Ty(getGlobalContext()), uVal, true);

                                //Constant* myConst = dyn_cast<Constant>(newConst);
                                //Type myType = llvm::Type::Type(getGlobalContext(), llvm::Type::IntegerTyID);
                                //Constant* myConst = llvm::ConstantInt::get(myType, newConst->getValue());

                                errs() << "    This item was in the set, it's value: " << val << "\n";
                                Op = newConst; //TODO: make this so it actually assigns the correct operand value
                            } else {
                                errs() << "ERROR: We don't have any info for this instruction, but it exists in the map - ignore/weird case.\n";
                                return false; 
                            }
                        } else
                        {                     
                            return false; // this op isn't constant, and it's not in our map - so there's nothing we can do with it. Ignore it.
                        }
                    } else {
                        // This was a constant int, but we need to check if it was a Constant Expression
                        if (ConstantExpr *NewCE = dyn_cast<ConstantExpr>(Op))
                        {
                            errs() << "######### TODO: Got a constant expression, the vallue is: " << NewCE << ". Will we handle this case???\n";
                            return false; //TODO look at next line
                            //Op = constantFold(NewCE, DL, TLI); // this is what the original code does.. 
                        }
                    }
                    // now that we've set Op in above code, place it into the operand array
                    Ops.push_back(Op);
                }

                //errs() << "done with for loop.\n";

                // now that we have all constants, we will merge them and update the DFF         
                // but we will only do it for certain kinds of instructions
                if (dyn_cast<CmpInst>(inst) || dyn_cast<LoadInst>(inst) || dyn_cast<InsertValueInst>(inst) || dyn_cast<ExtractValueInst>(inst)) 
                {
                  return false;
                }
                            
                // at this point all incoming values are the same and constant, 
                // so look up the op and perform the constant propigation
                unsigned Opcode = inst->getOpcode();
                //Type *DestTy = inst->getType();

                //errs() << "check for binop\n";
                //first check if it's a binop
                if(Instruction::isBinaryOp(Opcode))
                {
                    //errs() << "   this is a binop!\n";
                    if (isa<ConstantExpr>(Ops[0]) || isa<ConstantExpr>(Ops[1])) 
                    {
                        //if (C = llvm::SymbolicallyEvaluateBinop(Opcode, Ops[0], Ops[1], TD))
                            errs() << "TODO: ERROR: C is:= " << C << "\n";
                    }

                    C = ConstantExpr::get(Opcode, Ops[0], Ops[1]);
                    //errs() << "    which evaluated to: " << *C << ".\n";
                } else {
                    switch(Opcode)
                    {
                        errs() << "TODO: implement this!\n";
                        default: return false;
                    }
                }
            }

            // now we insert the constant into the instruction fact
            // but first we need to look up what variable it is in the map!
            string varID = (string) inst->getName();
            //errs() << "got the iname for the instruction, it's: " << varID << ".\n";

            //now extract the value from the Constant type
            ConstantInt* constInt = dyn_cast<ConstantInt>(C);
            int inputInt = (int) constInt->getSExtValue(); 

            // now that we know the variable we kill the original
            if(DFF<int>::exists(if_in, varID))
            {
                errs() << "ERROR: there was already a mapping in the DFF, so we will remove it. NOTE: In SSA this should never happen!\n";
                //DFF<int>::removeVarFacts(if_in, varID);
                (*if_in)[varID].erase((*if_in)[varID].begin(), (*if_in)[varID].end());
            }
            
            // and insert it into the temp map
            (*if_in)[varID].insert(inputInt);

            // finally place this into the DFF and check if it differs from the previous 
            return dataFlowFactsMap.setInsFact(inst, if_in);
        }

        InstFact* join (InstFact* a, InstFact* b)
        {   
            return dataFlowFactsMap.intersectFacts(a,b);
        }   

        void setBottom(Instruction* inst)
        {   
            dataFlowFactsMap.setFactFullSet(inst);
        }   

        void setTop(Instruction* inst)
        {  
            dataFlowFactsMap.setFactEmptySet(inst);
        }
        
        //populate the worklist queue with all basic blocks at start and inits them to bottom
        void populateBBQueue(Function &F){
            for (Function::iterator BB = F.begin(), E = F.end(); BB != E; BB++){
                //bbQueue.push(&*BB);
                bbQueue.insert(BB);

                for(BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; I++){
                    Instruction *inst = &*I;
                    setBottom(inst);
                }
            }
        }

        InstFact* joinAllPredecessors(set<Instruction*> predecessors) {
            InstFact* result = new InstFact;
            //Check if any one is the empty set and return empty set
            bool first = true;
            //errs()<<"Set has size: "<<predecessors.size()<<"\n";
            for (set<Instruction*>::iterator it=predecessors.begin(); it!=predecessors.end(); ++it) {
                if (first) {
                    //errs()<<"Cloning first element: "<<*it<<"\n";
                    result = dataFlowFactsMap.getTempFact(*it);
                    first = false;
                }   
                else {
                    //errs() <<"Joining instruction "<<*it<<"\n";
                    InstFact* if_tmp = dataFlowFactsMap.getTempFact(*it);
                    result = join(result,if_tmp); 
                }   
            }   
            return result;

            //copy the first element
            //loop through the rest elements
        } 


        //The worklist algorithm that runs the flow on the entire procedure
        void runWorkList(Function &F){

            populateBBQueue(F);

            while (!bbQueue.empty()){
                //BasicBlock* BB = bbQueue.front();
                //bbQueue.pop();
                set<BasicBlock*>::iterator BB_it = bbQueue.begin();
                BasicBlock* BB = *BB_it;
                bbQueue.erase(BB_it);

                BasicBlock* uniqPrev = NULL;
                InstFact* if_in = new InstFact;
                Instruction* inst;
                bool isBBOutChanged;

                //check if BB has only one predecessor
                if ((uniqPrev = BB->getUniquePredecessor()) != NULL){
                    //errs()<<"UNIQUE "<<(string)uniqPrev->getName()<<"\n";
                    if_in = getBBInstFactOut(uniqPrev);
                }
                //if it has more than one predecessor then join them all
                else {
                    set<Instruction*> predecessors;
                    //errs()<<"MULTIPLE\n";
                    for (pred_iterator it = pred_begin(BB), et = pred_end(BB); it != et; ++it)
                    {
                        BasicBlock* prevBB = *it;
                        //errs()<<"PREDECESSOR: "<<(string)prevBB->getName()<<"\n";
                        Instruction* in_temp = prevBB->getTerminator();
                        if (dataFlowFactsMap.isFullSet(in_temp)) continue;
                        predecessors.insert(in_temp);
                    }
                    if_in = joinAllPredecessors(predecessors);
                }       
               
                //run the flow function for each instruction in the basic block
                for (BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; I++){

                    inst = &*I;
                    string opname = inst->getOpcodeName();
                    isBBOutChanged = flowFunction(I, if_in);

                    if_in = dataFlowFactsMap.getTempFact(inst);
                    //if_in = dataFlowFactsMap.getInsFact(I);
                }

                //if the OUT of the last instruction in the bb has changed then add all successors to the worklist
                if (isBBOutChanged){
                    //errs()<<"Things changed. Scheduling successors\n";
                    for (succ_iterator it = succ_begin(BB), et = succ_end(BB); it != et; ++it)
                    {
                        BasicBlock* succBB = *it;
                        //errs()<<"Successor: "<<succBB->getName()<<"\n";
                        //bbQueue.push(succBB);
                        bbQueue.insert(succBB);
                    }
                }
            }
        }

        //get the instructionFact for the last instruction in the given basic block
        InstFact* getBBInstFactOut(BasicBlock *BB){
            Instruction *I;
            I = BB->getTerminator();

            //errs()<<"Terminator: "<<*I<<"\n";

            //return dataFlowFactsMap.getInsFact(NULL);
            //return dataFlowFactsMap.getInsFact(I);
            return dataFlowFactsMap.getTempFact(I);
        }
        

 /*       //The worklist algorithm that runs the flow on the entire procedure
        void runWorkList(Function &F){

            populateBBQueue(F);

            while (!bbQueue.empty()){
                //errs() << "popping off the front of the BB queue\n";
                BasicBlock* BB = bbQueue.front();
                bbQueue.pop();
                //errs() << "popped off the front of the BB queue\n";
                BasicBlock* uniqPrev = NULL;
                InstFact* if_in = new InstFact;
                InstFact* if_tmp;
                Instruction* inst;
                bool isBBOutChanged;

                //errs() << "check to see if only one predecessor\n";
                //check if BB has only one predecessor
                if ((uniqPrev = BB->getUniquePredecessor()) != NULL){
                    if_in = getBBInstFactOut(uniqPrev);
                }
                //if it has more than one predecessor then join them all
                else {
                    for (pred_iterator it = pred_begin(BB), et = pred_end(BB); it != et; ++it)
                    {
                        BasicBlock* prevBB = *it;
                        if_tmp = getBBInstFactOut(prevBB);
                        
                        //errs() << "*** We are joining the instruction facts!\n";
                        if_in = join (if_in, if_tmp);
                    }
                }

                //errs() << "done checking for number of predecessors\n";
                //run the flow function for each instruction in the basic block
                for (BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; I++){
                    inst = &*I;
                    string opname = inst->getOpcodeName();
                    //errs() << "applying the flow function!!!\n";
                    isBBOutChanged = flowFunction(I, if_in);
                    //errs() << "done applying the flow function!!!\n";

                    if_in = dataFlowFactsMap.getTempFact(inst);
                    //if_in = dataFlowFactsMap.getInsFact((string) I->getName());

                    //errs() << "\nPrinting out some usefull information.\n";
                    //dataFlowFactsMap.printInsFact(inst);
                    //errs() << "Done printing usefull info..\n\n\n";
                }

                //if the OUT of the last instruction in the bb has changed then add all successors to the worklist
                if (isBBOutChanged){
                    //errs() << "the BB out was different than the in, adding it back to the queue.\n";
                    for (succ_iterator it = succ_begin(BB), et = succ_end(BB); it != et; ++it)
                    {
                        BasicBlock* succBB = *it;
                        bbQueue.push(succBB);
                    }
                }
            }
        }

        //get the instructionFact for the last instruction in the given basic block
        InstFact* getBBInstFactOut(BasicBlock *BB){
            Instruction *I;
            I = &*BB->end();

            //return dataFlowFactsMap.getInsFact(I);
            return dataFlowFactsMap.getTempFact(I);
        }
*/
    };
}

char ConstantProp::ID = 0;

static RegisterPass<ConstantProp> X("combined_constant", "Propigate constant values", false, false);
