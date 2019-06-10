#ifndef THREAD_DATA_H
#define THREAD_DATA_H

#include <pthread.h>
#include <sys/time.h>
#include "barrier.h"
#include "relation.h"
#include "sketches.h"
#include "cm_benchmark.h"

typedef struct{
    int filter_id[16];
    unsigned int filter_count[16];
    int filterCount;
    int filterFull;
}FilterStruct;

typedef struct
{
    int tid;
    Count_Min_Sketch * theSketch;
    Relation * theData;
    int elementsProcessed;
    struct timeval start;
    struct timeval end;
    int startIndex;
    int endIndex;
    double returnData;
    FilterStruct Filter;
    int numQueries;
    int numInserts;
    Count_Min_Sketch ** sketchArray;
    Count_Min_Sketch * theGlobalSketch;
}threadDataStruct;

Count_Min_Sketch * globalSketch;
int numberOfThreads;
threadDataStruct * threadData;
int * threadIds;
pthread_t *threads;
pthread_attr_t attr;
barrier_t barrier_global;
barrier_t barrier_started;
volatile int startBenchmark = 0;

#endif 