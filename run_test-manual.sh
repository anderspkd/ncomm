#!/bin/bash

n=$1

if [ -z "$n" ]; then
    n=3
fi

if [ "$2" != "--no-logs" ]; then
    if [ -d logs ]; then
	echo "logs dir exist. remove it before running"
	# exit 1
	rm -rf logs
    fi
    mkdir logs
    save_logs=1
else
    echo "not saving outputs"
    save_logs=0
fi

for i in $(seq 0 $((n-2))); do
    echo "started party $i"
    if [ "${save_logs}" = "0" ]; then
	(./test-manual $i $n &>/dev/null) &
    else
	(./test-manual $i $n &>logs/p$i.log) &
    fi

done
echo "started party $((n-1))"
if [ "${save_logs}" = "0" ]; then
    ./test-manual $((n-1)) $n
else
    ./test-manual $((n-1)) $n &>logs/p$((n-1)).log
fi
