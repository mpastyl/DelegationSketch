from matplotlib import pyplot as plt

from itertools import cycle
lines = ["-","--","-.",":"]
linecycler = cycle(lines)

from matplotlib.font_manager import FontProperties
fontP = FontProperties()
fontP.set_size('large')

suffix="zipf"

#versions = ["shared", "local_copies", "hybrid", "remote_inserts", "remote_inserts_filtered", "shared_filtered", "local_copies_filtered", "augmented_sketch", "delegation_filters", "delegation_filters_with_linked_list"]
#versions = ["shared", "local_copies", "augmented_sketch", "delegation_filters", "delegation_filters_with_linked_list"]
versions = ["local_copies", "shared", "remote_inserts","shared_small"]
fancy_names = ["Thread-local", "Single Sketch", "Domain-spliting", "Reference"]
#filename = "count_min_results.txt"
#thread_list=range(1,29)
#thread_list=[10]
thread_list=range(1,11)

def sortAccordingToTrueValues(trueValues, answers):
    #zipped = zip(trueValues,answers)
    #res = [x for _, x in sorted(zipped, reverse = True)]
    sortingIndexes = sorted(range(len(trueValues)), key=lambda k: trueValues[k], reverse = True)
    res = []
    for x in sortingIndexes:
        res.append(answers[x])
    return res

def sortAll(trueValues, totalAnswers):
    for version in versions:
        totalAnswers[(version,thread_list[0])] = sortAccordingToTrueValues(trueValues, totalAnswers[(version,thread_list[0])])

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

totalARE={}
totalObsE={}
for version in versions:
    for threads in thread_list:
        filename = "logs/cm_"+version+"_"+str(threads)+"_accuracy.log"
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

        totalARE[(version, threads)] = computeARE(hist,answers) 
        totalObsE[(version, threads)] = computeObservedError(hist, answers)
        print "Version ", version, " Average Relative Error: ", totalARE[(version, threads)] 
        print "Version ", version, " Observed Error: ", totalObsE[(version, threads)]

        totalHist[(version,threads)] = hist
        totalAnswers[(version,threads)] = answers

sortAll(totalHist[(versions[0],thread_list[0])], totalAnswers)
totalHist[(versions[0],thread_list[0])] =  sorted(totalHist[(versions[0],thread_list[0])], reverse = True)
plt.plot(totalHist[(versions[0],thread_list[0])], label = "True")
for version in versions:
    plt.plot(totalAnswers[(version,thread_list[0])], next(linecycler), label = version)
plt.legend()
plt.show()

for version in versions:
    for i in range(len(totalAnswers[(version,thread_list[0])])):
        if totalAnswers[(version,thread_list[0])][i] < totalHist[(versions[0],thread_list[0])][i]:
            print "Under estimation in ",i

FIG_SIZE = (5,5)
plt.figure(figsize=FIG_SIZE)
c = 0
for version in versions:
    ARE = [totalARE[(version,threads)] for threads in thread_list]
    plt.plot(thread_list, ARE, label = fancy_names[c])
    c = c +1
plt.xlabel("Threads")
plt.ylabel("Average Relative Error")
lgd = plt.legend(loc = 'center right')
name="/home/chasty/sketches/rusu-sketches-size-join-estimation/average_relative_error_"+suffix+".pdf"
plt.savefig(name, bbox_extra_artists=(lgd,), bbox_inches = "tight")
plt.show()

plt.figure(figsize=FIG_SIZE)
c = 0
for version in versions:
    ObservedError = [ totalObsE[(version, threads)] for threads in thread_list]
    plt.plot(thread_list, ObservedError, label = fancy_names[c])
    c = c +1
plt.xlabel("Threads")
plt.ylabel("Observed Error (%)")
lgd = plt.legend(loc = 'center right')
name="/home/chasty/sketches/rusu-sketches-size-join-estimation/observed_error_"+suffix+".pdf"
plt.savefig(name, bbox_extra_artists=(lgd,), bbox_inches = "tight")
plt.show()
