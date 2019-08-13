#ifndef THREAD_DATA_H
#define THREAD_DATA_H

#include <pthread.h>
#include <sys/time.h>
#include "barrier.h"
#include "relation.h"
#include "sketches.h"
#include "libdivide.h"
//#include "cm_benchmark.h"

#define FILTER_SIZE 16

typedef struct Filter_T{
    alignas(32) int filter_id[FILTER_SIZE];
    volatile unsigned int filter_count[FILTER_SIZE];
    unsigned int filter_old_count[FILTER_SIZE];
    volatile int filterCount;
    Filter_T * volatile next;
    char padding[64-4]; // Need to figure out why it breaks for some sizes
    volatile int filterFull; 
    char padding2[64-4]; // Need to figure out why it breaks for some sizes
}FilterStruct;

void push(FilterStruct * filter, FilterStruct * volatile * headPointer){

    FilterStruct * volatile oldHead = *headPointer;
    filter->next = oldHead;
    while(! __sync_bool_compare_and_swap (headPointer, oldHead, filter)){
        oldHead = *headPointer;
        filter->next = oldHead;
    }
}

FilterStruct * pop(FilterStruct * volatile * headPointer){
    FilterStruct * volatile oldHead = *headPointer;
    FilterStruct * volatile newHead = oldHead->next;
    while(! __sync_bool_compare_and_swap (headPointer, oldHead, newHead)){
        oldHead = *headPointer;
        newHead = oldHead->next;
    }
    return oldHead;
}

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
    struct libdivide::libdivide_s32_t * fastDivHandle;
    double returnData;
    FilterStruct Filter;
    FilterStruct * volatile listOfFullFilters;
    int * pendingQueriesKeys; // need volatiles?
    unsigned int * pendingQueriesCounts;
    volatile int * pendingQueriesFlags;
    struct{
        volatile int insertsPending;
        char pad1[60];
    };
    struct{
        volatile int queriesPending;
        char pad2[60];
    };
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