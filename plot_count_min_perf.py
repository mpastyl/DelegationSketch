from matplotlib import pyplot as plt

versions = ["shared", "local_copies", "hybrid", "remote_inserts"]
#filename = "count_min_results.txt"
thread_list=range(1,29)

def computeARE(groundTruth, estimates):
    sum = 0
    for gt, est in zip(groundTruth,estimates):
        sum += abs(gt - est)/ float(gt)
    return sum / float (len(groundTruth))

def computeObservedError(groundTruth, estimates):
    sumError = 0
    sumTrueFreq = 0
    for gt, est in zip(groundTruth,estimates):
        sumError += abs(gt - est)
        sumTrueFreq += gt
    return float(sumError)/sumTrueFreq

def deserializeFrequencies(indexes,hist,answers):
    returnTrueFreqs = []
    returnEstimates = []
    for index in indexes:
        returnTrueFreqs += [hist[index]] * hist[index]
        returnEstimates += [answers[index]] * hist[index]
    return returnTrueFreqs, returnEstimates

totalHist = {}
totalAnswers = {}
for version in versions:
    for threads in thread_list:
        filename = "cm_"+version+"_"+str(threads)+"_accuracy.log"
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

        #print "Version ", version, " Average Relative Error: ", computeARE(hist,answers)
        #print "Version ", version, " Observed Error: ", computeObservedError(hist, answers)

        totalHist[(version,threads)] = hist
        totalAnswers[(version,threads)] = answers

plt.plot(totalHist[(versions[0],thread_list[0])], label = "True")
for version in versions:
    plt.plot(totalAnswers[(version,20)], label = version)
plt.legend()
plt.show()

realValues = totalHist[(versions[0],thread_list[0])]
for version in versions:
    ARE = [computeARE(realValues, totalAnswers[(version,threads)]) for threads in thread_list]
    plt.plot(ARE, label = version)
plt.xlabel("Threads")
plt.ylabel("Average Relative Error")
plt.legend(loc=2)
plt.show()

realValues = totalHist[(versions[0],thread_list[0])]
for version in versions:
    ObservedError = [computeObservedError(realValues, totalAnswers[(version,threads)]) for threads in thread_list]
    plt.plot(ObservedError, label = version)
plt.xlabel("Threads")
plt.ylabel("Observed Error")
plt.legend(loc=2)
plt.show()