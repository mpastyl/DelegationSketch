from matplotlib import pyplot as plt

filename = "count_min_results.txt"

def computeARE(groundTruth, estimates):
    sum = 0
    for gt, est in zip(groundTruth,estimates):
        sum += abs(gt - est)/ float(gt)
    return sum / float (len(groundTruth))

def deserializeFrequencies(indexes,hist,answers):
    returnTrueFreqs = []
    returnEstimates = []
    for index in indexes:
        returnTrueFreqs += [hist[index]] * hist[index]
        returnEstimates += [answers[index]] * hist[index]
    return returnTrueFreqs, returnEstimates

hist = []
answers = []
indexes = []

with open(filename) as fp:
    for line in fp:
        index = int(line.split()[0])
        trueValue = int(line.split()[1])
        approximation =  float(line.split()[2])
        
        indexes.append(index)
        hist.append(trueValue)
        answers.append(approximation)

hist, answers = deserializeFrequencies(indexes, hist, answers)

print "Average Relative Error: ", computeARE(hist,answers)
print "Absolute difference on most frequent element: ", answers[0] - hist[0]

plt.plot(answers, label = "Approximation")
plt.plot(hist,label = "True")
plt.legend()
plt.show()