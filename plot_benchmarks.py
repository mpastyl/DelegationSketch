from matplotlib import pyplot as plt

fname_shared_no_q = "shared_no_queries.txt"
fname_share_nothing_no_q = "share_nothing_no_queries.txt"

shared_no_q = []
share_nothing_no_q = []

with open(fname_shared_no_q,'r') as f:
    for line in f:
        perf = float(line.split()[3])
        shared_no_q.append(perf)

with open(fname_share_nothing_no_q,'r') as f:
    for line in f:
        perf = float(line.split()[3])
        share_nothing_no_q.append(perf)

plt.plot(shared_no_q,label='Shared sketch')
plt.plot(share_nothing_no_q,label="Independent Sketches")
plt.legend()
plt.xlabel("Threads")
plt.ylabel("Mops/sec")
plt.show()