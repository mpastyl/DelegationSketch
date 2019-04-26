#ifndef THREAD_DATA_H
#define THREAD_DATA_H

#include <pthread.h>
#include <sys/time.h>
#include "barrier.h"

typedef struct
{
    int tid;
    struct timeval start;
    struct timeval end;
    int startIndex;
    int endIndex;
    int returnData;
}threadDataStruct;


int numberOfThreads;
threadDataStruct * threadData;
int * threadIds;
pthread_t *threads;
pthread_attr_t attr;
barrier_t barrier_global;
barrier_t barrier_started;


#endif 