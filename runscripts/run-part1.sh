#!/usr/bin/env bash

pass="static_icount"

if [ -z "$1" ]; then
    echo "Usage : ./run-part1.sh benchmark path>"
    exit
fi

bench_path=$1
bench=` echo $bench_path | awk -F "/" '{print $NF}'`
HELP_PATH="$BENCHMARKS/dynamic"

opt -load $LLVMLIB/CSE231.so -$pass -analyze < $bench_path/$bench.bc > $OUTPUTLOGS/$bench.static.log
