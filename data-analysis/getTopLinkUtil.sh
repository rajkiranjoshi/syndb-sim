#!/bin/bash

if [ $# -ne 1 ];then
	echo "Usage: $0 <tor Links Util file>"
	exit 1
fi

sort -g -r $1 -o $1

# get number of top links (lines)
numLines=$(wc -l $1 | cut -d' ' -f1)
topLines=$((($numLines * 5 + 50) / 100)) # +50 is for rounding

# echo $topLines

topLowest=$(head -n $topLines $1 | tail -n1)
topHighest=$(head -n 1 $1)

echo $topLowest - $topHighest

