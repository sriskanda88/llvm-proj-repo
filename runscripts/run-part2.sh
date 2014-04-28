#!/usr/bin/env bash

pass="dynamic_icount"

if [ -z "$1" ]; then
    echo "Usage : ./run-part2.sh <benchmark path>"
    exit
fi

bench_path=$1
bench=` echo $bench_path | awk -F "/" '{print $NF}'`
HELP_PATH="$INSTRUMENTATION/dynamic"
help_module="dynamic_helper"

#generate helper module bitcode
clang -O0 -emit-llvm -c $HELP_PATH/$help_module.cpp -o $HELP_PATH/$help_module.bc

cd $bench_path

opt -load $LLVMLIB/CSE231.so -$pass < $bench.bc > $bench.instr.bc
llvm-link $BENCHMARKS/$bench/$bench.instr.bc $HELP_PATH/$help_module.bc -o $bench.linked.bc

llvm-dis $bench.instr.bc
llvm-dis $bench.linked.bc

llc -filetype=obj $bench.linked.bc -o=$bench.o
g++ $bench.o -o $bench.exe

./$bench.exe 2> $OUTPUTLOGS/$bench.dynamic.log

cd -
