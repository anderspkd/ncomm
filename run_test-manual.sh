#!/bin/bash

n=5

for i in $(seq 0 $((n-2))); do
    echo "started party $i"
    (./test-manual $i $n 'exchange_all' &>/dev/null) &
done
echo "started party $((n-1))"
./test-manual $((n-1)) $n 'exchange_all'
