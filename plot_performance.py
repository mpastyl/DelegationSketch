from matplotlib import pyplot as plt
import math

threads = range(1,29)
query_rates = [0,2,4,6]
versions = ["shared", "local_copies", "hybrid", "remote_inserts", "remote_inserts_filtered", "shared_filtered", "local_copies_filtered", "augmented_sketch"] 

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
        serial_ax[plot_count].plot(threads,ScalingData[(version,queries)], label = version+" - "+str(queries)+"%")
        serial_ax[plot_count].legend(loc=2)
        serial_ax[plot_count].set_xlabel("Threads")
        serial_ax[plot_count].set_ylabel("Mops/sec")
        #serial_ax[plot_count].set_ylim(0,70)
    
    plot_count +=1
#plt.xlabel("Threads")
#plt.ylabel("Mops/sec")
plt.show()

query_rates = [0,2,4,6]
threads="20"
QueriesData = {}
for version in versions:
    QueriesData[version] = read_perf("logs/cm_"+version+"_"+threads+"_threads.log")

for version in versions:
    plt.plot(query_rates,QueriesData[version], label = version)
plt.legend()
plt.xlabel("Query rate (%)")
plt.ylabel("Mops/sec")
plt.show()

skew_rates_raw_list = "0 0.25 0.5 0.75 1 1.75 2 2.25 2.5 2.75 3 3.5 4"
skew_rates = [x for x in skew_rates_raw_list.split()]
threads="28"
SkewnesData = {}
for version in versions:
    SkewnesData[version] = read_perf("logs/skew_cm_"+version+"_"+threads+"_threads.log")

for version in versions:
    plt.plot(skew_rates,SkewnesData[version], label = version)
plt.legend()
plt.xlabel("Skew parameter")
plt.ylabel("Mops/sec")
plt.show()
