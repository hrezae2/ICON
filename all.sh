#!/bin/bash


#screen -dm bash -c './waf --run "scratch/ICON --load=0.2  --incastdegree=20 --dctcp=1  --filename=L20I20coms.txt --runtime=0.250" ; python calc_fct.py -i L20I20coms.txt'

#sleep 2

#screen -dm bash -c './waf --run "scratch/ICON --load=0.4  --incastdegree=20 --dctcp=1  --filename=L40I20coms.txt --runtime=0.150" ; python calc_fct.py -i L40I20coms.txt'

#sleep 2

#screen -dm bash -c './waf --run "scratch/ICON --load=0.6  --incastdegree=20 --dctcp=1  --filename=L60I20coms.txt --runtime=0.100" ; python calc_fct.py -i L60I20coms.txt'

#sleep 2

#screen -dm bash -c './waf --run "scratch/ICON --load=0.8  --incastdegree=20 --dctcp=1  --filename=L80I20coms.txt --runtime=0.05" ; python calc_fct.py -i L80I20coms.txt'

#sleep 2

#screen -dm bash -c './waf --run "scratch/ICON --load=0.2  --incastdegree=26 --dctcp=1  --filename=L20I26coms.txt --runtime=0.250" ; python calc_fct.py -i L20I26coms.txt'

#sleep 4

#screen -dm bash -c './waf --run "scratch/ICON --load=0.4  --incastdegree=26 --dctcp=1  --filename=L40I26coms.txt --runtime=0.150" ; python calc_fct.py -i L40I26coms.txt'

#sleep 4

#screen -dm bash -c './waf --run "scratch/ICON --load=0.6  --incastdegree=26 --dctcp=1  --filename=L60I26coms.txt --runtime=0.100" ; python calc_fct.py -i L60I26coms.txt'

#sleep 4

#screen -dm bash -c './waf --run "scratch/ICON --load=0.8  --incastdegree=26 --dctcp=1  --filename=L80I26coms.txt --runtime=0.05" ; python calc_fct.py -i L80I26coms.txt'


screen -dm bash -c './waf --run "scratch/ICON --load=0.2  --incastdegree=32 --dctcp=1  --filename=L20I32coms.txt --runtime=0.250" ; python calc_fct.py -i L20I32coms.txt'

sleep 2

screen -dm bash -c './waf --run "scratch/ICON --load=0.4  --incastdegree=32 --dctcp=1  --filename=L40I32coms.txt --runtime=0.150" ; python calc_fct.py -i L40I32coms.txt'

sleep 2

screen -dm bash -c './waf --run "scratch/ICON --load=0.6  --incastdegree=32 --dctcp=1  --filename=L60I32coms.txt --runtime=0.100" ; python calc_fct.py -i L60I32coms.txt'

sleep 2

screen -dm bash -c './waf --run "scratch/ICON --load=0.8  --incastdegree=32 --dctcp=1  --filename=L80I32coms.txt --runtime=0.05" ; python calc_fct.py -i L80I32coms.txt'

#screen -dm python calc_fct.py -i L20I20coms.txt
#screen -dm python calc_fct.py -i L40I20coms.txt
#screen -dm python calc_fct.py -i L60I20coms.txt
#screen -dm python calc_fct.py -i L80I20coms.txt

#screen -dm python calc_fct.py -i L20I26coms.txt
#screen -dm python calc_fct.py -i L40I26coms.txt
#screen -dm python calc_fct.py -i L60I26coms.txt
#screen -dm python calc_fct.py -i L80I26coms.txt

#screen -dm python calc_fct.py -i L20I32coms.txt
#screen -dm python calc_fct.py -i L40I32coms.txt
#screen -dm python calc_fct.py -i L60I32coms.txt
#screen -dm python calc_fct.py -i L80I32coms.txt






