#!/usr/bin/python

import sys
import getopt
import numpy as np
import matplotlib.pyplot as plt
import math
from scipy import stats


statsFile="utily.txt"
list_of_utilization = []
list_of_nonzeros = []
variancelog = []
def ReadArgs(argv):
	global statsFile
	statsfile = ''
   	try:
		opts, args = getopt.getopt(argv,"hi:","ifile=")
	except getopt.GetoptError:
		print 'usage: python my_plotter.py -i <inputfile>'
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			print 'usage: python my_plotter.py -i <inputfile>'
			sys.exit()
		elif opt in ("-i", "--ifile"):
			statsfile = arg
			statsFile = statsfile
	print 'Stats file is ', statsFile
	
def ReadFile():
        global list_of_utilization
	global list_of_nonzeros
	s,futil = np.loadtxt(statsFile, delimiter=':' , usecols = (2,3), dtype =  str , unpack=True)
	for i in range (0,len(futil)):
		list_of_utilization.append(futil[i])
	for i in range (0,len(list_of_utilization)):
		list_of_utilization[i] = float(list_of_utilization[i])
	
def AverageUtilization():
	print len(list_of_utilization)
	index = 200 #int(0.1*len(list_of_utilization))
	logmean = np.mean(list_of_utilization[:index])
	for i in range (0, index):
		variancelog.append(list_of_utilization[i] - logmean)
		variancelog[i] = variancelog[i]*variancelog[i]
	print logmean
	print np.mean(variancelog)
	print math.sqrt(np.mean(variancelog))
if __name__=="__main__":
	ReadArgs(sys.argv[1:])
	ReadFile()
	AverageUtilization()
