#!/bin/bash

#    TCP Reno
sleep 2
echo "Lauched DCTCP 1610 "
screen -dm ./waf --run "scratch/mqq-incast-jitter-app --load=0.3  --filename=1630.txt --incastdegree=16"

sleep 2
echo "Lauched DCTCP 2410"
screen -dm ./waf --run "scratch/mqq-incast-jitter-app --load=0.6  --filename=2460.txt --incastdegree=24"

sleep 2
echo "Lauched DCTCP 3210"
screen -dm ./waf --run "scratch/mqq-incast-jitter-app --load=0.3  --filename=3230.txt --incastdegree=32"
'
sleep 2
echo "Launched TCP new Reno with 30% Load and only Short Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.3 --flowsize=16384 --filename=StatsR30S.txt --linkutilization=1"

sleep 2
echo "Launched TCP new Reno with 60% Load and only Short Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.6 --flowsize=16384 --filename=StatsR60S.txt --linkutilization=1"

sleep 2
echo "Lauched TCP new Reno with 10% Load and only Long Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.1 --flowsize=16777216 --filename=StatsR10L.txt --linkutilization=1"

sleep 2
echo "Launched TCP new Reno with 30% Load and only Long Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.3 --flowsize=16777216 --filename=StatsR30L.txt --linkutilization=1"

sleep 2
echo "Launched TCP new Reno with 60% Load and only Long Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.6 --flowsize=16777216 --filename=StatsR60L.txt --linkutilization=1"

#    DCTCP
sleep 2
echo "Lauched DCTCP with 10% Load and only Short Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.1 --flowsize=16384 --dctcp=1 --filename=StatsD10S.txt --linkutilization=1"

sleep 2
echo "Launched DCTCP with 30% Load and only Short Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.3 --flowsize=16384 --dctcp=1 --filename=StatsD30S.txt --linkutilization=1"

sleep 2
echo "Launched DCTCP with 60% Load and only Short Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.6 --flowsize=16384 --dctcp=1 --filename=StatsD60S.txt --linkutilization=1"

sleep 2
echo "Lauched DCTCP with 10% Load and only Long Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.1 --flowsize=16777216 --dctcp=1 --filename=StatsD10L.txt --linkutilization=1"

sleep 2
echo "Launched DCTCP with 30% Load and only Long Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.3 --flowsize=16777216 --dctcp=1 --filename=StatsD30L.txt --linkutilization=1"

sleep 2
echo "Launched DCTCP with 60% Load and only Long Flows"
screen -dm ./waf --run "scratch/datacenter --load=0.6 --flowsize=16777216  --dctcp=1 --filename=StatsD60L.txt --linkutilization=1"
'
