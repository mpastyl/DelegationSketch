from matplotlib import pyplot as plt
import math
import subprocess
from matplotlib.font_manager import FontProperties
import numpy as np
from plotters import plot_bars

#fontP = FontProperties()
#fontP.set_size('large')
import matplotlib

fontsize =15
matplotlib.rcParams.update({'font.size': fontsize})


from itertools import cycle
#lines = ["-","--","-.",":"]
lines = [":"]
linecycler = cycle(lines)
markers = ['+','.','o','^']
markercycler = cycle(markers)


name_prefix = "odysseus_"
threads = range(8,296,8)
query_rates = [0,1,2,3]
#versions = ["shared", "local_copies", "hybrid", "remote_inserts", "remote_inserts_filtered", "shared_filtered", "local_copies_filtered", "augmented_sketch", "delegation_filters", "delegation_filters_with_linked_list"] 
#versions = ["shared", "local_copies", "augmented_sketch", "delegation_filters", "delegation_filters_with_linked_list"] 
versions = ["shared", "local_copies", "augmented_sketch", "delegation_filters_with_linked_list"] 
fancy_names = ["Single-shared", "Thread-local", "Augmented Sketch", "Delegation Sketch"] 



REPS = 10

def read_perf(filename):
    returnList = []  
    with open(filename,'r') as f:
        for line in f:
            perf = float(line.split()[3])
            returnList.append(perf)
    return returnList

def average_and_std(l , reps):
	ret_avg = []
	ret_std = []
	steps = len(l)/reps
	index = 0
	for i in range(steps):
		segment = l[index:index+reps]
		ret_avg.append(np.average(segment))
		ret_std.append(np.std(segment))
		index += reps
	return ret_avg, ret_std

def subsample(x):
    if (len(x) % 2) == 0:
        return x[1::2]
    else:
        return x[0::2]

#######real world data

RealData = {}
RealDataStd = {}
query_rates = [0,1,2,3]
datasetNames = ["CAIDA_2018_source_ips","CAIDA_2018_source_ports"]
fancy_data_set_names  = ["CAIDA (low skew)", "CAIDA (high skew)"]
for version in versions:
	for query in query_rates:
	    for dataName in datasetNames:
	    	raw_data = read_perf("logs/cm_"+version+"_"+str(query)+"_queries_real_data_"+dataName+"_10_times_final.log")
	    	RealData[(version, query,dataName)], RealDataStd[(version, query,dataName)] = average_and_std(raw_data,REPS)

c = 0
matplotlib.rcParams.update({'font.size': 16})
FIG_SIZE = [5,6]
for query in query_rates:
	fig , ax = plt.subplots(1,1,figsize = FIG_SIZE)
	kernels = []
	kernels_std = []
	for version in versions:
		kernels.append ( [RealData[(version,query,dataName)][0] for dataName in datasetNames]  )
		kernels_std.append ( [RealDataStd[(version,query,dataName)][0] for dataName in datasetNames]  )
	lgd = plot_bars(ax, kernels, fancy_data_set_names,"Versions",fancy_names,[],kernels_std)
	if (c==0):
	    plt.legend(fancy_names)
	c += 1
	plt.tight_layout()
	name = "/home/chasty/sketches/rusu-sketches-size-join-estimation/"+name_prefix+"real_data_10_times"+str(query)+"_queries_final.pdf"
	plt.savefig(name)
	plt.show()
#######

ScalingData ={}
ScalingStd ={}
for version in versions:
    for queries in query_rates:
	raw_data = read_perf("logs/cm_"+version+"_"+str(queries)+"_queries_1_5_skew_10_times_final.log")
        ScalingData[(version,queries)], ScalingStd[(version,queries)] = average_and_std(raw_data, REPS)

rows = int(math.sqrt(len(query_rates)))
columns = len(query_rates)/rows
fig,ax = plt.subplots(rows, columns, figsize=(20,15))
serial_ax = []
for row in ax:
    for col in row:
        serial_ax.append(col)

plot_count = 0
for queries in query_rates:
    linecycler = cycle(lines)
    maerkercycler = cycle(markers)
    for version in versions:
	name = fancy_names[versions.index(version)]
        serial_ax[plot_count].plot(threads,ScalingData[(version,queries)], next(linecycler), label = name+" - 0."+str(queries)+"%", marker = next(markercycler), markersize=4 )
        #serial_ax[plot_count].errorbar(threads,ScalingData[(version,queries)], ScalingStd[(version,queries)], label = version+" - 0."+str(queries)+"%",  linestyle = next(linecycler))
        #serial_ax[plot_count].legend(loc=2)
        serial_ax[plot_count].legend(bbox_to_anchor=(0,1.02,1,0.2), loc="lower left", mode="expand", borderaxespad=0, ncol=2)
        serial_ax[plot_count].set_xlabel("Threads")
        serial_ax[plot_count].set_ylabel("Mops/sec")
        serial_ax[plot_count].set_ylim(0,1400)
    
    plot_count +=1
