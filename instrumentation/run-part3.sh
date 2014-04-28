#!/bin/bash

if [[ $# != 1 ]]; then
    echo "Usage: ./run-part3.sh <BENCH PATH>"
    echo "Example: ./run-part3.sh $BENCHMARKS/hadamard"
    exit
fi

bench=`echo $1 | awk 'BEGIN { FS = "/" } ; { print $NF}'`
echo $bench

HELP_PATH="$INSTRUMENTATION/branchbias"

origin=`pwd`
# Get into the benchmark folder
cd $1

# Compile the instrumentation code
clang -O0 -emit-llvm -c $HELP_PATH/helper.cpp -o $HELP_PATH/helper.bc

# Run the pass on the benchmark's bitcode
opt -load $LLVMLIB/CSE231.so -BranchBias < $bench.bc > $bench.instr.bc

# Link the benchmark and instrumentation code
llvm-link $bench.instr.bc $HELP_PATH/helper.bc -o $bench.linked.bc

# Create executable
llc -filetype=obj $bench.linked.bc -o=$bench.o
g++ $bench.o -o $bench.exe

# Run it
./$bench.exe > $OUTPUTLOGS/$bench.branchbias.log

# Go to the original folder
cd $origin
