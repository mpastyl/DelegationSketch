#ifndef CM_BENCHMARK_H
#define CM_BENCHMARK_H


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

#ifndef ITHACA
#define ITHACA 0
#endif

int QUERRY_RATE;
int DURATION;
void * threadEntryPoint(void * threadArgs);
#endif