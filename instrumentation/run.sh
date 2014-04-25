#!/usr/bin/env bash

pass=$1

if [ $pass == "" ]; then
    echo "Please provide pass name as argument [static_icount, dynamic_icount, BranchBias]"
    exit
elif [ $pass != "static_icount" ] && [ $pass != "dynamic_icount" ] && [ $pass != "BranchBias" ]; then
    echo "Valid pass names are : static_icount, dynamic_icount, BranchBias"
    exit
fi


# The script must be in $BENCHMARKS folder. There has to exist a folder 
# named "helper" with the helper.bc file

HELP_PATH="$BENCHMARKS/helper"
for bench in compression gcd hadamard welcome; do
    echo ""
    echo "Executing benchmark: $bench"
    cd $bench

    if [ $pass == "static_icount" ]; then
        opt -load $LLVMLIB/CSE231.so -$pass -analyze < $bench.bc
    else 

        opt -load $LLVMLIB/CSE231.so -$pass < $bench.bc > $bench.instr.bc
        llvm-link $BENCHMARKS/$bench/$bench.instr.bc $HELP_PATH/helper.bc -o $bench.linked.bc

        llvm-dis $bench.instr.bc
        llvm-dis $bench.linked.bc

        llc -filetype=obj $bench.linked.bc -o=$bench.o
        g++ $bench.o -o $bench.exe
        ./$bench.exe
    fi

    cd ..
done
