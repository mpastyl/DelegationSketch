#ifndef CM_BENCHMARK_H
#define CM_BENCHMARK_H

#include "thread_data.h"

//define preprocessor flags that are not set in the Makefile
#ifndef SHARED_SKETCH
#define SHARED_SKETCH 0
#endif

#ifndef LOCAL_COPIES
#define LOCAL_COPIES 0
#endif

#ifndef HYBRID
#define HYBRID 0
#endif

#ifndef REMOTE_INSERTS
#define REMOTE_INSERTS 0
#endif

#ifndef USE_MPSC
#define USE_MPSC 0
#endif

#ifndef USE_FILTER
#define USE_FILTER 0
#endif

#ifndef AUGMENTED_SKETCH
#define AUGMENTED_SKETCH 0
#endif

#ifndef DELEGATION_FILTERS
#define DELEGATION_FILTERS 0
#endif

#ifndef USE_LIST_OF_FILTERS
#define USE_LIST_OF_FILTERS 0
#endif

#ifndef ITHACA
#define ITHACA 0
#endif

int QUERRY_RATE;
int DURATION;
void * threadEntryPoint(void * threadArgs);
void insert(threadDataStruct * localThreadData, unsigned int key, unsigned int increment);
#endif