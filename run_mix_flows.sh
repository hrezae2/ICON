#!/bin/bash

#    TCP Reno
sleep 2
echo "Lauched TCP new Reno with 10% Load and Mix Flows"
screen -dm ./waf --run "scratch/mix --load=0.1 --filename=StatsR10M.txt --linkutilization=1"

sleep 2
echo "Launched TCP new Reno with 30% Load and Mix Flows"
screen -dm ./waf --run "scratch/mix --load=0.3 --filename=StatsR30M.txt --linkutilization=1"

sleep 2
echo "Launched TCP new Reno with 60% Load and Mix Flows"
screen -dm ./waf --run "scratch/mix --load=0.6 --filename=StatsR60M.txt --linkutilization=1"

#    DCTCP
sleep 2
echo "Lauched DCTCP new Reno with 10% Load and Mix Flows"
screen -dm ./waf --run "scratch/mix --load=0.1 --dctcp=1 --filename=StatsD10M.txt --linkutilization=1"

sleep 2
echo "Launched DCTCP new Reno with 30% Load and Mix Flows"
screen -dm ./waf --run "scratch/mix --load=0.3 --dctcp=1 --filename=StatsD30M.txt --linkutilization=1"

sleep 2
echo "Launched DCTCP new Reno with 60% Load and Mix Flows"
screen -dm ./waf --run "scratch/mix --load=0.6 --dctcp=1 --filename=StatsD60M.txt --linkutilization=1"
