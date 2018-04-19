#!/bin/bash

sleep 1
echo "ALL to ALL JOBS ...."
./run_all_to_all.sh

sleep 2
echo "ALL Mix JOBS ...."
./run_mix_flows.sh

sleep 2
echo "INCAST JOBS ...."
./run_incast_flows.sh
