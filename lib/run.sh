#!/usr/bin/env bash

# The script must be in $BENCHMARKS folder. There has to exist a folder 
# named "helper" with the helper.bc file

HELP_PATH="$BENCHMARKS/helper"
for bench in compression gcd hadamard welcome; do
    echo ""
    echo "Executing benchmark: $bench"
    cd $bench
    opt -load $LLVMLIB/CSE231.so -BranchBias < $bench.bc > $bench.instr.bc
    llvm-link $BENCHMARKS/$bench/$bench.instr.bc $HELP_PATH/helper.bc -o $bench.linked.bc

    llvm-dis $bench.instr.bc
    llvm-dis $bench.linked.bc

    llc -filetype=obj $bench.linked.bc -o=$bench.o
    g++ $bench.o -o $bench.exe
    ./$bench.exe
    cd ..
done
