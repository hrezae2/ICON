screen -dm bash -c './waf --run "scratch/mqq-incast-jitter-app --load=0.6  --incastdegree=26 --dctcp=1  --filename=L20I20coms.txt --runtime=0.150" ; python calc_fct.py -i L20I20coms.txt'
