#ifndef CM_BENCHMARK_H
#define CM_BENCHMARK_H

#define QUERRY_RATE 4 //percentage of queries vs inserts, per thread

#define FIXED_DURATION 1 //sec

void * threadEntryPoint(void * threadArgs);
#endif