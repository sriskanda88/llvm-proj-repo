#include "DFF.h"
#include <iostream>
using namespace std;

int main () {

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

}
