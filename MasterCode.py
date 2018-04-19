import subprocess
import sys
import time
import os

#Exe youd like to execute, delay
def addTask(task):
    if (task.endswith('.py')):
        task = ['python', task]
    task = subprocess.Popen(task, shell=True, bufsize=0)
    return task

def findNth(line, char, n):
    index = 0
    for chars in line:
        if chars == char:
            n -= 1
        if n == 0:
            return index
        index += 1
    return -1

subprocess.Popen('cat /dev/null > nohup.out', shell=True)
#seperate commands
assignments = ['nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=32 --load=0.1 --filename=8MBRRMQQ32D10I.txt --runtime=0.25" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=32 --load=0.3 --filename=8MBRRMQQ32D30I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=32 --load=0.4 --filename=8MBRRMQQ32D40I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=32 --load=0.5 --filename=8MBRRMQQ32D50I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=32 --load=0.6 --filename=8MBRRMQQ32D60I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=28 --load=0.1 --filename=8MBRRMQQ28D10I.txt --runtime=0.25" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=28 --load=0.3 --filename=8MBRRMQQ28D30I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=28 --load=0.4 --filename=8MBRRMQQ28D40I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=28 --load=0.5 --filename=8MBRRMQQ28D50I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=28 --load=0.6 --filename=8MBRRMQQ28D60I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=26 --load=0.1 --filename=8MBRRMQQ26D10I.txt --runtime=0.25" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=26 --load=0.3 --filename=8MBRRMQQ26D30I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=26 --load=0.4 --filename=8MBRRMQQ26D40I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=26 --load=0.5 --filename=8MBRRMQQ26D50I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=26 --load=0.6 --filename=8MBRRMQQ26D60I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=24 --load=0.1 --filename=8MBRRMQQ24D10I.txt --runtime=0.25" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=24 --load=0.3 --filename=8MBRRMQQ24D30I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=24 --load=0.4 --filename=8MBRRMQQ24D40I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=24 --load=0.5 --filename=8MBRRMQQ24D50I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=24 --load=0.6 --filename=8MBRRMQQ24D60I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=20 --load=0.1 --filename=8MBRRMQQ20D10I.txt --runtime=0.25" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=20 --load=0.3 --filename=8MBRRMQQ20D30I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=20 --load=0.4 --filename=8MBRRMQQ20D40I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=20 --load=0.5 --filename=8MBRRMQQ20D50I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=20 --load=0.6 --filename=8MBRRMQQ20D60I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=16 --load=0.1 --filename=8MBRRMQQ16D10I.txt --runtime=0.25" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=16 --load=0.3 --filename=8MBRRMQQ16D30I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=16 --load=0.4 --filename=8MBRRMQQ16D40I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=16 --load=0.5 --filename=8MBRRMQQ16D50I.txt --runtime=0.125" >nohup.out 2>&1',
               'nohup ./waf --run "scratch/mqq-incast-jitter --incastdegree=16 --load=0.6 --filename=8MBRRMQQ16D60I.txt --runtime=0.125" >nohup.out 2>&1']
tasks = []
outputFiles = []
errors = []
postFiles = []
terminated = []
PIDS = []
list = 0
filt = ['nohup', 'SIG']
post_Process = []
logfile = open("nohup.log", 'a')

print("Starting " + str(assignments.__len__()) + " Tasks")

#assignment of tasks
for i in assignments:
    tasks.append(addTask(i))
    PIDS.append(tasks[tasks.__len__() - 1].pid)
    outputFiles.append(i[i.index("filename=")+9:i.index('.txt')+4])

for i in outputFiles:
    postFiles.append(i.rstrip(".txt") + ".pdf")
    postFiles.append("result" + i)

count = tasks.__len__() #Represents the amount of main tasks to complete
postCount = 0 #Represents the amount of post process tasks to complete
sumCount = 0 #Represents the amount of summary tasks to complete
#While there are any tasks jobs or post processes
while (count > 0) or (postCount > 0):

    i = 0
    #Command line errors
    for line in open("nohup.out", 'r'):
        if ("SIG" in line) and (i not in errors):
            terminated.append(line[line.index("--filename"):line.index(".txt")])
            errors.append(i)
            print('\033[91m' + line + '\033[0m')
        i += 1

    for task in tasks:

        #Removes and reports errors
        if (task.poll() != None) and (task.poll() != 0):
            print("Process " + str(task.pid) + " failed with file " + str(outputFiles[PIDS.index(task.pid)]) + " and return code: " + str(task.returncode))
            tasks.remove(task)
            count -= 1

        #
        elif (task.poll() != None) and (task.poll() == 0):
            print(str(task.pid) + ": Task Complete")
            file = outputFiles[PIDS.index(task.pid)] #Gets the index of the parallel array
            new_Task = subprocess.Popen("nohup python Graphs.py -f " + outputFiles[PIDS.index(task.pid)] + " >>nohup.out 2>&1", shell=True) #Starts a new post process job
            PIDS.append(new_Task.pid)
            outputFiles.append(file)  # Assigns a post process task
            post_Process.append(new_Task)  # Assigns a post process task

            new_Task = subprocess.Popen("nohup python calc_fct.py -i " + outputFiles[PIDS.index(task.pid)] + " >>nohup.out 2>&1", shell=True)
            PIDS.append(new_Task.pid)  #Sets the new task as the PID in the parallel array
            outputFiles.append(file)  # Assigns a post process task
            post_Process.append(new_Task) #Assigns a post process task

            tasks.remove(task)
            count -= 1
            postCount += 2 #Flags that there is a post process running

    #Checks if there are post process tasks assigned
    if (postCount > 0):
        for pos in post_Process:

            if (pos.poll() != None) and (pos.poll() != 0):
                print("Post Process " + str(pos.pid) + " failed " + str(outputFiles[PIDS.index(pos.pid)]) + " and return code: " + str(pos.returncode))
                post_Process.remove(pos)
                postCount -= 1

            elif (pos.poll() != None) and (pos.poll() == 0):
                print(str(pos.pid) + ": Post Process Task Complete")
                post_Process.remove(pos)
                postCount -= 1

for i in postFiles:
    if i in os.listdir(os.curdir):
        continue
    else:
        print("File " + i + " failed to be created.")

for line in open("nohup.out", 'r'):
    logfile.write(line)

logfile.write("---------------------------------------------------------------------------------------------------------------------------\n")

print("All Tasks finished")
