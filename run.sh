#!/bin/bash

if [ $# -ne 1 ];then
    echo "Usage: $0 <Name for expt run>"
    exit 1
fi

echo "Running expt: $1"

./syndb-sim

curl -X POST -H "Content-Type: application/json" -d "{\"value1\":\"$1\"}" https://maker.ifttt.com/trigger/SyNDB-sim_completed/with/key/gB88SULNID5Te0obIzRqK-6a-6EO6tHSSBT5ulPEBbT

echo ""
