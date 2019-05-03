from matplotlib import pyplot as plt

def read_perf(filename):
    returnList = []  
    with open(filename,'r') as f:
        for line in f:
            perf = float(line.split()[3])
            returnList.append(perf)
    return returnList

fname_shared_no_q = "shared_no_queries.txt"
fname_share_nothing_no_q = "share_nothing_no_queries.txt"
fname_shared_hybrid = "shared_hybrid.txt"

shared_no_q =  read_perf(fname_shared_no_q)
share_nothing_no_q = read_perf(fname_share_nothing_no_q)
shared_hybrid = read_perf(fname_shared_hybrid)

plt.plot(shared_no_q,label='Shared sketch')
plt.plot(share_nothing_no_q,label="Independent Sketches")
plt.plot(shared_hybrid,label='Hybrid')
plt.legend()
plt.xlabel("Threads")
plt.ylabel("Mops/sec")
plt.show()


fname_shared_20_threads = "shared_20_threads.txt"
fname_share_nothing_20_threads = "share_nothing_20_threads.txt"
fname_hybrid_20_threads = "hybrid_20_threads.txt"

shared_20_threads = read_perf(fname_shared_20_threads)
share_nothing_20_threads = read_perf(fname_share_nothing_20_threads)
hybrid_20_threads = read_perf(fname_hybrid_20_threads)

query_rates = [0,2,4,6,8,10]
plt.plot(query_rates, shared_20_threads,label='Shared sketch')
plt.plot(query_rates, share_nothing_20_threads,label="Independent Sketches")
plt.plot(query_rates, hybrid_20_threads,label='Hybrid sketch')
plt.legend()
plt.xlabel("Query rate (%)")
plt.ylabel("Mops/sec")
plt.show()

fname_shared_10_threads = "shared_10_threads.txt"
fname_share_nothing_10_threads = "share_nothing_10_threads.txt"

shared_10_threads = read_perf(fname_shared_10_threads)
share_nothing_10_threads = read_perf(fname_share_nothing_10_threads)

query_rates = [0,2,4,6,8,10]
plt.plot(query_rates, shared_10_threads,label='Shared sketch')
plt.plot(query_rates, share_nothing_10_threads,label="Independent Sketches")
plt.legend()
plt.xlabel("Query rate (%)")
plt.ylabel("Mops/sec")
plt.show()