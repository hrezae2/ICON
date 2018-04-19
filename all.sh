#!/bin/bash

screen -dm bash -c './waf --run "scratch/ICON --load=0.1  --incastdegree=24 --dctcp=1  --filename=hamed.txt --runtime=0.007748" '
#screen -dm bash -c './waf --run "scratch/mqq-incast-jitter-app   --load=0.1   --dctcp=1  --filename=test10.txt     --runtime=0.5" ; python calc_fct.py -i test10.txt'
#sleep 2
#screen -dm bash -c './waf --run "scratch/jitter_incast      --load=0.1   --dctcp=1  --filename=newincast10.txt  --runtime=0.5" ; python calc_fct.py -i newincast10.txt'

#sleep 2

#screen -dm bash -c './waf --run "scratch/mqq-incast-jitter-app   --load=0.4   --dctcp=1  --filename=newmqqincast40.txt     --runtime=0.25" ; python calc_fct.py -i newmqqincast40.txt'
#sleep 2
#screen -dm bash -c './waf --run "scratch/jitter_incast      --load=0.3   --dctcp=1  --filename=newincast30.txt  --runtime=0.25" ; python calc_fct.py -i newincast30.txt'

#screen -dm bash -c './waf --run "scratch/mqq-incast-jitter-app   --load=0.5   --dctcp=1  --filename=newmqqincast50.txt     --runtime=0.125" ; python calc_fct.py -i newmqqincast50.txt'
#sleep 2
#screen -dm bash -c './waf --run "scratch/jitter_incast      --load=0.6   --dctcp=1  --filename=newincast60.txt  --runtime=0.125" ; python calc_fct.py -i newincast60.txt'
