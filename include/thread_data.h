#ifndef THREAD_DATA_H
#define THREAD_DATA_H

#include <pthread.h>
#include <sys/time.h>
#include "barrier.h"
#include "relation.h"
#include "sketches.h"
//#include "cm_benchmark.h"

#define FILTER_SIZE 16

typedef struct{
    alignas(32) int filter_id[FILTER_SIZE];
    unsigned int filter_count[FILTER_SIZE];
    unsigned int filter_old_count[FILTER_SIZE];
    int filterCount;
    char padding[64]; // Need to figure out why it breaks for some sizes
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
    int * pendingQueriesKeys; // need volatiles?
    unsigned int * pendingQueriesCounts;
    volatile int * pendingQueriesFlags;
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