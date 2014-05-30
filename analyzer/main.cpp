#include "DFF.h"
#include <iostream>
using namespace std;

int main () {

    typedef map<string, set<float> > INS_FACT;

    DFF<float> myDff;
    myDff.setVarFact ("Ins1", "X", 1.2);
    myDff.setVarFact ("Ins1", "X", 2.3);
    myDff.setVarFact ("Ins1", "Y", 3.4);

    myDff.clonePrevIns ("Ins2", "Ins1");
    myDff.setVarFact ("Ins2", "Z", 4.2);
    myDff.setVarFact ("Ins2", "Z", 5.3);
    myDff.setVarFact ("Ins2", "Z", 6.4);

    myDff.removeVarFacts ("Ins2", "Y");

    myDff.printInsFact("Ins1");
    myDff.printInsFact("Ins2");

    // This is what I want to perform:
    // INS_3_OUT = INS_3_IN - { Y -> * } U { W -> 2.3 }

    // Get INS_3_IN ==> INS_2_OUT in a placeholder
    INS_FACT *insFact = myDff.getTempFact("Ins2");
    // Perform INS_3_IN - { Y-> * }
    (*insFact)["Y"].clear();
    // Perform ( U { W -> 2.3 } )
    (*insFact)["W"].insert(2.3);

    // Set the modified map back in the structure and check for changes
    bool modified = myDff.setInsFact ("Ins3", insFact);
    if (modified) cout << "Map modified" << endl;
    else cout << "Map NOT modified"<<endl;

    myDff.setFactEmptySet("Ins1");
    myDff.setFactFullSet("Ins2");


    myDff.printInsFact("Ins1");
    myDff.printInsFact("Ins2");
    myDff.printInsFact("Ins3");



    myDff.setVarFact ("Ins4", "X", 1);
    myDff.setVarFact ("Ins4", "X", 2);
    myDff.setVarFact ("Ins4", "Y", 4);

    myDff.setVarFact ("Ins5", "X", 3);
    myDff.setVarFact ("Ins5", "Y", 5);
    myDff.setVarFact ("Ins5", "Y", 6);
    myDff.setVarFact ("Ins5", "Z", 1);

    INS_FACT* ins4 = myDff.getInsFact("Ins4");
    INS_FACT* ins5 = myDff.getInsFact("Ins5");
    INS_FACT* ins6 = myDff.intersectFacts(ins4, ins5);
    myDff.setInsFact ("Ins6", ins6);

    myDff.printInsFact("Ins4");
    myDff.printInsFact("Ins5");
    myDff.printInsFact("Ins6");

}
