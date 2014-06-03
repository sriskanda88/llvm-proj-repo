#ifndef _DFF_
#define _DFF_

#include <map>
#include <set>
#include <iostream>
#include "llvm/IR/Instruction.h"
using namespace std;
using namespace llvm;

template <class T>
class DFF {
    private:
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
		if (!isFirstElement) cout <<", ";
		else isFirstElement = false;
		cout << var_id<< "->" << *it;
	    }
	}

	// Checks if an instruction exists in the structure
	bool exists( Instruction* key) {
	    return dffMap.count(key) != 0;
	}

        // Returns true if the two maps are identical
        bool compareMaps (InstFact* map1, InstFact* map2) {
            if (map1->size() != map2->size()) return false;
            InstFact_IT it1, it2;
            for (it1=map1->begin(), it2=map2->begin(); it1!=map1->end(); ++it1, ++it2) {
                if (*it1 != *it2) return false;
            }
            return true;
        } 

    public:
	DFF() {
	    isFirstElement = true;
	};

	InstFact* getInsFact(Instruction* ins_id) {
            if (!exists(ins_id)) dffMap[ins_id] = new InstFact;
            return dffMap[ins_id];
	}

	// Given an instruction (ins_id), a variable name (var_id) and the 
	// "fact", it adds it in the map structure
	void setVarFact (Instruction* ins_id, string var_id, T value) {

            typeDescription[ins_id] = REGULAR;

	    InstFact* temp_fact;
	    if (exists(ins_id)) {
		temp_fact = getInsFact(ins_id);
	    }
	    else {
		temp_fact = new InstFact;
		dffMap[ins_id] = temp_fact;
	    }
	    (*temp_fact)[var_id].insert(value);
	}

	// Given an instruction, it prints its facts in the form { X->10, Y->5, ... }
	void printInsFact (Instruction* ins_id) {
            if (typeDescription[ins_id] == FULL) {
                cout <<ins_id << ": FULL SET"<<endl;
                return;
            }
            if (typeDescription[ins_id] == EMPTY) {
                cout <<ins_id << ": EMPTY SET"<<endl;
                return;
            }
	    InstFact* fact = getInsFact(ins_id); 
            ins_id->dump();
	    cout <<"    "<<ins_id << ": { ";
	    isFirstElement = true;
	    for (InstFact_IT it=fact->begin(); it!=fact->end(); ++it) {
		printSet (it->first, it->second);
	    }
	    cout <<" }"<<endl;
	}
    
        void printInsFact (InstFact* fact) {
            cout <<": { ";
            isFirstElement = true;
            for (InstFact_IT it=fact->begin(); it!=fact->end(); ++it) {
                printSet (it->first, it->second);
            }
            cout <<" }"<<endl;
        }

        void printEverything () {
            for (typename map<Instruction*, InstFact*>::iterator it = dffMap.begin(); it!=dffMap.end(); ++it) {
                printInsFact(it->first);
            }
        }

	// Given an instruction and a variable, it performs in - { VAR -> * }
	void removeVarFacts (Instruction* ins_id, string var_id) {
	    if (!exists(ins_id)) return;
	    InstFact* fact = getInsFact(ins_id);
            fact->erase(var_id);
            if (fact->size() == 0) typeDescription[ins_id] = EMPTY;
	}

	// Given an instruction (ins_id) and its predecessor instruction (previous_id)
	// it clones the facts of the previous instruction into the current one
	void clonePrevIns (Instruction* ins_id, Instruction* previous_id) {
            
            typeDescription[ins_id] = typeDescription[previous_id];

	    InstFact* prev_fact = getInsFact(previous_id);
	    InstFact* current_fact;
	    if (exists(ins_id)) current_fact = getInsFact(ins_id);
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
        bool setInsFact (Instruction* ins_id, InstFact* new_fact) {
            typeDescription[ins_id] = REGULAR;
            InstFact* prev_fact;
            if (exists(ins_id)) {
                prev_fact = getInsFact(ins_id);
                dffMap[ins_id] = new_fact;
                return (!(compareMaps(prev_fact, new_fact)));
            }
            else {
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
 
};

#endif






