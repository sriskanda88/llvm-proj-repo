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

    myDff.printInsFact("Ins1");
    myDff.printInsFact("Ins2");

    // This is what I want to perform:
    // INS_3_OUT = INS_3_IN - { Y -> * } U { W -> 2.3 }

    // Get INS_3_IN ==> INS_2_OUT ins a placeholder
    INS_FACT *insFact = myDff.getTempFact("Ins2");
    // Perform INS_3_IN - { Y-> * }
    (*insFact)["Y"].clear();
    // Perform ( U { W -> 2.3 } )
    (*insFact)["Y"].insert(3.4);

    // Set the modified map back in the structure and check for changes
    bool modified = myDff.setInsFact ("Ins3", insFact);
    if (modified) cout << "Map modified" << endl;
    else cout << "Map NOT modified"<<endl;


    // Get INS_3_IN ==> INS_2_OUT ins a placeholder
    insFact = myDff.getTempFact("Ins2");
    // Perform INS_3_IN - { Y-> * }
    (*insFact)["Y"].clear();
    // Perform ( U { W -> 2.3 } )
    (*insFact)["W"].insert(3.4);

    // Set the modified map back in the structure and check for changes
    modified = myDff.setInsFact ("Ins3", insFact);
    if (modified) cout << "Map modified" << endl;
    else cout << "Map NOT modified"<<endl;

    myDff.printInsFact("Ins1");
    myDff.printInsFact("Ins2");
    myDff.printInsFact("Ins3");

}
