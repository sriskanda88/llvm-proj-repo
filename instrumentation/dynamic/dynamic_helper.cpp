#include <map>
#include <iostream>
#include <cstdio>
using namespace std;

map <string, int> dynamicInstructionCountMap;

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
    int total = 0;

    for (map<string, int>::iterator it = dynamicInstructionCountMap.begin(); it != dynamicInstructionCountMap.end(); it++){
        string instType = it->first;
        int count = it->second;
        total += count;
        fprintf (stderr, "%-15s %5d\n", instType.c_str(), count);
    }

    fprintf (stderr, "%-15s %5d\n", "TOTAL", total);
}
