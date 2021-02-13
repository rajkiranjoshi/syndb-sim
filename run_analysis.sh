#!/bin/bash

if [ $# -lt 4 ];then
    echo "Usage: $0 <Start Trigger> <End Trigger> <Increment> <server>"
    exit 1
fi

startTrigger=$1
endTrigger=$2
increment=$3
server=$4

echo "Running triggers from $startTrigger to $endTrigger with each run processing $increment triggers!!!"

make cleaner
make -j20 analysis

while  [ $startTrigger -lt $endTrigger ]
do
    tempEndTrigger=$(($startTrigger + $increment - 1))
    cp syndb-analysis syndb-analysis-$startTrigger-$server
    echo "screen -S nvme_$startTrigger bash -c './syndb-analysis-$startTrigger-$server $startTrigger $tempEndTrigger  2>&1 | tee ./data-analysis-output/patronus/dump_1_4_46/output_for_triggers_$startTrigger-$tempEndTrigger.txt'"
    read varname
    startTrigger=$(($tempEndTrigger + 1))
    # echo "$startTrigger"
done

