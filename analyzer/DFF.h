#ifndef _DFF_
#define _DFF_

#include <map>
#include <set>
#include <iostream>
using namespace std;

template <class T>
class DFF {
    private:
	typedef map<string, set<T> > INS_FACT;
	typedef map <string, INS_FACT* > DATA_FLOW_FACT;
	typedef typename map<string, set<T> >::iterator INS_FACT_IT;
	typedef typename set<T>::iterator SET_IT;

	// The data flow fact map structure
	DATA_FLOW_FACT dffMap;

	bool isFirstElement;

	// Prints a set in the form X->5, X->10, ...
	void printSet(string var_id, set<T> temp_set) {
	    for (SET_IT it=temp_set.begin(); it!=temp_set.end(); ++it) {
		if (!isFirstElement) cout <<", ";
		else isFirstElement = false;
		cout << var_id << "->" << *it;
	    }
	}

	// Checks if an instruction exists in the structure
	bool exists( string key) {
	    return dffMap.count(key) != 0;
	} 

    public:
	DFF() {
	    isFirstElement = true;
	};

	INS_FACT* getInsFact(string ins_id) {
	    return dffMap[ins_id];
	}

	// Given an instruction (ins_id), a variable name (var_id) and the 
	// "fact", it adds it in the map structure
	void setVarFact (string ins_id, string var_id, T value) {
	    INS_FACT* temp_fact;
	    if (exists(ins_id)) {
		temp_fact = getInsFact(ins_id);
	    }
	    else {
		temp_fact = new INS_FACT;
		dffMap[ins_id] = temp_fact;
	    }
	    (*temp_fact)[var_id].insert(value);
	}

	// Given an instruction, it prints its facts in the form { X->10, Y->5, ... }
	void printInsFact (string ins_id) {
	    INS_FACT* fact = getInsFact(ins_id); 
	    cout <<ins_id << ": { ";
	    isFirstElement = true;
	    for (INS_FACT_IT it=fact->begin(); it!=fact->end(); ++it) {
		printSet (it->first, it->second);
	    }
	    cout <<" }"<<endl;
	}

	// Given an instruction and a variable, it performs in - { VAR -> * }
	void removeVarFacts (string ins_id, string var_id) {
	    if (!exists(ins_id)) return;
	    INS_FACT* fact = getInsFact(ins_id);
	    (*fact)[var_id].clear();
	}

	// Given an instruction (ins_id) and its predecessor instruction (previous_id)
	// it clones the facts of the previous instruction into the current one
	void clonePrevIns (string ins_id, string previous_id) {
	    INS_FACT* prev_fact = getInsFact(previous_id);
	    INS_FACT* current_fact;
	    if (exists(ins_id)) current_fact = getInsFact(ins_id);
	    else {
		current_fact = new INS_FACT;
		dffMap[ins_id] = current_fact;
	    }
	    for (INS_FACT_IT it=prev_fact->begin(); it!=prev_fact->end(); ++it) {
		string var = it->first;
		set<T> t_set = it->second;
		(*current_fact)[var] = t_set;
	    }
	}
};

#endif
