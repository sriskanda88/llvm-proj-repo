#include <map>
#include <iostream>
#include <cstdio>
using namespace std;

map <string, int> functionMapping;
map <string, int> functionTotal;

map <string, int> dynamicInstructionCountMap;

void branchTaken (char* functionName) {
    string temp(functionName);
    if (functionMapping.count(temp) == 0) {
        functionMapping[temp] = 1;
    }
    else {
        functionMapping[temp]++;
    }
}

void foundBranch (char* functionName) {
    //Count the total branches
    string temp (functionName);
    if (functionTotal.count(temp) == 0) {
        functionTotal[temp] = 1;
    }
    else {
        functionTotal[temp]++;
    }
}

void printStatistics () {

        cout<<endl<<"STATISTICS:"<<endl<<"-----------"<<endl<<endl;
        // No branches in benchmark
        if (functionTotal.size() == 0) {
            printf ("%15s %5s %5s %5s\n", "FUNCTION", "BIAS", "TAKEN", "TOTAL");
            printf ("%15s %5s %5d %5d\n", "main", "NaN", 0, 0);
            return;
        }

        printf ("%15s %5s %5s %5s\n", "FUNCTION", "BIAS", "TAKEN", "TOTAL");
    for (map<string,int>::iterator it=functionMapping.begin(); it!=functionMapping.end(); ++it) {
        string funcName = it->first;
        int taken = it->second;
        int total = functionTotal[funcName];
        printf ("%15s %3.2f %5d %5d\n", funcName.c_str(), ((double)taken / total) * 100, taken, total);
    }
}

void basicBlockInstructionCount(char *instType, int count){
    string temp (instType);
    if (dynamicInstructionCountMap.count(temp)== 0){
        dynamicInstructionCountMap[temp] = 1;
    }
    else{
        dynamicInstructionCountMap[temp]++;
    }
}

void printDynamicInstCountStatistics () {
    cout <<endl<<"DYNAMIC INSTRUCTION COUNT STATICTICS"<<endl<<"-----------"<<endl<<endl;

    printf ("%15s %5s\n", "INST TYPE", "COUNT");
    for (map<string, int>::iterator it = dynamicInstructionCountMap.begin(); it != dynamicInstructionCountMap.end(); it++){
        string instType = it->first;
        int count = it->second;
        printf ("%15s %5d\n", instType.c_str(), count);
    }

    cout<<endl<<"-----------"<<endl<<endl;
}
