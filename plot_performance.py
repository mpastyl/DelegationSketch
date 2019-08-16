from matplotlib import pyplot as plt
import math
import subprocess

from itertools import cycle
lines = ["-","--","-.",":"]
linecycler = cycle(lines)

threads = range(1,73,4)
query_rates = [0,1,2,3]
#versions = ["shared", "local_copies", "hybrid", "remote_inserts", "remote_inserts_filtered", "shared_filtered", "local_copies_filtered", "augmented_sketch", "delegation_filters", "delegation_filters_with_linked_list"] 
versions = ["shared", "local_copies", "augmented_sketch", "delegation_filters", "delegation_filters_with_linked_list"] 

def read_perf(filename):
    returnList = []  
    with open(filename,'r') as f:
        for line in f:
            perf = float(line.split()[3])
            returnList.append(perf)
    return returnList


ScalingData ={}
for version in versions:
    for queries in query_rates:
        ScalingData[(version,queries)] = read_perf("logs/cm_"+version+"_"+str(queries)+"_queries.log")

rows = int(math.sqrt(len(query_rates)))
columns = len(query_rates)/rows
fig,ax = plt.subplots(rows, columns,squeeze=False)
serial_ax = []
for row in ax:
    for col in row:
        serial_ax.append(col)

plot_count = 0
for queries in query_rates:
    for version in versions:
        serial_ax[plot_count].plot(threads,ScalingData[(version,queries)], next(linecycler), label = version+" - 0."+str(queries)+"%")
        serial_ax[plot_count].legend(loc=2)
        serial_ax[plot_count].set_xlabel("Threads")
        serial_ax[plot_count].set_ylabel("Mops/sec")
        #serial_ax[plot_count].set_ylim(0,70)
    
    plot_count +=1
#plt.xlabel("Threads")
#plt.ylabel("Mops/sec")
name="/home/chasty/sketches/rusu-sketches-size-join-estimation/scaling_at_1.5_skew.pdf"
plt.savefig(name)
plt.show()

query_rates = [0,1,2,3,4,5,6]
threads="72"
QueriesData = {}
for version in versions:
    QueriesData[version] = read_perf("logs/cm_"+version+"_"+threads+"_threads.log")

for version in versions:
    #plt.plot(query_rates,QueriesData[version],  next(linecycler), label = version)
    plt.plot([float(x)/10 for x in query_rates], QueriesData[version],  next(linecycler), label = version)
lgd=plt.legend()
plt.xlabel("Query rate (%)")
plt.ylabel("Mops/sec")
name="/home/chasty/sketches/rusu-sketches-size-join-estimation/query_effect_1.5_skew.pdf"
plt.savefig(name,bbox_extra_artists=(lgd,), bbox_inches = "tight")
plt.show()

skew_rates_raw_list = "0 0.25 0.5 0.75 1 1.25 1.5 1.75 2 2.25 2.5 2.75 3 3.25 3.5 3.75 4"
skew_rates = [float(x) for x in skew_rates_raw_list.split()]
threads="72"
SkewnesData = {}
query_rates = [0,1,2,3]
for version in versions:
	for query in query_rates:
            SkewnesData[(version, query)] = read_perf("logs/skew_cm_"+version+"_"+threads+"_threads_"+str(query)+"_queries.log")

for query in query_rates:
    for version in versions:
        plt.plot(skew_rates,SkewnesData[(version,query)], next(linecycler), label = version + " 0."+ str(query)+ "% queries")
    lgd = plt.legend()
    plt.xlabel("Skew parameter")
    plt.ylabel("Mops/sec")
    name="/home/chasty/sketches/rusu-sketches-size-join-estimation/skew_"+str(query)+"_queries.pdf"
    plt.savefig(name,bbox_extra_artists=(lgd,), bbox_inches = "tight")
    #subprocess.Popen("pdfcrop "+name+" "+name,shell=True)
    #subprocess.Popen("pdfcrop")
    plt.show()
