from matplotlib import pyplot as plt
import math

threads = [20]
query_rates = [0]
versions = ["shared", "local_copies", "remote_inserts"]
collumns = range(10,210,10)
rows = 32
total_sizes = [float(x*threads[0]*rows*4) /1024.0 for x in collumns]

def read_perf(filename):
    returnList = []  
    with open(filename,'r') as f:
        for line in f:
            perf = float(line.split()[3])
            returnList.append(perf)
    return returnList


SizePerfData ={}
for version in versions:
    for queries in query_rates:
        for thread in threads:
            SizePerfData[(version,queries,thread)] = read_perf("logs/size_cm_"+version+"_"+str(queries)+"_queries_"+str(thread)+"_threads.log")

for version in versions:
    for queries in query_rates:
        for thread in threads:
            plt.plot(total_sizes,SizePerfData[(version,queries,thread)], label=version) 
plt.legend()
plt.xlabel("Total KBs")
plt.ylabel("Mops/sec")
plt.show()