name = "/home/chasty/sketches/rusu-sketches-size-join-estimation/"+name_prefix+"scaling_at_1_5_skew_10_times_final.pdf"
plt.savefig(name)
plt.show()

#########same but separate plots
matplotlib.rcParams.update({'font.size': 20})
FIG_SIZE = [8,6]
c=0
for queries in query_rates:
    plt.figure(figsize = FIG_SIZE)
    linecycler = cycle(lines)
    maerkercycler = cycle(markers)
    for version in versions:
	name = fancy_names[versions.index(version)]
	if (c==0): 
            plt.plot(subsample(threads),subsample(ScalingData[(version,queries)]), next(linecycler), label = name+"-"+str(queries)+"%", marker = next(markercycler), markersize=8, linewidth = 3)
	else:
            plt.plot(subsample(threads),subsample(ScalingData[(version,queries)]), next(linecycler), label = name+"-0."+str(queries)+"%", marker = next(markercycler), markersize=8, linewidth = 3)
        #plt.errorbar(threads,ScalingData[(version,queries)], ScalingStd[(version,queries)], label = version+" - 0."+str(queries)+"%",  linestyle = next(linecycler))
        #plt.legend(loc=2)
    #plt.legend(bbox_to_anchor=(0,1.02,1,0.2), loc="lower left", mode="expand", borderaxespad=0, ncol=2)
    plt.legend(loc="upper left")
    plt.xlabel("Threads")
    plt.ylabel("Throughput (Mops/sec)")
    plt.ylim(0,2200)
    c += 1
    name="/home/chasty/sketches/rusu-sketches-size-join-estimation/"+name_prefix+"scaling_at_1_5_queries_"+str(queries)+"_skew_10_times_final.pdf"
    plt.tight_layout()
    plt.savefig(name)
    plt.show()


query_rates = [0,1,2,3,4,5,6]
threads="288"
QueriesData = {}
QueriesStd = {}
for version in versions:
    raw_data = read_perf("logs/cm_"+version+"_"+threads+"_threads_1_5_skew_10_times_final.log")
    QueriesData[version], QueriesStd[version] = average_and_std(raw_data, REPS)



matplotlib.rcParams.update({'font.size': 20})
FIG_SIZE = [6,6]
plt.figure(figsize = FIG_SIZE)
for version in versions:
    #plt.plot(query_rates,QueriesData[version],  next(linecycler), label = version)
    name = fancy_names[versions.index(version)]
    plt.plot([float(x)/10 for x in query_rates], QueriesData[version],  next(linecycler), label = name, marker = next(markercycler), markersize = 8, linewidth = 3)
lgd=plt.legend(loc = 'upper right')
#lgd = plt.legend(bbox_to_anchor=(0,1.02,1,0.2), loc="lower left", mode="expand", borderaxespad=0, ncol=3, prop=fontP)
plt.xlabel("Query rate (%)")
plt.ylabel("Throughput (Mops/sec)")
plt.tight_layout()
plt.ylim([-100,2499])
yticks = [0,500,1000,1500,2000]
yticks_labels = ['0','500','1000','1500','2000']
plt.yticks(yticks,yticks_labels)
name="/home/chasty/sketches/rusu-sketches-size-join-estimation/"+name_prefix+"query_effect_1_5_skew_10_times_final.pdf"
#plt.savefig(name,bbox_extra_artists=(lgd,), bbox_inches = "tight")
plt.savefig(name)
plt.show()


matplotlib.rcParams.update({'font.size': 15})
FIG_SIZE = (7,5)
skew_rates_raw_list = "0 0.25 0.5 0.75 1 1.25 1.5 1.75 2 2.25 2.5 2.75 3 3.25 3.5 3.75 4"
skew_rates = [float(x) for x in skew_rates_raw_list.split()]
threads="288"
SkewnesData = {}
SkewnesStd = {}
query_rates = [0,1,2,3]
for version in versions:
	for query in query_rates:
	    SkewnesData[(version, query)] = read_perf("logs/skew_cm_"+version+"_"+threads+"_threads_"+str(query)+"_queries.log")


c = 0
for query in query_rates:
    linecycler = cycle(lines)
    plt.figure(figsize=FIG_SIZE)
    for version in versions:
	name = fancy_names[versions.index(version)]
        #plt.plot(skew_rates,SkewnesData[(version,query)], next(linecycler), label = name + " - 0."+ str(query)+ "% queries", marker = next(markercycler), markersize = 4)
        plt.plot(skew_rates,SkewnesData[(version,query)], next(linecycler), label = name, marker = next(markercycler), markersize = 4)
    if (c==0):
        plt.legend(loc = 'upper left')
    plt.xlabel("Skew parameter")
    plt.ylabel("Throuhput (Mops/sec)")
    plt.tight_layout()
    c +=1
    name="/home/chasty/sketches/rusu-sketches-size-join-estimation/"+name_prefix+"skew_"+str(query)+"_queries_final.pdf"
    plt.savefig(name)
    plt.show()
