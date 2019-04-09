from matplotlib import pyplot as plt

filename = "count_min_results.txt"

hist = []
answers = []

with open(filename) as fp:
    for line in fp:
        index = line.split()[0]
        trueValue = float(line.split()[1])
        approximation =  float(line.split()[2])
        
        hist.append(trueValue)
        answers.append(approximation)

plt.plot(answers,'.', label = "Approximation")
plt.plot(hist,'.',label = "True")
plt.legend()
plt.show()
