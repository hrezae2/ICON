#!/usr/bin/python

import sys
import getopt
import numpy as np
from scipy import stats


statsFile="Client.txt"
num_of_shortflows=0
num_of_longflows=0
list_of_shortflows = []
list_of_longflows = [] 
list_of_shortecnM = []
list_of_longecnM = []
list_of_shortecnC = []
list_of_longecnC = []
outputfile = 'result'

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
	global num_of_shortflows
	global num_of_longflows
        fct,fsize = np.loadtxt(statsFile, delimiter=':' , usecols = (2,3), dtype =  str , unpack=True)
	for i in range (0,len(fct)):
		c,d = fsize[i].split('\t')
                a,b = fct[i].split('\t')
		c = float(c)
		if(c == 16):
			list_of_shortflows.append(a)
			num_of_shortflows+=1
		elif(c == 1024):
			list_of_longflows.append(a)
			num_of_longflows+=1
		for i in range (0,num_of_shortflows):
			list_of_shortflows[i] = float(list_of_shortflows[i])
		for i in range (0,num_of_longflows):
			list_of_longflows[i] = float(list_of_longflows[i])
def plotMean(value):
        i50print = 0
        i99print = 0
        dummy_ecn_list = []
        f = open(outputfile, 'a+')
        if(value =='short'):
                f.write( "\nplotting Short\n")
                list_of_lists = list_of_shortflows
        if(value == 'long'):
                f.write( "\nplotting Long\n")
                list_of_lists = list_of_longflows
        a = np.mean(list_of_lists)
	b = stats.hmean(list_of_lists)
        f.write(str(a))
	f.write('\n')
	f.write("Long flows throughput with arithmetic mean:")
	f.write(str(0.008388608/(a/1000)))
        f.write('\n')
	f.write("Long flows throughput with harmonic mean:")
	f.write(str(0.008388608/(b/1000)))
	f.write('\n')
        f.close()



def plotCDF(value):
	i50print = 0
	i99print = 0
	dummy_ecn_list = []
	f = open(outputfile, 'a+')
	if(value =='short'):
		f.write( "\nplotting Short\n")
		list_of_lists = list_of_shortflows
	if(value == 'long'):
		f.write( "\nplotting Long\n")
		list_of_lists = list_of_longflows
	list_of_lists = sorted(list_of_lists)
        a = np.percentile(list_of_lists, 50)
        b = np.percentile(list_of_lists, 80)
        c = np.percentile(list_of_lists, 90)
	z = np.percentile(list_of_lists, 91)
	y = np.percentile(list_of_lists, 92)
	x = np.percentile(list_of_lists, 93)
	w = np.percentile(list_of_lists, 94)
        d = np.percentile(list_of_lists, 95)
	l = np.percentile(list_of_lists, 96)
	h = np.percentile(list_of_lists, 97)
	i = np.percentile(list_of_lists, 98)
        e = np.percentile(list_of_lists, 99)
        g = np.percentile(list_of_lists, 99.9)
	k = np.mean(list_of_lists)
	j = stats.hmean(list_of_lists)
        f.write(str(a))
        f.write('\n')
        f.write(str(b))
        f.write('\n')
        f.write(str(c))
        f.write('\n')
        f.write(str(z))
        f.write('\n')
        f.write(str(y))
        f.write('\n')
        f.write(str(x))
        f.write('\n')
        f.write(str(w))
        f.write('\n')
        f.write(str(d))
        f.write('\n')
        f.write(str(l))
        f.write('\n')
        f.write(str(h))
        f.write('\n')
        f.write(str(i))
        f.write('\n')
        f.write(str(e))
        f.write('\n')
        f.write(str(g))
        f.write('\n')
	f.write('\n')
	f.write("Short flows throughput with arithmetic mean:")
	f.write(str(0.000131072/(k/1000)))
        f.write('\n')
        f.write("Short flows throughput with harmonic mean:")
	f.write(str(0.000131072/(j/1000)))
	f.write('\n')
        f.close()


if __name__=="__main__":
	ReadArgs(sys.argv[1:])
	ReadFile()
	outputfile = outputfile + statsFile
	f = open(outputfile, 'w')
	f.close
	if(num_of_shortflows>1):
		plotCDF("short")
	if(num_of_longflows>1):
		plotMean("long")
