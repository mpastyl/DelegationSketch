from matplotlib import pyplot as plt


versions = ["shared", "local_copies", "hybrid", "remote_inserts"]

def read_perf(filename):
    returnList = []  
    with open(filename,'r') as f:
        for line in f:
            perf = float(line.split()[3])
            returnList.append(perf)
    return returnList


ScalingData ={}
for version in versions:
    ScalingData[version] = read_perf("cm_"+version+"_no_queries.log")

for version in versions:
    plt.plot(ScalingData[version], label = version)
plt.legend()
plt.xlabel("Threads")
plt.ylabel("Mops/sec")
plt.show()

query_rates = [0,2,4,6,8,10]
threads="20"
QueriesData = {}
for version in versions:
    QueriesData[version] = read_perf("cm_"+version+"_"+threads+"_threads.log")

for version in versions:
    plt.plot(query_rates,QueriesData[version], label = version)
plt.legend()
plt.xlabel("Query rate (%)")
plt.ylabel("Mops/sec")
plt.show()