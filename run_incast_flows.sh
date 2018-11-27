#!/bin/bash

#    TCP Reno
sleep 2
echo "Lauched TCP new Reno with 10% Load and Incast Flows"
screen -dm ./waf --run "scratch/incast --load=0.1 --filename=StatsR10I.txt --linkutilization=1"

sleep 2
echo "Launched TCP new Reno with 30% Load and Incast Flows"
screen -dm ./waf --run "scratch/incast --load=0.3 --filename=StatsR30I.txt --linkutilization=1"

sleep 2
echo "Launched TCP new Reno with 60% Load and Incast Flows"
screen -dm ./waf --run "scratch/incast --load=0.6 --filename=StatsR60I.txt --linkutilization=1"

#    DCTCP
sleep 2
echo "Lauched DCTCP new Reno with 10% Load and Incast Flows"
screen -dm ./waf --run "scratch/incast --load=0.1 --dctcp=1 --filename=StatsD10I.txt --linkutilization=1"

sleep 2
echo "Launched DCTCP new Reno with 30% Load and Incast Flows"
screen -dm ./waf --run "scratch/incast --load=0.3 --dctcp=1 --filename=StatsD30I.txt --linkutilization=1"

sleep 2
echo "Launched DCTCP new Reno with 60% Load and Incast Flows"
screen -dm ./waf --run "scratch/incast --load=0.6 --dctcp=1 --filename=StatsD60I.txt --linkutilization=1"
