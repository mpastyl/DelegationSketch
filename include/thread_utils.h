#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H


#include "thread_data.h"
#include "barrier.h"
#include "cm_benchmark.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void initThreadData(Count_Min_Sketch ** sketchArray, Relation * relation){
    int i;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    barrier_init(&barrier_global, numberOfThreads + 1);
    barrier_init(&barrier_started, numberOfThreads);

    threadIds = (int *) calloc(numberOfThreads, sizeof(int));
    threadData = (threadDataStruct *) calloc(numberOfThreads, sizeof(threadDataStruct));
    for (i=0; i<numberOfThreads; ++i){
        threadData[i].tid = i;
        threadIds[i] = i;
        threadData[i].theSketch = sketchArray[i];
        threadData[i].sketchArray  = sketchArray;
        threadData[i].theGlobalSketch = globalSketch;
        threadData[i].theData = relation;
        threadData[i].elementsProcessed = 0;
    }
}

void spawnThreads(){
    int i;
    threads = (pthread_t *)malloc(numberOfThreads * sizeof(pthread_t));
    for (i=0; i<numberOfThreads; i++){
        int rc = pthread_create(&threads[i], NULL, threadEntryPoint, (void *) &(threadIds[i]));
        if (rc){
            perror("Could not spawn threads, exiting...\n");
            exit(0);
        }
    }
}

void collectThreads(){
    int i;
    for(i=0; i<numberOfThreads; i++){
        pthread_join(threads[i],NULL);
    }
}

void setaffinity_oncpu(unsigned int cpu)
{
    cpu_set_t cpu_mask;
    int err;

    CPU_ZERO(&cpu_mask);
    CPU_SET(cpu, &cpu_mask);

    err = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
    if (err) {
        perror("sched_setaffinity");
        exit(1);
    }
}

#endif