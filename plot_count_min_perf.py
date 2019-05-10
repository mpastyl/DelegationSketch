from matplotlib import pyplot as plt

versions = ["shared", "local_copies", "hybrid", "remote_inserts"]
#filename = "count_min_results.txt"
threads="20"

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
    filename = "cm_"+version+"_"+threads+"_accuracy.log"
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

    print "Version ", version, " Average Relative Error: ", computeARE(hist,answers)
    print "Version ", version, " Observed Error: ", computeObservedError(hist, answers)
    #print "Absolute difference on most frequent element: ", answers[0] - hist[0]

    # plt.plot(answers, label = "Approximation")
    # plt.plot(hist,label = "True")
    # plt.legend()
    # plt.show()
    
    totalHist[version] = hist
    totalAnswers[version] = answers

plt.plot(totalHist[versions[0]], label = "True")
for version in versions:
    plt.plot(totalAnswers[version], label = version)
plt.legend()
plt.show()
