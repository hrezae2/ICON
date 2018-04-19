#!/bin/bash

#    TCP Reno
#sleep 4
#echo "Lauched TCP new Reno with 10% Load and Incast Flows"
#screen -dm ./waf --run "scratch/new-mqq-incast-jitter-app --numofspines=10  --numofracks=20 --filename=IDEA10.txt --incastdegree=32"


#sleep 4
#echo "Lauched TCP new Reno with 10% Load and Incast Flows"
#screen -dm ./waf --run "scratch/new-mqq-incast-jitter-app --numofspines=5  --numofracks=20 --filename=IDEA5.txt --incastdegree=32"

#sleep 4
#echo "Lauched TCP new Reno with 10% Load and Incast Flows"
#screen -dm ./waf --run "scratch/new-mqq-incast-jitter-app --numofspines=4  --numofracks=20 --filename=IDEA4.txt --incastdegree=32"

#sleep 4
#echo "Lauched TCP new Reno with 10% Load and Incast Flows"
#screen -dm ./waf --run "scratch/new-mqq-incast-jitter-app --numofspines=2  --numofracks=20 --filename=IDEA2.txt --incastdegree=32"

#sleep 4
#echo "Lauched TCP new Reno with 10% Load and Incast Flows"
#screen -dm ./waf --run "scratch/jitter_incast --numofspines=10  --numofracks=20 --filename=DCTCP10.txt --incastdegree=32"


#sleep 4
#echo "Lauched TCP new Reno with 10% Load and Incast Flows"
#screen -dm ./waf --run "scratch/jitter_incast --numofspines=5  --numofracks=20 --filename=DCTCP5.txt --incastdegree=32"

#sleep 4
#echo "Lauched TCP new Reno with 10% Load and Incast Flows"
#screen -dm ./waf --run "scratch/jitter_incast --numofspines=4  --numofracks=20 --filename=DCTCP4.txt --incastdegree=32 "

#sleep 4
#echo "Lauched TCP new Reno with 10% Load and Incast Flows"
#screen -dm ./waf --run "scratch/jitter_incast --numofspines=2  --numofracks=20 --filename=DCTCP2.txt --incastdegree=32 "




screen -dm python calc_fct.py -i IDEA2.txt
screen -dm python calc_fct.py -i IDEA4.txt
screen -dm python calc_fct.py -i IDEA5.txt
screen -dm python calc_fct.py -i IDEA10.txt
#screen -dm python calc_fct.py -i new102020idea.txt
#screen -dm python calc_fct.py -i new102020dctcp.txt



#sleep 2
#echo "Launched TCP new Reno with 30% Load and Incast Flows"
#screen -dm ./waf --run "scratch/incast --load=0.3 --filename=StatsR30I.txt --linkutilization=1"

#sleep 2
#echo "Launched TCP new Reno with 60% Load and Incast Flows"
#screen -dm ./waf --run "scratch/incast --load=0.6 --filename=StatsR60I.txt --linkutilization=1"

#    DCTCP
#sleep 2
#echo "Lauched DCTCP new Reno with 10% Load and Incast Flows"
#screen -dm ./waf --run "scratch/incast --load=0.1 --dctcp=1 --filename=StatsD10I.txt --linkutilization=1"

#sleep 2
#echo "Launched DCTCP new Reno with 30% Load and Incast Flows"
#screen -dm ./waf --run "scratch/incast --load=0.3 --dctcp=1 --filename=StatsD30I.txt --linkutilization=1"

#sleep 2
#echo "Launched DCTCP new Reno with 60% Load and Incast Flows"
#screen -dm ./waf --run "scratch/incast --load=0.6 --dctcp=1 --filename=StatsD60I.txt --linkutilization=1"
