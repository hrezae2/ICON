#screen -dm bash -c './waf --run "scratch/queuetest --load=0.8  --incastdegree=26 --dctcp=1  --filename=q80.txt --runtime=0.10" > 80queuelen.txt'
#screen -dm bash -c './waf --run "scratch/queuetest --load=0.2  --incastdegree=26 --dctcp=1  --filename=q20.txt --runtime=0.250" > 20queuelen.txt'
screen -dm bash -c './waf --run scratch/queuetest > 25queuelen.txt'

